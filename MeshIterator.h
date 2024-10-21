#ifndef MESHITERATOR_H_
#define MESHITERATOR_H_

/**
* @file MeshIterator.h
* @brief this declares the `MeshIterator` class
*/

#include "MeshTiler.h"
#include "GridIterator.h"

namespace stt {
    class MeshIterator;
}

/**
 * @brief this forward iterates over all `MeshTile`s in a `MeshTiler`
 *
 * instances of this class take a `MeshTiler` in the constructor and are used
 * to forward iterate over all tiles in the tiler, returning a `MeshTile *`
 * when dereferenced. it is the caller's responsibility to call `delete`
 * on the tile
 */
class stt::MeshIterator: public GridIterator
{
public:
    /// instantiate an iterator with a tiler
    MeshIterator(const MeshTiler &tiler):
        MeshIterator(tiler, tiler.maxZoomLevel(), 0)
    {}

    MeshIterator(const MeshTiler &tiler, i_zoom startZoom, i_zoom endZoom = 0):
        GridIterator(tiler.grid(), tiler.bounds(), startZoom, endZoom),
        tiler(tiler)
    {}

    /// override the dereference operator to return a Tile
    virtual MeshTile *
    operator*(stt::GDALDatasetReader *reader) const {
        return tiler.createMesh(tiler.dataset(), *(GridIterator::operator*()), reader);
    }

protected:
    const MeshTiler &tiler;
};


#endif /* MESHITERATOR_H_ */
