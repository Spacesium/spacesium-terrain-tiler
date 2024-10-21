#ifndef MESHTILE_H_
#define MESHTILE_H_

/**
 * @file MeshTile.h
 * @brief this declares the `MeshTile` class
 */

#include "config.h"
#include "Mesh.h"
#include "TileCoordinate.h"
#include "Tile.h"
#include "STTOutputStream.h"

namespace stt {
    class MeshTile;
}

/**
 * @brief `Terrain` data associated with a `Mesh`
 *
 * this aims to implement the Cesium [quantized-mesh-1.0 terrain
 * format](https://github.com/CesiumGS/quantized-mesh).
 */
class STT_DLL stt::MeshTile: public Tile
{
    friend class MeshTiler;

public:
    /// create an empty mesh tile object
    MeshTile();

    /// create a mesh tile from a tile coordiante
    MeshTile(const TileCoordinate &coord);

    /// write terrain data to the filesystem
    void
    writeFile(const char *fileName, bool writeVertexNormals = false) const;

    /// writer terrain data to an output stream
    void
    writeFile(STTOutputStream &ostream, bool writeVertexNormals = false) const;

    /// does the terrain tile have child tiles?
    bool
    hasChildren() const;

    /// does the terrain tile have a south west child tile?
    bool
    hasChildSW() const;

    /// does the terrain tile have a south east child tile?
    bool
    hasChildSE() const;

    /// does the terrain tile have a north west child tile?
    bool
    hasChildNW() const;

    /// does the terrain tile have a north east child tile?
    bool
    hasChildNE() const;

    /// specify that there is a south west child tile
    void
    setChildSW(bool on = true);

    /// specify that there is a south east child tile
    void
    setChildSE(bool on = true);

    /// specify that there is a north west child tile
    void
    setChildNW(bool on = true);

    /// specify that there is a north east child tile
    void
    setChildNE(bool on = true);

    /// specify that all child tiles are present
    void
    setAllChildren(bool on = true);

    /// get the mesh data as a const object
    const stt::Mesh & getMesh() const;

    /// get the mesh data
    stt::Mesh & getMesh();

protected:
    /// the terrain mesh data
    stt::Mesh mMesh;

private:
    char mChildren;              /// the child flags

    /**
     * @brief bit flags defining child tile existence
     *
     * there is a good discussion on bitflags
     * [here](http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags").
     */
    enum Children {
        TERRAIN_CHILD_SW = 1,    // 2^0, bit 0
        TERRAIN_CHILD_SE = 2,    // 2^1, bit 1
        TERRAIN_CHILD_NW = 4,    // 2^2, bit 2
        TERRAIN_CHILD_NE = 8,    // 2^3, bit 3
    };
};

#endif /* MESHTILE_H_ */
