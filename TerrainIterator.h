#ifndef TERRAINITERATOR_H_
#define TERRAINITERATOR_H_

/**
* @file TerrainIterator.h
* @brief this declares the `TerrainIterator` class
*/

#include "TerrainTile.h"
#include "TerrainTiler.h"

namespace stt {
    class TerrainIterator;
}

/**
* @brief this forward iterates over all `TerrainTile`s in a `TerrainTiler`
*
* instances of this class take a `TerrainTiler` in the constructor and are used
* to forward iterate over all tiles in the tiler, returning a `TerrainTile *`
* when dereferenced. it is the caller's responsibility to call `delete` on
* the tile.
*/
class stt::TerrainIterator: public TilerIterator
{
public:
    /// instantiate an iterator with a tiler
    TerrainIterator(const TerrainTiler &tiler):
        TerrainIterator(tiler, tiler.maxZoomLevel(), 0)
    {}

    /// the target constructor
    TerrainIterator(const TerrainTiler &tiler, i_zoom startZoom, i_zoom endZoom):
        TilerIterator(tiler, startZoom, endZoom)
    {}

    virtual TerrainTile *
    operator*() const override {
        return static_cast<TerrainTile *>(TilerIterator::operator*());
    }

    virtual TerrainTile *
    operator*(stt::GDALDatasetReader *reader) const {
        return (static_cast<const TerrainTiler &>(tiler)).createTile(
            tiler.dataset(),
            *(GridIterator::operator*()),
            reader
        );
    }
};

#endif /* TERRAINITERATOR_H_ */
