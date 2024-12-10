#ifndef TILERITERATOR_H_
#define TILERITERATOR_H_

/**
 * @file TilerIterator.h
 * @brief this declares and defines the `TilerIterator` class
 */

#include "GridIterator.h"
#include "GDALTiler.h"

namespace stt {
    class TilerIterator;
}

/**
 * @brief forward iterate over tiles in a `GDALTiler`
 *
 * instances of this class take a `GDALTiler` (or derived class) in the
 * constructor and are used to forward iterate over all tiles in the tiler,
 * returning a `Tile *` when dereferenced. it is the caller's responsibility to
 * call `delete` on the tile.
 */
class stt::TilerIterator: public GridIterator
{
public:
    /// instantiate an iterator with a tiler
    TilerIterator(const GDALTiler &tiler):
        TilerIterator(tiler, tiler.maxZoomLevel(), 0)
    {}

    TilerIterator(const GDALTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0):
        GridIterator(tiler.grid(), tiler.bounds(), startZoom, endZoom),
        tiler(tiler)
    {}

    /// override the dereference operator to return a Tile
    virtual Tile *
    operator*() const {
        return tiler.createTile(tiler.dataset(), *(GridIterator::operator*()));
    }

protected:
    const GDALTiler &tiler;    /// the tiler we are iterating over
};

#endif /* TILERITERATOR_H_ */
