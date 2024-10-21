#ifndef STTFILETILESERIALIZER_H_
#define STTFILETILESERIALIZER_H_

/**
 * @file STTFileTileSerializer.h
 * @brief this declares and defines the `STTFileTileSerializer` class
 */

#include <string>

#include "TileCoordinate.h"
#include "GDALSerializer.h"
#include "TerrainSerializer.h"
#include "MeshSerializer.h"

namespace stt {
    class STTFileTileSerializer;
}

/// implements a serializer `Tile` based in a directory of files
class STT_DLL stt::STTFileTileSerializer :
    public stt::GDALSerializer,
    public stt::TerrainSerializer,
    public stt::MeshSerializer
{
public:
    STTFileTileSerializer(const std::string &outputDir, bool resume):
        moutputDir(outputDir),
        mresume(resume)
    {}

    /// start a new serialization task
    virtual void startSerialization() {};

    /// returns if the specified Tile Coordiante should be serialized
    virtual bool mustSerializeCoordinate(const stt::TileCoordinate *coordinate);

    /// serilize a GDALTile to the store
    virtual bool serializeTile(
        const stt::GDALTile *tile,
        GDALDriver *driver,
        const char *extension,
        CPLStringList &creationOptions
    );

    /// serialize a TerrainTile to the store
    virtual bool serializeTile(const stt::TerrainTile *tile);

    /// serialize a MeshTile to the store
    virtual bool serializeTile(
        const stt::MeshTile *tile,
        bool writeVertexNormals = false
    );

    /// serialization finished, releases any resources loaded
    virtual void endSerialization() {};

    /// create a filename for a tile coordinate
    static std::string getTileFilename(
        const TileCoordinate *coord,
        const std::string dirname,
        const char *extension
    );

protected:
    /// the target directory where serializing
    std::string moutputDir;

    /// do not overwrite existing files
    bool mresume;
};

#endif /* STTFILETILESERIALIZER_H_ */
