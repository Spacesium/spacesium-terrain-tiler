#ifndef TERRAINTILE_H_
#define TERRAINTILE_H_

/**
 * @file TerrainTile.h
 * @brief this declares the `Terrain` and `TerrainTile` classes
 */

#include <vector>

#include "gdal_priv.h"

#include "config.h"
#include "Tile.h"
#include "TileCoordinate.h"
#include "STTOutputStream.h"

namespace stt {
    class Terrain;
    class TerrainTile;
}

/**
 * @brief model the terrain heightmap specification
 *
 * this aims to implement the cesium [heightmap-1.0 terrain
 * format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html).
 */
class STT_DLL stt::Terrain
{
public:
    /// create an empty terrain object
    Terrain();

    /// instantiate using terrain data on the file system
    Terrain(const char *fileName);

    /// read terrain data from a file handle
    Terrain(FILE *fp);

    /// read terrain data from the filesystem
    void readFile(const char *fileName);

    /// write terrain data to a file handle
    void writeFile(FILE *fp) const;

    /// write terrain data to the filesystem
    void writeFile(const char *fileName) const;

    /// write terrain data to an output stream
    void writeFile(STTOutputStream &ostream) const;

    /// get the water mask as a boolean mask
    std::vector<bool> mask();

    /// does the terrain tile have child tiles?
    bool hasChildren() const;

    /// does the terrain tile have a south west child tile?
    bool hasChildSW() const;

    /// does the terrain tile have a south east child tile?
    bool hasChildSE() const;

    /// does the terrain tile have a north west child tile?
    bool hasChildNW() const;

    /// does the terrain tile have a north east child tile?
    bool hasChildNE() const;

    /// specify that there is a south west child tile
    void setChildSW(bool on = true);

    /// specify that there is a south east child tile
    void setChildSE(bool on = true);

    /// specify that there is a north west child tile
    void setChildNW(bool on = true);

    /// specify that there is a north east child tile
    void setChildNE(bool on = true);

    /// specify that all child tiles are present
    void setAllChildren(bool on = true);

    /// specify that this tile is all water
    void setIsWater();

    /// is this tile all water?
    void isWater() const;

    /// specify that this tile is all land
    void setIsLand();

    /// is this tile all land?
    bool isLand() const;

    /// does this tile have a water mask?
    bool hasWaterMask() const;

    /// get the height data as a const vector
    const std::vector<i_terrain_height> &getHeights();

    /// get the height data as a vector
    std::vector<i_terrain_height> &getHeights();

protected:
    /// the terrain height data
    std::vector<i_terrain_height> mHeights; // NOTE: replace with `std::array` in C++11

    /// the number of heights cells within a terrain tile
    static const unsigned short int TILE_CELL_SIZE = TILE_SIZE * TILE_SIZE;

    /// the number of water mask cells within a terrain tile
    static const unsigned int MASK_CELL_SIZE = MASK_SIZE * MASK_SIZE;

    /**
     * @brief the maximum byte size of an uncompressed terrain tile
     *
     * this is calculated as (heights + child flags + water mask).
     */
    static const unsigned int MAX_TERRAIN_SIZE = (TILE_CELL_SIZE * 2) + 1 + MASK_CELL_SIZE;

private:
    char mChildren;                /// the child flags
    char mMask[MASK_CELL_SIZE];    /// the water mask
    size_t mMaskLength;            /// what size is the water mask?

    /**
     * @brief bit flags defining child tile existence
     *
     * there is a good discussion on bit flags
     * [here](http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags").
     */

    enum Children {
        TERRAIN_CHILD_SW = 1,    // 2^0, bit 0
        TERRAIN_CHILD_SE = 2,    // 2^1, bit 0
        TERRAIN_CHILD_NW = 4,    // 2^2, bit 0
        TERRAIN_CHILD_NE = 8,    // 2^3, bit 0
    };
};

/**
 * @brief `Terrain` data associated with a `Tile`
 *
 * associating terrain data with a tile coordinate allows the tile to be
 * converted to a geo-referenced raster (see `TerrainTile::heightsToRaster`).
 */
class STT_DLL stt::TerrainTile: public Terrain, public Tile
{
    friend class TerrainTiler;

public:
    /// create a terrain tile from a tile coordinate
    TerrainTile(const TileCoordinate &coord);

    /// create a terrain tile from a file
    TerrainTile(const char *fileName, const TileCoordiante &coord);

    /// create a terrain tile from terrain data
    TerrainTile(const Terrain &terrain, const TileCoordiante &coord);

    /// get the height data as an in memory GDAL raster
    GDALDatasetH heightsToRaster() const;
};

#endif /* TERRAINTILE_H_ */
