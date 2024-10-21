#ifndef RASTERTILER_H_
#define RASTERTILER_H_

/**
 * @file RasterTiler.h
 * @brief this declares and defines and `RasterTiler` class
 */

#include "GDALTiler.h"

namespace stt {
    class RasterTiler;
}

class stt::RasterTiler: public stt::GDALTiler
{
public:
    /// instantiate a tiler with all required arguments
    RasterTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
        GDALTiler(poDataset, grid, options)
    {}

    /// instantiate a tiler with an empty GDAL dataset
    RasterTiler(): GDALTiler() {}

    /// instantiate a tiler with a dataset and grid but no options
    RasterTiler(GDALDataset *poDataset, const Grid &grid):
        RasterTiler(poDataset, grid, TilerOptions())
    {}

    /// overload the assignment operator
    RasterTiler &
    operator=(const RasterTiler &other) {
        GDALTiler::operator=(other);
        return *this;
    }

    /// override to return a covariant data type
    virtual GDALTile *
    createTile(GDALDataset *dataset, const TileCoordinate &coord) const override {
        return createRasterTile(dataset, coord);
    }
};

#endif /* RASTERTILER_H_ */
