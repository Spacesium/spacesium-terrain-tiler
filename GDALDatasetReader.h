#ifndef GDALDATASETREADER_H_
#define GDALDATASETREADER_H_

/**
* @file GDALDatasetReader.h
* @brief this declares the `GDALDatasetReader` class
*/

#include <string>
#include <vector>
#include "gdalwarper.h"

#include "TileCoordinate.h"
#include "GDALTiler.h"
#include "TerrainTiler.h"

namespace stt {
    class GDALDatasetReader;
    class GDALDatasetReaderWithOverviews;
}

/**
* @brief read raster tiles from a GDAL dataset
*
* this abstract base class is associated with a GDAL dataset.
* it allows to read a region of the raster according to
* a region defined by a Tile Coordinate.
*
* we can define our own
*/
class STT_DLL stt::GDALDatasetReader
{
public:
    static float *
    readRasterHeights(const GDALTiler &tiler, GDALDataset *dataset,
        const TileCoordinate &coord, stt::i_tile tileSizeX,
        stt::i_tile tileSizeY);

    /// read a region of raster heights into an array for the specified
    /// Dataset and Coordinate
    virtual float *
    readRasterHeights(GDALDataset *dataset, const TileCoordinate &coord,
        stt::i_tile tileSizeX, stt::i_tile tileSizeY) = 0;

protected:
    /// create a raster tile from a tile coordinate
    static GDALTile *
    createRasterTile(const GDALTiler &tiler, GDALDataset *dataset,
        const TileCoordinate &coord);

    /// create a VTR raster overview from GDALDataset
    static GDALDataset *
    createOverview(const GDALTiler &tiler, GDALDataset *dataset,
        const TileCoordinate &coord, int overviewIndex);
};

/**
* @brief implements a GDALDatasetReader that takes care of 'Integer overflow'
* errors
*
* this class creates Overviews to avoid 'Integer overflow' erros when
* extracting raster data.
*/
class STT_DLL stt::GDALDatasetReaderWithOverviews: public stt::GDALDatasetReader
{
public:
    /// instantiate a GDALDatasetReaderWithOverviews
    GDALDatasetReaderWithOverviews(const GDALTiler &tiler):
        poTiler(tiler),
        mOverviewIndex(0)
    {}

    /// the descructor
    ~GDALDatasetReaderWithOverviews();

    /// read a region of raster heights into an array for the specified
    /// Dataset and Coordinate
    virtual float *
    readRasterHeights(GDALDataset *dataset, const TileCoordinate &coord,
        stt::i_tile tileSizeX, stt::i_tile tileSizeY) override;

    /// releases all overviews
    void reset();

protected:
    /// the tiler to use
    const GDALTiler &poTiler;

    /// list of VRT overviews of the uderlying GDAL dataset
    std::vector<GDALDataset *> mOverviews;

    /// current vrt overview
    int mOverviewIndex;
};

#endif /* GDALDATASETREADER_H_ */
