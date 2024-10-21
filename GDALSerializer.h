#ifndef GDALSERIALIZER_H_
#define GDALSERIALIZER_H_

/**
 * @file GDALSerializer.h
 * @brief this declares and defines the `GDALSerializer` class
 */

#include "config.h"
#include "TileCoordinate.h"
#include "GDALTile.h"

#include "cpl_string.h"

namespace stt {
    class GDALSerializer;
}

/// store `GDALTile`s from a GDAL Dataset
class STT_DLL stt::GDALSerializer {
public:
    /// start a new serialization task
    virtual void startSerialization() = 0;

    /// returns if the specified Tile Coordiante shoudl be serialized
    virtual bool mustSerializeCoordinate(const stt::TileCoordinate *coordinate) = 0;

    /// serialize a GDALTile to the store
    virtual bool serializeTile(
        const stt::GDALTile *tile,
        GDALDriver *driver,
        const char *extension,
        CPLStringList &creationOptions
    ) = 0;

    /// serialization finished, releases any resources loaded
    virtual void endSerialization() = 0;
};

#endif /* GDALSERIALIZER_H_ */
