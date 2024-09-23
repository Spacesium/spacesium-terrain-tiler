#ifndef STTTILE_H_
#define STTTILE_H_

/**
 * @file Tile.h
 * @brief this declares the `Tile` class
 */

#include "Grid.h"

namespace stt {
    class Tile;
}

/**
 * @brief an abstract base class for a tile
 *
 * this provides a way of associating a `TileCoordinate` with tile data.
 */

class stt::Tile: public TileCoordinate
{
public:
    virtual ~Tile() = 0; // this is an abstract base class

    /// create an empty tile from a grid
    Tile(): TileCoordinate() {}

    /// create a tile from a tile coordinate
    Tile(const TileCoordinate &coord): TileCoordinate(coord) {}
};

inline stt::Tile::~Tile() {} // prevents linker errors

#endif /* STTTILE_H_ */
