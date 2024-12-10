/**
* @file STTFileTileSerializer.cpp
* @brief this defines the `STTFileTileSerializer` class
*/

#include <cstdio>
#include <string>
#include <mutex>

#include "concat.h"
#include "cpl_vsi.h"
#include "STTException.h"
#include "STTFileTileSerializer.h"

#include "GDALDatasetReader.h"
#include "STTFileOutputStream.h"
#include "STTZOutputStream.h"

static const char *osDirSep = "/";


/// create a filename for a tile coordinate
std::string
stt::STTFileTileSerializer::getTileFilename(const TileCoordinate *coord, const std::string dirname, const char *extension)
{
    static std::mutex mutex;
    VSIStatBufL stat;
    std::string filename = concat(dirname, coord->zoom, osDirSep, coord->x);

    std::lock_guard<std::mutex> lock(mutex);

    // check whether the `{zoom}/{x}` directory exists or not
    if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
        filename = concat(dirname, coord->zoom);

        // check whether the `{zoom}` directory exists or not
        if (VSIStatExL(filename.c_str(), &stat, VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG)) {
            // create the `{zoom}` directory
            if (VSIMkdir(filename.c_str(), 0755))
                throw STTException("Could not create the zoom level directory");

        } else if (!VSI_ISDIR(stat.st_mode)) {
            throw STTException("Zoom level file path is not a directory");
        }

        // create the `{zoom}/{x}` directory
        filename += concat(osDirSep, coord->x);
        if (VSIMkdir(filename.c_str(), 0755))
            throw STTException("Could not create the x level directory");

    } else if (!VSI_ISDIR(stat.st_mode)) {
        throw STTException("X level file path is not a directory");
    }

    // create the filename itself, adding the extension if required
    filename += concat(osDirSep, coord->y);
    if (extension != NULL) {
        filename += ".";
        filename += extension;
    }

    return filename;
}

/// check if file exists
static bool
fileExists(const std::string &filename) {
    VSIStatBufL statbuf;
    return VSIStatExL(filename.c_str(), &statbuf, VSI_STAT_EXISTS_FLAG) == 0;
}

/**
* @details
* returns if the specified TileCoordinate should be serialized
*/
bool stt::STTFileTileSerializer::mustSerializeCoordinate(const stt::TileCoordinate *coordinate)
{
    if (!mresume)
        return true;

    const std::string filename = getTileFilename(coordinate, moutputDir, "terrian");

    return !fileExists(filename);
}

/**
* @details
* serializer a GDALTile to the directory store
*/
bool
stt::STTFileTileSerializer::serializeTile(const stt::GDALTile *tile, GDALDriver *driver, const char *extension, CPLStringList &creationOptions)
{
    const TileCoordinate *coordinate = tile;
    const std::string filename = getTileFilename(coordinate, moutputDir, extension);
    const std::string temp_filename = concat(filename, ".tmp");

    GDALDataset *poDstDS;
    poDstDS = driver->CreateCopy(temp_filename.c_str(), tile->dataset, FALSE, creationOptions, NULL, NULL);

    // close the datasets, flushing data to destination
    if (poDstDS == NULL) {
        throw STTException("Could not create GDAL tile");
    }

    GDALClose(poDstDS);

    if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw STTException("Could not rename temporary file");
    }

    return true;
}

/**
 * @details
 * serialize a TerrainTile to the directory store
 */
bool
stt::STTFileTileSerializer::serializeTile(const stt::TerrainTile *tile)
{
    const TileCoordinate *coordinate = tile;
    const std::string filename = getTileFilename(coordinate, moutputDir, "terrain");
    const std::string temp_filename = concat(filename, ".tmp");

    STTZFileOutputStream ostream(temp_filename.c_str());
    tile->writeFile(ostream);
    ostream.close();

    if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw STTException("Could not rename temporary file");
    }

    return true;
}


/**
* @details
* serializer a MeshTile to the directory store
*/
bool
stt::STTFileTileSerializer::serializeTile(const stt::MeshTile *tile, bool writeVertexNormals)
{
    const TileCoordinate *coordinate = tile;
    const std::string filename = getTileFilename(coordinate, moutputDir, "terrain");
    const std::string temp_filename = concat(filename, ".tmp");

    STTZFileOutputStream ostream(temp_filename.c_str());
    tile->writeFile(ostream, writeVertexNormals);
    ostream.close();

    if (VSIRename(temp_filename.c_str(), filename.c_str()) != 0) {
        throw STTException("Could not rename temporary file");
    }

    return true;
}
