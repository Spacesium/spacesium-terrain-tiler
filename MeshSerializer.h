#ifndef MESHSERIALIZER_H_
#define MESHSERIALIZER_H_

/**
 * @file MeshSerializer.h
 * @brief this declares and defines the `MeshSerializer` class
 */

#include "config.h"
#include "TileCoordinate.h"
#include "MeshTile.h"

namespace stt {
    class MeshSerializer;
}

/// store `MeshTile`s from a GDAL dataset
class STT_DLL stt::MeshSerializer
{
public:
    /// start a new serialization task
    virtual void startSerialization() = 0;

    /// returns if the specified Tile Coordinate shoudl be serialized
    virtual bool mustSerializeCoordinate(const stt::TileCoordinate *coordinate) = 0;

    /// serialize a MeshTile to the store
    virtual bool serializeTile(const stt::MeshTile *tile, bool writeVertexNormals = false) = 0;

    /// serialization finished, releases any resources loaded
    virtual void endSerialization() = 0;
};

#endif /* MESHSERIALIZER_H_ */
