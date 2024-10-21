#ifndef TERRAINSERIALIZER_H_
#define TERRAINSERIALIZER_H_

/**
 * @file TerrainSerializer.h
 * @brief this declares and defines the `TerrainSerializer` class
 */

#include "config.h"
#include "TileCoordinate.h"
#include "TerrainTile.h"

namespace stt {
    class TerrainSerializer;
}

/// store `TerrainTile`s from a GDAL Dataset
class STT_DLL stt::TerrainSerializer {
public:
    /// start a new serialization task
    virtual void startSerialization() = 0;

    /// returns if the specified Tile Coordiante should be serialized
    virtual bool mustSerializeCoordinate(const stt::TileCoordinate *coordinate) = 0;

    /// serialize a TerrainTile to the store
    virtual bool serializeTile(const stt::TerrainTile *tile) = 0;

    /// serialization finished, releases any resources loaded
    virtual void endSerialization() = 0;
};

#endif /* TERRAINSERIALIZER_H_ */

