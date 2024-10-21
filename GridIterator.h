#ifndef GRIDITERATOR_H_
#define GRIDITERATOR_H_

/**
 * @file GridIterator.h
 * @brief this declares and defines the `GridIterator` class
 */

#include <iterator>

#include "TileCoordinate.h"
#include "Grid.h"

namespace stt {
    class GridIterator;
}

/**
 * @brief a `GridIterator` forward iterates over tiles in a `Grid`
 *
 * instances of this class take a `Grid` (or derived class) in the constructor
 * and are used to forward iterate over all tiles contained in the grid,
 * starting from a specified maximum zoom level and moving up to a specified
 * minimum zoom level e.g.
 *
 * \code
 *   for (GridIterator iter(tiler); !iter.exhausted(); ++iter) {
 *     TileCoordinate tile = *iter;
 *     // do stuff with tile coordinate
 *   }
 * \endcode
 *
 * by default the iterator iterates over the full extent represented by the
 * grid, but alternative extents can be passed in to the constructor, acting as
 * a spatial filter
 */

class stt:GridIterator: public std::iterator<std::input_iterator_tag, TileCoordinate *>
{
public:
    /// instantiate an iterator with a grid and separate bounds
    GridIterator(const Grid &grid, i_zoom startZoom, i_zoom endZoom = 0) :
        grid(grid),
        startZoom(startZoom),
        endZoom(endZoom),
        gridExtent(grid.getExtent()),
        bounds(grid.getTileExtent(startZoom)),
        currentTile(TileCoordinate(startZoom, bounds.getLowerLeft())) // the initial tile coordinate
    {
        if (startZoom < endZoom)
            throw STTException("Iterating from a starting zoom level that is less than the end zoom level");
    }

    /// instantiate an iterator with a grid and separate bounds
    GridIterator(const Grid &grid, const CRSBounds &extent, i_zoom startZoom, i_zoom endZoom = 0) :
        grid(grid),
        startZoom(startZoom),
        endZoom(endZoom),
        gridExtent(extent)
    {
        if (startZoom < endZoom)
            throw STTException("Iterating from a starting zoom level that is less than the end zoom level");

        currentTile.zoom = startZoom;
        setTileBounds();
    }

    /// override the ++prefix operator
    GridIterator &
    operator++() {
        // don't increment if exhausted
        if (exhausted())
            return *this;

        /*
         * the statements in this function are the equivalent of the following `for`
         * loops but broken down for use in the iterator:
         *
         * for (i_zoom zoom = maxZoom; zoom >= 0; zoom--) {
         *   tiler.lowerLeftTile(zoom, tminx, bounds.getMinY());
         *   tiler.upperRightTile(zoom, bounds.getMaxX(), bounds.getMaxY());
         *
         *   for (int tx = tminx; tx <= bounds.getMaxX(); tx++) {
         *     for (int ty = bounds.getMinY(); ty <= bounds.getMaxY(); ty++) {
         *       TerrainTile *terrainTile = tiler.createTerrainTile(zoom, tx, ty);
         *     }
         *   }
         * }
         *
         * starting off in the lower left corner at the maximum zoom level iterate
         * over the Y tiles (columns) first from left to right; if columns are
         * exhausted then reset Y to the first column and increment the X to
         * iterate over the next row (from bottom to top). if the rows are
         * exhausted then we have iterated over theat zoom level: decrease the zoom
         * level and repeat the process for the new zoom level. do this until zoom
         * level 0 is reached.
         */

        if (++(currentTile.y) > bounds.getMaxY()) {
            if (++(currentTile.x) > bounds.getMaxX()) {
                if (currentTile.zoom > endZoom) {
                    (currentTile.zoom)--;

                    setTileBounds();
                }
            } else {
                currentTile.y = bounds.getMinY();
            }
        }

        return *this;
    }

    /// override the postfix++ operator
    GridIterator
    operator++(int) {
        GridIterator result(*this);    // make a copy for returning
        ++(*this);                     // use the prefix version to do the work
        return result;                 // return the copy (the old) value
    }

    /// override the equality operator
    bool
    operator==(const GridIterator &other) const {
        return currentTile == other.currentTile
        && startZoom == other.startZoom
        && endZoom == other.endZoom
        && bounds == other.bounds
        && gridExtent == other.gridExtent
        && grid == other.grid
    }

    /// override the inequality operator
    bool
    operator!=(const GridIterator &other) const {
        return !operator==(other);
    }

    /// deference the iterator to retrive a `TileCoordinate`
    virtual const TileCoordinate *
    operator*() const {
        return &currentTile;
    }

    /// return `true` if the iterator is at the end
    bool
    exhausted() const {
        return currentTile.zoom == endZoom && currentTile.x > bounds.getMaxX() && currentTile.y > bounds.getMaxY();
    }

    /// reset the iterator to a certain point
    void
    reset(i_zoom start, i_zoom end) {
        if (start < end)
            throw STTException("Starting zoom level cannot be less than the end zoom level");

        currentTile.zoom = startZom = start;
        endZoom = end;

        setTileBounds();
    }

    /// get the total number of elements in the iterator
    i_tile
    getSize() const {
        i_tile size = 0;
        for (i_zoom zoom = endZoom; zoom <= startZoom; ++zoom) {
            TileCoordinate ll = grid.crsToTile(gridExtent.getLowerLeft(), zoom);
            TileCoordinate ur = grid.crsToTile(gridExtent.getUpperRight(), zoom);

            TileBounds zoomBound(ll, ur);
            size += (zoomBound.getWidth() + 1) * (zoomBound.getHegith() + 1);
        }

        return size;
    }

    /// get the grid we are iterating over
    const Grid &
    getGrid() const {
        return grid;
    }

protected:
    /// set the tile bounds of the grid for the current zoom level
    void
    setTileBounds() {
        TileCoordinate ll = grid.crsToTile(gridExtent.getLowerLeft(), currentTile.zoom),
        TileCoordinate ur = grid.crsToTile(gridExtent.getUpperRight(), currentTile.zoom),

        // set the bounds
        bounds = TileBounds(ll, ur);

        // set the current tile
        currentTile.setPoint(ll);
    }

    const Grid &grid;                /// the grid we are iterating over
    i_zoom startZoom;                /// the starting zom level
    i_zoom endZoom;                  /// the final zoom level
    CRSBounds gridExtent;            /// the extent of the underlying grid to iterate over
    TileBounds bounds;               /// the extent of the currently iterated zoom level
    TileCoordiante currentTile;      /// the identity of the current tile being pointed to
};

#endif /* GRIDITERATOR_H_ */
