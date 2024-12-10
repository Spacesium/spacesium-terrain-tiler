#ifndef STTMESH_H_
#define STTMESH_H_

/**
 * @file Mesh.h
 * @brief this declares the `Mesh` class
 */

#include <cstdint>
#include <vector>
#include <cstring>

#include "types.h"
#include "STTException.h"

namespace stt {
    class Mesh;
}

/**
 * @brief an abastract base class for a mesh of triangles
 */
class stt::Mesh
{
public:
    /// create an empty mesh
    Mesh() {}

    /// the array of shared vertices of a mesh
    std::vector<CRSVertex> vertices;

    /// the index collection for each triangle in the mesh (3 for each triangle)
    std::vector<uint32_t> indices;

    /// write mesh data to a WKT file
    void writeWktFile(const char *fileName) const {
        FILE *fp = fopen(fileName, "w");

        if (fp == NULL) {
            throw STTException("Failed to open file");
        }

        char wktText[512];
        memset(wktText, 0, sizeof(wktText));

        for (int i = 0, icount = indices.size(); i < icount; i += 3) {
            CRSVertex v0 = vertices[indices[i]];
            CRSVertex v1 = vertices[indices[i + 1]];
            CRSVertex v2 = vertices[indices[i + 2]];

            sprintf(wktText, "(%.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f)",
                v0.x, v0.y, v0.z,
                v1.x, v1.y, v1.z,
                v2.x, v2.y, v2.z,
                v0.x, v0.y, v0.z
            );
            fprintf(fp, "POLYGON Z(%s)\n", wktText);
        }
        fclose(fp);
    };
};

#endif /* STTMESH_H_ */
