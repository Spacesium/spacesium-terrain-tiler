#include <iostream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include "boost/program_options.hpp"
#include "gdal_priv.h"

#include "GDALDatasetReader.h"
#include "GlobalMercator.h"
#include "RasterIterator.h"
// #include "TerrainIterator.h"
#include "MeshIterator.h"
// #include "GDALDatasetReader.h"
#include "STTFileTileSerializer.h"
// #include "RasterTiler.h"

using namespace stt;
namespace po = boost::program_options;
namespace fs = std::filesystem;

struct paramsStruct {
    fs::path inputFile;
    fs::path outputDir;
    std::string profile;
    int threadCount;
    int tileSize;
    int startZoom;
    int endZoom;
    double meshQualityFactor;
    bool cesiumFriendly;
    bool vertexNormals;
    po::variables_map varMap;
    bool quiet;
    bool verbose;
    bool resume;
    std::string outputFormat;
    bool metadata;
};

paramsStruct parseOptions(int argc, char *argv[])
{
    paramsStruct params;
    po::options_description optionalArgs;

    optionalArgs.add_options()
        ("help,h", "show help message")
        (
            "output-directory,o",
            po::value<fs::path>(&params.outputDir)->default_value(fs::current_path()),
            "output directory"
        )
        (
            "profile,p",
            po::value<std::string>(&params.profile)->default_value("geodetic"),
            "specify the TMS profile for the tiles. this is either `geodetic` (the default) or `mercator`"
        )
        (
            "format,f",
            po::value<std::string>(&params.outputFormat)->default_value("Mesh"),
            "specify the output format for the tiles. this is either `Terrain` (the default), `Mesh` (Chunked LOD mesh), or any format listed by `gdalinfo --formats`"
        )
        (
            "verbose,v",
            po::value<bool>(&params.verbose)->default_value(false),
            "be more noisy"
        )
        (
            "quiet,q",
            po::value<bool>(&params.quiet)->default_value(false),
            "only output errors"
        )
    ;

    po::options_description hiddenArgs;
    hiddenArgs.add_options()
        ("input-file,i", po::value<fs::path>(&params.inputFile), "input file")
    ;

    po::options_description commandLineArgs;
    commandLineArgs.add(optionalArgs).add(hiddenArgs);

    po::positional_options_description positionArgs;
    positionArgs.add("input-file", -1);

    po::store(
        po::command_line_parser(argc, argv).
        options(commandLineArgs).
        positional(positionArgs).
        run(),
        params.varMap
    );

    po::notify(params.varMap);

    if (params.varMap.count("help")) {
        std::cout << "space-terrain-tiler\n";
        std::cout << optionalArgs << "\n";
        std::cout << hiddenArgs << "\n";
    }

    return params;
}

/// handle the terrain metadata
class TerrainMetadata
{
public:
    TerrainMetadata() {}

    // defines the valid tile indices of a level in a Tileset
    struct LevelInfo {
    public:
        LevelInfo() {
            startX = startY = std::numeric_limits<int>::max();
            finalX = finalY = std::numeric_limits<int>::min();
        }
        int startX, startY;
        int finalX, finalY;

        inline void add(const TileCoordinate *coordinate) {
            startX = std::min(startX, (int)coordinate->x);
            startY = std::min(startY, (int)coordinate->y);
            finalX = std::max(finalX, (int)coordinate->x);
            finalY = std::max(finalY, (int)coordinate->y);

        }

        inline void add(const LevelInfo &level) {
            startX = std::min(startX, level.startX);
            startY = std::min(startY, level.startY);
            finalX = std::max(finalX, level.startX);
            finalY = std::max(finalY, level.startY);

        }
    };

    std::vector<LevelInfo> levels;

    // defines the bounding box covered by the Terrain
    CRSBounds bounds;

    // add metadata of the specified Coordinate
    void add(const Grid &grid, const TileCoordinate *coordinate) {
        CRSBounds tileBounds = grid.tileBounds(*coordinate);
        i_zoom zoom = coordinate->zoom;

        if ((1 + zoom) > levels.size()) {
            levels.resize(1 + zoom, LevelInfo());
        }
        LevelInfo &level = levels[zoom];
        level.add(coordinate);

        if (bounds.getMaxX() == bounds.getMinX()) {
            bounds = tileBounds;
        } else {
            bounds.setMinX(std::min(bounds.getMinX(), tileBounds.getMinX()));
            bounds.setMinY(std::min(bounds.getMinY(), tileBounds.getMinY()));
            bounds.setMaxX(std::max(bounds.getMaxX(), tileBounds.getMaxX()));
            bounds.setMaxY(std::max(bounds.getMaxY(), tileBounds.getMaxY()));
        }
    }

    // add metadata info
    void add(const TerrainMetadata &otherMetadata) {
        if (otherMetadata.levels.size() > 0) {
            const CRSBounds &otherBounds = otherMetadata.bounds;

            if (otherMetadata.levels.size() > levels.size()) {
                levels.resize(otherMetadata.levels.size(), LevelInfo());
            }

            for (size_t i = 0; i < otherMetadata.levels.size(); i++) {
                levels[i].add(otherMetadata.levels[i]);
            }

            bounds.setMinX(std::min(bounds.getMinX(), otherBounds.getMinX()));
            bounds.setMinY(std::min(bounds.getMinY(), otherBounds.getMinY()));
            bounds.setMaxX(std::max(bounds.getMaxX(), otherBounds.getMaxX()));
            bounds.setMaxY(std::max(bounds.getMaxY(), otherBounds.getMaxY()));
        }
    }

    /// output the layer.json metadata file
    /// https://help.agi.com/TerrainServer/RESTAPIGuide.html
    /// https://github.com/mapbox/tilejson-spec/tree/master/3.0.0
    void writeJsonFile(
        const std::string &filename,
        const std::string &datasetName,
        const std::string &outputFormat = "Mesh",
        const std::string &profile = "geodetic",
        bool writeVertexNormals = false) const {

        FILE *fp = fopen(filename.c_str(), "w");

        if (fp == NULL) {
            throw STTException("Failed to open metadata file");
        }

        fprintf(fp, "{\n");
        fprintf(fp, "  \"tilejson\": \"3.0.0\",\n");
        fprintf(fp, "  \"name\": \"%s\"\n", datasetName.c_str());

        fprintf(fp, "}\n");
        fclose(fp);
    }
};

/*
* increment a TilerIterator whilst cooperating between threads
*
* this function maintains a global index on an iterator and when called
* ensures the iterator is incremented to point to the next global index. this
* can therefore be called with different tiler iterators by different threads
* to ensure all tiles are iterated over consecutively. it assumes individual
* tile iterators point to the same source GDAL dataset.
*/
static int globalIteratorIndex {0}; // keep track of where we are globally
template<typename T> int incrementIterator(T &iter, int currentIndex)
{
    static std::mutex mutex; // ensure iterations occure serially between threads
    std::lock_guard<std::mutex> lock(mutex);

    while (currentIndex < globalIteratorIndex) {
        ++iter;
        ++currentIndex;
    }
    ++globalIteratorIndex;

    return currentIndex;
}

/// get a handle on the total number of tiles to be created
static int iteratorSize = 0; // the total number of tiles
template<typename T> void setIteratorSize(T &iter)
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    if (iteratorSize == 0) {
        iteratorSize = iter.getSize();
    }
}

/// a thread safe wrapper around `GDALTermProgress`
static int CPL_STDCALL termProgress(double dfComplete, const char *pszMessage,
                                    void *pProgressArg)
{
    static std::mutex mutex;
    int status;
}

// default to outputting using the GDAL progress meter
static GDALProgressFunc progressFunc = termProgress;

/// output the progress of the tiling operation
int showProgress(int currentIndex, std::string filename)
{
    std::stringstream stream;
    stream << "created " << filename << " in thread " << std::this_thread::get_id();
    std::string message = stream.str();

    return progressFunc(currentIndex / (double) iteratorSize, message.c_str(), NULL);
}

int showProgress(int currentIndex)
{
    return progressFunc(currentIndex / (double) iteratorSize, NULL, NULL);
}

/// output mesh tiles represented by a tiler to a directory
static void buildMesh(MeshSerializer &serializer, const MeshTiler &tiler,
    paramsStruct &params, TerrainMetadata *metadata,
    bool writeVertexNormals = false)
{

    i_zoom startZoom = (params.startZoom < 0) ? tiler.maxZoomLevel() : params.startZoom;
    i_zoom endZoom = (params.endZoom < 0) ? 0 : params.endZoom;

    // DEBUG Chunker:
    #if 0
    bool writeVertexNormals {false};
    const std::string dirname = params.outputDir;
    std::cout << "dirname: " << dirname << "\n";
    TileCoordinate coordinate(13, 8102, 6047);
    MeshTile *tile = tiler.createMesh(tiler.dataset(), coordinate);
    const std::string txtname = STTFileTileSerializer::getTileFilename(&coordinate, dirname, "wkt");
    const Mesh &mesh = tile->getMesh();
    mesh.writeWktFile(txtname.c_str());

    CRSBounds bounds = tiler.grid().tileBounds(coordinate);
    double x = bounds.getMinX() + 0.5 * (bounds.getMaxX() - bounds.getMinX());
    double y = bounds.getMinY() + 0.5 * (bounds.getMaxY() - bounds.getMinY());
    CRSPoint point(x, y);
    TileCoordinate c = tiler.grid().crsToTile(point, coordinate.zoom);

    const std::string filename = STTFileTileSerializer::getTileFilename(&coordinate, dirname, "terrain");
    tile->writeFile(filename.c_str(), writeVertexNormals);
    #endif

    std::cout << "startZoom: " << startZoom << "\n";
    std::cout << "endZoom: " << endZoom << "\n";

    MeshIterator iter(tiler, startZoom, endZoom);
    int currentIndex = incrementIterator(iter, 0);
    setIteratorSize(iter);
    GDALDatasetReaderWithOverviews reader(tiler);

    std::cout << "before while\n";
    while (!iter.exhausted()) {
        const TileCoordinate *coordinate = iter.GridIterator::operator*();
        if (metadata) {
            metadata->add(tiler.grid(), coordinate);
        }

        // std::cout << "coordinate x: " << coordinate->x << "\n";
        // std::cout << "coordinate y: " << coordinate->y << "\n";
        // std::cout << "currentIndex: " << currentIndex << "\n";

        if (serializer.mustSerializeCoordinate(coordinate)) {
            MeshTile *tile = iter.operator*(&reader);
            serializer.serializeTile(tile, writeVertexNormals);
            delete tile;
        }
        std::cout << "currentIndex: " << currentIndex << "\n";

        currentIndex = incrementIterator(iter, currentIndex);
        showProgress(currentIndex);
    }
}


static void buildMetadata(const RasterTiler &tiler, paramsStruct params,
                          TerrainMetadata *metadata)
{
    const std::string dirname = params.outputDir;
}

int main(int argc, char *argv[])
{
    paramsStruct params = parseOptions(argc, argv);

    if (params.varMap.count("input-file")) {
        if (fs::is_regular_file(params.inputFile)) {
            std::cout << "input file: " << params.inputFile << "\n";
        } else {
            std::cerr << "input file " << params.inputFile << " not found\n";
            return EXIT_FAILURE;
        }
    }

    if (params.varMap.count("output-directory")) {
        if (fs::is_directory(params.outputDir)) {
            std::cout << "output directory: " << params.outputDir << "\n";
        } else {
            std::cout << "output directory " << params.outputDir;
            std::cout << " not found. using current path\n";
        }
    }

    std::cout << "root name: " << params.outputDir.root_name().string() << "\n";
    std::cout << "file name: " << params.outputDir.filename() << "\n";
    std::cout << "stem: " << params.outputDir.stem() << "\n";
    std::cout << "extension: " << params.outputDir.extension() << "\n";
    std::cout << "quiet: " << params.quiet << "\n";
    std::cout << "profile: " << params.profile << "\n";
    std::cout << "output format: " << params.outputFormat << "\n";

    int zoomVal { 1 };

    fs::path newPath = params.outputDir.parent_path() / std::to_string(zoomVal) / "somethingelse.png";
    std::cout << "new path: " << newPath.string() << "\n";

    GDALAllRegister();

    // define the grid we are going to use
    Grid grid;
    int tileSize = { 65 };
    grid = GlobalGeodetic(tileSize);

    std::cout << grid.tileSize() << "\n";
    std::cout << grid.getSRS().exportToWkt() << "\n";

    const char *charInputFile = params.inputFile.c_str();

    GDALDataset *poDataset;
    poDataset = GDALDataset::FromHandle(GDALOpen(charInputFile, GA_ReadOnly));

    if (poDataset == NULL) {
        std::cerr << "Error: could not open GDAL dataset" << "\n";
        return 1;
    }

    std::cout << poDataset->GetRasterXSize() << "\n";
    std::cout << poDataset->GetRasterYSize() << "\n";
    std::cout << poDataset->GetSpatialRef()->exportToWkt() << "\n";

    TilerOptions options;

    std::cout << options.resampleAlg << "\n";
    std::cout << options.errorThreshold << "\n";
    std::cout << options.warpMemoryLimit << "\n";

    options.resampleAlg = GRA_Average;
    options.errorThreshold = 0.125;
    options.warpMemoryLimit = 0.0;

    STTFileTileSerializer serializer(params.outputDir, params.resume);

    // Height Map Option
    // const TerrainTiler tiler2(poDataset, grid);

    // buildTerrain(serializer, tiler, command, threadMetadata);


    // Quantized Mesh Option
    if (params.outputFormat.compare("Mesh") == 0) {
        const MeshTiler mtiler(poDataset, grid, options, params.meshQualityFactor);
        const RasterTiler rtiler(poDataset, grid, options);
        TerrainMetadata *metadata = params.metadata ? new TerrainMetadata() : NULL;
        TerrainMetadata *threadMetadata = metadata ? new TerrainMetadata() : NULL;
        std::cout << params.outputFormat << "\n";
        std::cout << "rtiler->maxZoomLevel: " << rtiler.maxZoomLevel() << "\n";
        std::cout << "mtiler->maxZoomLevel: " << mtiler.maxZoomLevel() << "\n";
        buildMetadata(rtiler, params, threadMetadata);
        buildMesh(serializer, mtiler, params, threadMetadata, params.vertexNormals);
    }

    std::cout << "compare -- " << params.outputFormat.compare("Mesh") << "\n";

    GDALClose(poDataset);

    std::cout << "AIAIAIAIAIAIAIAIAIAIAIAIAIAIAIAIAIA" << std::endl;
}

