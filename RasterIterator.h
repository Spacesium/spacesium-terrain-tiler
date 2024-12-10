#ifndef RASTERITERATOR_H_
#define RASTERITERATOR_H_

/**
 * @file RasterIterator.h
 * @brief this declares the `RasterIterator` class
 */

#include "gdal_priv.h"

#include "TilerIterator.h"
#include "RasterTiler.h"

namespace  stt {
    class RasterIterator;
}

/**
 * @brief this forward iterates over all tiles in a `RasterTiler`
 *
 * instances of this class take a `RasterTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler, returning a `GDALTile *` when
 * dereferenced. it is the caller's responsibility to call `delete` on the tile.
 */

class stt::RasterIterator: public TilerIterator
{
public:
    /// instantiate an iterator with a tiler
    RasterIterator(const RasterTiler &tiler):
        RasterIterator(tiler, tiler.maxZoomLevel(), 0)
    {}

    /// the target constructor
    RasterIterator(const RasterTiler &tiler, i_zoom startZoom, i_zoom endZoom):
        TilerIterator(tiler, startZoom, endZoom)
    {}

    virtual GDALTile *
    operator*() const override {
        return static_cast<GDALTile *>(TilerIterator::operator*());
    }
};

#endif /* RASTERITERATOR_H_ */
