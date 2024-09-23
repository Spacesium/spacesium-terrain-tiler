/**
* @file GDALDatasetReader.cpp
* @brief this defines the `GDALDatasetReader` class
*/

#include "gdal_priv.h"
#include "gdalwarper.h"

#include "STTException.h"
#include "GDALDatasetReader.h"
#include "TerrainTiler.h"

using namespace stt;

/**
* @details
* read a region of raster heights for the specified Dataset and Coordinate.
* this method uses `GDALRasterBand::RasterIO` function.
*/
float *
stt::GDALDatasetReader::readRasterHeights(const GDALTiler &tiler,
    GDALDataset *dataset, const TileCoordinate &coord,
    stt::i_tile tileSizeX, stt::i_tile tileSizeY)
{
    // the raster associated with this tile coordinate
    GDALTile *rasterTile = createRasterTile(tiler, dataset, coord);

    const stt::i_tile TILE_CELL_SIZE = tileSizeX * tileSizeY;
    float *rasterHeights = (float *)CPLCalloc(TILE_CELL_SIZE, sizeof(float));

    GDALRasterBand *heightsBand = rasterTile->dataset->GetRasterBand(1);

    if (heightsBand->RasterIO(GF_Read, 0, 0, tileSizeX, tileSizeY,
        (void *) rasterHeights, tileSizeX, tileSizeY, GDT_Float32,
        0, 0) != CE_None) {
        delete rasterTile;
        CPLFree(rasterHeights);

        throw STTException("Could not read heights from raster");
    }

    delete rasterTile;
    return rasterHeights;
}

/// create a raster tile from a tile coordinate
GDALTile *
stt::GDALDatasetReader::createRasterTile(const GDALTiler &tiler,
    GDALDataset *dataset, const TileCoordinate &coord)
{
    return tiler.createRasterTile(dataset, coord);
}

/// create a raster tiel from a GDALDataset
GDALTile *
stt::GDALDatasetReader::createOverview(const GDALTiler &tiler,
    GDALDataset *dataset, const TileCoordinate &coord, int overviewIndex)
{
    int nFactorScale = 2 << overviewIndex;
    int nRasterXSize = dataset->GetRasterXSize() / nFactorScale;
    int nRasterYSize = dataset->GetRasterYSize() / nFactorScale;

    GDALDataset *poOverview = NULL;
    double adfGeoTransform[6];

    // should we create an overview of the Dataset?
    if (nRasterXSize > 4 &&  nRasterYSize > 4 && dataset->GetGeoTransform(adfGeoTransform) == CE_None) {
        adfGeoTransform[1] *= nFactorScale;
        adfGeoTransform[5] *= nFactorScale;

        TerrainTiler tempTiler(tiler.dataset(), tiler.grid(), tiler.options);
        tempTiler.crsWKT = "";

        GDALTile *rasterTile = createRasterTile(tempTiler, dataset, coord);
        if (rasterTile) {
            poOverview = rasterTile->detach();
            delete rasterTile;
        }
    }

    return poOverview;
}

/// the destructor
stt::GDALDatasetReaderWIthOverviews::~GDALDatasetReaderWithOverviews()
{
    reset();
}

/// read a region of raster heights into an array for the specified
/// Dataset and Coordinate
float *
stt::GDALDatasetReaderWithOverviews::readRasterHeights(GDALDataset *dataset,
    const TileCoordinate &coord, stt::i_tile tileSizeX, stt::i_tile tileSizeY)
{
    GDALDataset *mainDataset = dataset;

    const stt::i_tile TILE_CELL_SIZE = tileSizeX *tileSizeY;
    float *rasterHeights = (float *)CPLCalloc(TILE_CELL_SIZE, sizeof(float));

    // replace GDAL Dataset by last valid Overview.
    for (int i = mOverviews.size() - 1; i >= 0; --i) {
        if (mOverviews[i]) {
            dataset = mOverviews[i];
            break;
        }
    }

    // extract the raster data, using overviews when necessary
    bool rasterOk = false;
    while (!rasterOk) {
        // the raster associated with this file coordinate
        GDALTile *rasterTile = createRasterTile(poTiler, dataset, coord);

        GDALRasterBand *heightsBand = rasterTile->dataset->GetRasterBand(1);

        if (heightsBand->RasterIO(GF_Read, 0, 0, tileSizeX, tileSizeY,
            (void *) rasterHeights, tileSizeX, tileSizeY, GDT_Float32,
            0, 0) != CE_None) {

            GDALDataset *psOverview = createOverview(poTiler, mainDataset,
                coord, mOverviewIndex++);
            if (psOverview) {
                mOverviews.push_back(psOverview);
                dataset = psOverview;
            } else {
                delete rasterTile;
                CPLFree(rasterHeights);
                throw STTException("Could not create an overview of current GDAL dataset");
            }

        } else {
            rasterOk = true;
        }

        delete rasterTile;
    }

    // everything ok?
    if (!rasterOk) {
        CPLFree(rasterHeights);
        throw STTException("Could not read heights from raster");
    }

    return rasterHeights;
}

/// releases all overviews
void
stt::GDALDatasetReaderWithOverviews::reset()
{
    mOverviewIndex = 0;

    for (int i = mOverviews.size() - 1; i >= 0; --i) {
        GDALDataset *poOverview = mOverviews[i];
        GDALClose(poOverview);
    }
    mOverviews.clear();
}
