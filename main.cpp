#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

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

/// output mesh tiles represented by a tiler to a directory
// static void buildMesh(MeshSerializer &serializer, const MeshTiler &tiler,
//     TerrainBuild *command, TerrainMetadata *metadata,
//     bool writeVertexNormals = false)
// {
// }


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
        const MeshTiler tiler(poDataset, grid, options, params.meshQualityFactor);
        // const RasterTiler tiler(poDataset, grid, options);
        std::cout << params.outputFormat << "\n";

        i_zoom startZoom = (params.startZoom < 0) ? tiler.maxZoomLevel() : params.startZoom;
        i_zoom endZoom = (params.endZoom < 0) ? 0 : params.endZoom;

        // DEBUG Chunker:
        #if 1
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

        std::cout << "x: " << x << "\n";
        std::cout << "y: " << y << "\n";
        #endif

        std::cout << "startZoom: " << startZoom << "\n";
        std::cout << "endZoom: " << endZoom << "\n";
    }

    std::cout << "compare -- " << params.outputFormat.compare("Mesh") << "\n";

    GDALClose(poDataset);

    std::cout << "AIAIAIAIAIAIAIAIAIAIAIAIAIAIAIAIAIA" << std::endl;
}
