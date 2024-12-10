#ifndef MESHTILER_H_
#define MESHTILER_H_

/**
 * @file MeshTiler.h
 * @brief this declares the `MeshTiler` class
 */

#include "MeshTile.h"
#include "TerrainTiler.h"

namespace stt {
    class MeshTiler;
}

/**
 * @brief create `MeshTile`s from a GDAL dataset
 *
 * this class derives from `GDALTiler` and `TerrainTiler` enabling `MeshTile`s
 * to be created for a specific `TileCoordinate`.
 */
class STT_DLL stt::MeshTiler: public TerrainTiler
{
public:
    /// instantiate a tiler with all required arguments
    MeshTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options, double meshQualityFactor = 1.0):
        TerrainTiler(poDataset, grid, options),
        mMeshQualityFactor(meshQualityFactor)
    {}

    /// instantiate a tiler with an empty GDAL dataset
    MeshTiler(double meshQualityFactor = 1.0):
        TerrainTiler(),
        mMeshQualityFactor(meshQualityFactor)
    {}

    /// instantiate a tiler with a dataset and grid but no options
    MeshTiler(GDALDataset *poDataset, const Grid &grid, double meshQualityFactor = 1.0):
        TerrainTiler(poDataset, grid, TilerOptions()),
        mMeshQualityFactor(meshQualityFactor)
    {}

    /// overload the assignment operator
    MeshTiler &
    operator=(const MeshTiler &other);

    /// create a mesh from a tile coordinate
    MeshTile *
    createMesh(GDALDataset *dataset, const TileCoordinate &coord) const;

    /// create a mesh from a tile coordinate
    MeshTile *
    createMesh(GDALDataset *dataset, const TileCoordinate &coord, GDALDatasetReader *reader) const;

protected:
    // specifies the factor of the quality to convert terrain heightmaps to meshes.
    double mMeshQualityFactor;

    // determines an appropriate geometric error estimate when the goemetry comes from a heightmap.
    static double getEstimatedLevelZeroGeometricErrorForAHeightmap(
        double maximumRadius,
        double heightmapTerrainQuality,
        int tileWidth,
        int numberOfTilesAtLevelZero
    );

    /// assigns settings of Tile just to use.
    void prepareSettingsOfTile(
        MeshTile *tile,
        GDALDataset *dataset,
        const TileCoordinate &coord,
        float *rasterHeights,
        stt::i_tile tileSizeX,
        stt::i_tile tileSizeY
    ) const;
};

#endif /* MESHTILER_H_ */
