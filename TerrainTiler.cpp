/**
* @file TerrainTiler.cpp
* @brief this defines the `TerrainTiler` class
*/

#include "STTException.h"
#include "TerrainTiler.h"
#include "GDALDatasetReader.h"

using namespace stt;

void stt::TerrainTiler::prepareSettingsOfTile(
    TerrainTile *terrainTile,
    const TileCoordinate &coord,
    float *rasterHeights,
    stt::i_tile tileSizeX,
    stt::i_tile tileSizeY
) const
{
    const stt::i_tile TILE_CELL_SIZE = tileSizeX * tileSizeY;

    // convert the raster data into the terrain tile heights. this assumes the
    // input raster data represents meters above sea level. each terrain height
    // value is the number of 1/5 meter units above -1000 meters.

    // TODO try doing this using a VRT derived band:
    // (https://gdal.org/en/stable/drivers/raster/vrt.html)
    for (unsigned short int i = 0; i < TILE_CELL_SIZE; i++) {
        terrainTile->mHeights[i] = (i_terrain_height) ((rasterHeights[i] + 1000) * 5);
    }

    // if we are not at the maximum zoom level we need to set child flags on
    // the tile where child tiles overlap the dataset bounds.
    if (coord.zoom != maxZoomLevel()) {
        CRSBounds tileBounds = mGrid.tileBounds(coord);

        if (! (bounds().overlaps(tileBounds))) {
            terrainTile->setAllChildren(false);
        } else {
            if (bounds().overlaps(tileBounds.getSW())) {
                terrainTile->setChildSW();
            }
            if (bounds().overlaps(tileBounds.getNW())) {
                terrainTile->setChildNW();
            }
            if (bounds().overlaps(tileBounds.getNE())) {
                terrainTile->setChildNE();
            }
            if (bounds().overlaps(tileBounds.getSE())) {
                terrainTile->setChildSE();
            }
        }
    }
}

TerrainTile * stt::TerrainTiler::createTile(GDALDataset *dataset, const TileCoordinate &coord) const
{
    // copy the raster data into an array
    float *rasterHeights = stt::GDALDatasetReader::readRasterHeights(*this, dataset, coord, TILE_SIZE, TILE_SIZE);

    // get a terrain tile represented by the tile coordinate
    TerrainTile *terrainTile = new TerrainTile(coord);
    prepareSettingsOfTile(terrainTile, coord, rasterHeights, TILE_SIZE, TILE_SIZE);
    CPLFree(rasterHeights);

    return terrainTile;
}

TerrainTile * stt::TerrainTiler::createTile(GDALDataset *dataset,
    const TileCoordinate &coord, stt::GDALDatasetReader *reader) const
{
    // copy the raster data into an array
    float *rasterHeights = reader->readRasterHeights(dataset, coord,
        TILE_SIZE, TILE_SIZE);

    // get a mesh tile represented by the tile coordinate
    TerrainTile *terrainTile = new TerrainTile(coord);
    prepareSettingsOfTile(terrainTile, coord, rasterHeights, TILE_SIZE, TILE_SIZE);
    CPLFree(rasterHeights);

    return terrainTile;
}

GDALTile * stt::TerrainTiler::createRasterTile(GDALDataset *dataset,
    const TileCoordinate &coord) const
{
    // ensure we have some data from which to create a tile
    if (dataset && dataset->GetRasterCount() < 1) {
        throw STTException("At least one band must be present in the GDAL dataset");
    }

    // get the bounds and resolution for a tile coordinate which represents
    // the data overlap requested by the terrain specification.
    double resolution;
    CRSBounds tileBounds = terrainTileBounds(coord, resolution);

    // convert the tile bounds into a geo transform
    double adfGeoTransform[6];
    adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
    adfGeoTransform[1] = resolution;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = tileBounds.getMaxY(); // max longitude
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -resolution;

    GDALTile *tile = GDALTiler::createRasterTile(dataset, adfGeoTransform);

    // the previous geotransform represented the data with an overlap as
    // required by the terrain specification. this now needs to be
    // overwritten so that the data is shifted to the bounds defined by
    // tile itself.
    tileBounds = mGrid.tileBounds(coord);
    resolution = mGrid.resolution(coord.zoom);
    adfGeoTransform[0] = tileBounds.getMinX(); // min longitude
    adfGeoTransform[1] = resolution;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = tileBounds.getMaxY(); // max longitude
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -resolution;

    // set the shifted geo transform to thw VRT
    if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
        throw STTException("Could not set geo transform on VRT");
    }

    return tile;
}

TerrainTiler & stt::TerrainTiler::operator=(const TerrainTiler &other)
{
    GDALTiler::operator=(other);

    return *this;
}
