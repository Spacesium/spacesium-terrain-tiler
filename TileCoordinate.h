#ifndef TILECOORDINATE_H_
#define TILECOORDINATE_H_

/**
 * @file TileCoordinate.h
 * @brief this declares and defines the `TileCoordinate` class
 */

#include "types.h"

namespace stt {
    class TileCoordinate;
}

/**
 * @brief a `TileCoordiante` identifies a particular tile
 *
 * an instance of this class is composed of a tile point and a zoom level:
 * together this identifies an individual tile.
 *
 */

class stt::TileCoordinate: public TilePoint
{
public:
    /// create the 0-0-0 level tile coordinate
    TileCoordinate(): TilePoint(0, 0), zoom(0) {}

    /// the const copy constructor
    TileCoordinate(const TileCoordinate &other):
        TilePoint(other.x, other.y),
        zoom(other.zoom)
    {}

    /// instantiate a tile coordinate from the zoom, x and y
    TileCoordinate(i_zoom zoom, i_tile x, i_tile y):
        TilePoint(x, y),
        zoom(zoom)
    {}

    /// instantiate a tile coordinate using the zoom and a tile point
    TileCoordinate(i_zoom zoom, const TilePoint &coord):
        TilePoint(coord),
        zoom(zoom)
    {}

    /// override the equality operator
    inline bool
    operator==(const TileCoordinate &other) const {
        return TilePoint::operator==(other)
        && zoom == other.zoom;
    }

    /// override the assignment operator
    inline void
    operator=(const TileCoordinate &other) {
        TilePoint::operator=(other);
        zoom = other.zoom;
    }

    /// set the point
    inline void
    setPoint(const TilePoint &point) {
        TilePoint::operator=(point);
    }

    i_zoom zoom;
};

#endif /* TILECOORDINATE_H_ */
