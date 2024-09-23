#ifndef STTTYPES_H_
#define STTTYPES_H_

/**
* @file types.h
* @brief this declares basic types used by libstt
*/

#include <cstdint>
#include "Bounds.h"
#include "Coordinate3D.h"

/// all terrain related data types reside in this namespace
namespace stt {
    // simple types
    typedef unsigned int i_pixel;      /// a pixel value
    typedef unsigned int i_tile;       /// a tile coordinate
    typedef unsigned short int i_zoom; /// a zoom level
    typedef uint16_t i_terrain_height; /// a terrain tile height

    // complex types
    typedef Bounds<i_tile> TileBounds;      /// tile extents in tile coordinates
    typedef Coordinate<i_pixel> PixelPoint; /// tile location of a pixel
    typedef Coordinate<double> CRSPoint;    /// a coordinate reference system coordinate
    typedef Coordinate3D<double> CRSVertex; /// a 3D-vertex of a mesh or tile in CRS coordinates
    typedef Bounds<double> CRSBounds;       /// extents in CRS coordinates
    typedef Coordinate<i_tile> TilePoint;   /// the location of a tile
};

#endif /* STTTYPES_H_ */
