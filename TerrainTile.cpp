/**
* @file TerrainTile.cpp
* @brief this defines the `Terrain` and `TerrainTile` classes
*/

#include <string.h>

#include "zlib.h"
#include "ogr_spatialref.h"

#include "STTException.h"
#include "TerrainTile.h"
#include "GlobalGeodetic.h"
#include "Bounds.h"
#include "STTFileOutputStream.h"
#include "STTZOutputStream.h"

using namespace stt;

Terrain::Terrain(): mHeights(TILE_CELL_SIZE), mChildren(0)
{
    setIsLand();
}

/**
* @details this reads gzipped terrain data from a file
*/
Terrain::Terrain(const char *fileName): mHeights(TILE_CELL_SIZE)
{
    readFile(fileName);
}

/**
* @details This reads raw uncompressed terrain data from a file handle.
*/
Terrain::Terrain(FILE *fp): mHeights(TILE_CELL_SIZE)
{
    unsigned char bytes[2];
    int count = 0;

    // get the height data from the file handle
    while (count < TILE_CELL_SIZE && fread(bytes, 2, 1, fp) != 0) {
        // adapted from
        // <http://stackoverflow.com/questions/13001183/how-to-read-little-endian-integers-from-file-in-c>
        mHeights[count++] = bytes[0] | (bytes[1] << 8);
    }

    // check we have the expected amount of height data
    if (count + 1 != TILE_CELL_SIZE) {
        throw STTException("Not enough height data");
    }

    // get the child flag
    if (fread(&(mChildren), 1, 1, fp) != 1) {
        throw STTException("Could not read child tile byte");
    }

    // get the water mask
    mMaskLength = fread(mMask, 1, MASK_CELL_SIZE, fp);
    switch (mMaskLength) {
    case MASK_CELL_SIZE:
        break;
    case 1:
        break;
    default:
        throw STTException("Not contain enoguth water mask data");
    }
}

/**
* @details this reads gzipped terrain data from a file
*/
void Terrain::readFile(const char *fileName)
{
    unsigned char inflateBuffer[MAX_TERRAIN_SIZE];
    unsigned int inflatedBytes;
    gzFile terrainFile = gzopen(fileName, "rb");

    if (terrainFile == NULL) {
        throw STTException("Failed to open file");
    }

    // uncompress the file into the buffer
    inflatedBytes = gzread(terrainFile, inflateBuffer, MAX_TERRAIN_SIZE);
    if (gzread(terrainFile, inflateBuffer, 1) != 0) {
        gzclose(terrainFile);
        throw STTException("File has too many bytes to be a valid terrain");
    }
    gzclose(terrainFile);

    // check the water mask type
    switch (inflatedBytes) {
    case MAX_TERRAIN_SIZE:               // a water mask is present
        mMaskLength = MASK_CELL_SIZE;
        break;
        case (TILE_CELL_SIZE * 2) + 2:   // there is nowater mask
        mMaskLength = 1;
        break;
    default:                             // it can't be a terrain file
        throw STTException("File has wrong file size to be a valid terrain");
    }

    // get the height data
    short int byteCount = 0;
    for (short int i = 0; i < TILE_CELL_SIZE; i++, byteCount = i * 2) {
        mHeights[i] = inflateBuffer[byteCount] | (inflateBuffer[byteCount + 1] << 8);
    }

    // get the child flag
    mChildren = inflateBuffer[byteCount]; // byte 8451

    // get the water mask
    memcpy(mMask, &(inflateBuffer[++byteCount]), mMaskLength);
}

/**
* @details this writes raw uncompressed terrain data to a filehandle
*/
void Terrain::writeFile(FILE *fp) const
{
    STTFileOutputStream ostream(fp);
    writeFile(ostream);
}

/**
* @details this writes gzipped terrain data to a file
*/
void Terrain::writeFile(const char *fileName) const
{
    STTZFileOutputStream ostream(fileName);
    writeFile(ostream);
}

/**
* @details this writes raw terrain data to an output stream.
*/
void Terrain::writeFile(STTOutputStream &ostream) const
{
    // write the height data
    if (ostream.write((const void *) mHeights.data(), TILE_CELL_SIZE * 2) != TILE_CELL_SIZE * 2) {
        throw STTException("Failed to write height data");
    }

    // write the child flags
    if (ostream.write(&mChildren, 1) != 1) {
        throw STTException("Failed to write child flags");
    }

    // write the water mask
    if (ostream.write(&mMask, mMaskLength) != mMaskLength) {
        throw STTException("Failed to write water mask");
    }
}


std::vector<bool> Terrain::mask()
{
    std::vector<bool> mask;
    mask.assign(mMask, mMask + mMaskLength);
    return mask;
}

bool Terrain::hasChildren() const
{
    return mChildren;
}

bool Terrain::hasChildSW() const
{
    return ((mChildren & TERRAIN_CHILD_SW) == TERRAIN_CHILD_SW);
}

bool Terrain::hasChildSE() const
{
    return ((mChildren && TERRAIN_CHILD_SE) == TERRAIN_CHILD_SE);
}

bool Terrain::hasChildNW() const
{
    return ((mChildren & TERRAIN_CHILD_NW) == TERRAIN_CHILD_NW);
}

bool Terrain::hasChildNE() const
{
    return ((mChildren & TERRAIN_CHILD_NE) == TERRAIN_CHILD_NE);
}

void Terrain::setChildSW(bool on)
{
    if (on) {
        mChildren |= TERRAIN_CHILD_SW;
    } else {
        mChildren &= ~TERRAIN_CHILD_SW;
    }
}

void Terrain::setChildSE(bool on)
{
    if (on) {
        mChildren |= TERRAIN_CHILD_SE;
    } else {
        mChildren &= ~TERRAIN_CHILD_SE;
    }
}

void Terrain::setChildNW(bool on)
{
    if (on) {
        mChildren |= TERRAIN_CHILD_NW;
    } else {
        mChildren &= ~TERRAIN_CHILD_NW;
    }
}

void Terrain::setChildNE(bool on)
{
    if (on) {
        mChildren |= TERRAIN_CHILD_NE;
    } else {
        mChildren &= ~TERRAIN_CHILD_NE;
    }
}

void Terrain::setAllChildren(bool on)
{
    if (on) {
        mChildren = TERRAIN_CHILD_SW | TERRAIN_CHILD_SE | TERRAIN_CHILD_NW | TERRAIN_CHILD_NE;
    } else {
        mChildren = 0;
    }
}

void Terrain::setIsWater()
{
    mMask[0] = 1;
    mMaskLength = 1;
}

bool Terrain::isWater() const
{
    return mMaskLength == 1 && (bool) mMask[0];
}

void Terrain::setIsLand()
{
    mMask[0] = 0;
    mMaskLength = 1;
}

bool Terrain::isLand() const
{
    return mMaskLength == 1 && ! (bool) mMask[0];
}

bool Terrain::hasWaterMask() const
{
    return mMaskLength == MASK_CELL_SIZE;
}

/**
* @details the data in the returned vector can be altered but do not alter
* the number of elements in the vector.
*/
const std::vector<i_terrain_height> &Terrain::getHeights() const
{
    return mHeights;
}

TerrainTile::TerrainTile(const TileCoordinate &coord):
    Terrain(), Tile(coord)
{}

TerrainTile::TerrainTile(const char *fileName, const TileCoordinate &coord):
    Terrain(fileName), Tile(coord)
{}

TerrainTile::TerrainTile(const Terrain &terrain, const TileCoordinate &coord):
    Terrain(terrain), Tile(coord)
{}

GDALDatasetH TerrainTile::heightsToRaster() const
{
    // create the geo transform for this raster tile
    const GlobalGeodetic profile;
    const CRSBounds tileBounds = profile.tileBounds(*this);
    const i_tile tileSize = profile.tileSize();
    const double resolution = tileBounds.getWidth() / tileSize;
    double adfGeoTransform[6] = {
        tileBounds.getMinX(),
        resolution,
        0,
        tileBounds.getMaxY(),
        0,
        -resolution
    };

    // create the spatial reference system for the raster
    OGRSpatialReference oSRS;

    oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

    if (oSRS.importFromEPSG(4326) != OGRERR_NONE) {
        throw STTException("could not create EPSG:4326 spatial reference");
    }

    char *pszDstWKT = NULL;
    if (oSRS.exportToWkt(&pszDstWKT) != OGRERR_NONE) {
        CPLFree(pszDstWKT);
        throw STTException("could not create EPSG:4326 WKT string");
    }

    // create an 'in memory' raster
    GDALDriverH hDriver = GDALGetDriverByName("MEM");
    GDALDatasetH hDstDS;
    GDALRasterBandH hBand;

    hDstDS = GDALCreate(hDriver, "", tileSize, tileSize, 1, GDT_Int16, NULL);
    if (hDstDS == NULL) {
        CPLFree(pszDstWKT);
        throw STTException("could not create in memory raster");
    }
    CPLFree(pszDstWKT);

    // apply the geo transform
    if (GDALSetGeoTransform(hDstDS, adfGeoTransform) != CE_None) {
        GDALClose(hDstDS);
        throw STTException("could not set projection on VRT");
    }

    // finally write the height data
    hBand = GDALGetRasterBand(hDstDS, 1);
    if (GDALRasterIO(hBand, GF_Write, 0, 0, tileSize, tileSize,
        (void *) mHeights.data(), tileSize, tileSize, GDT_Int16, 0, 0) != CE_None) {
        GDALClose(hDstDS);
        throw STTException("could not write heights to in memory raster");
    }

    return hDstDS;
}
