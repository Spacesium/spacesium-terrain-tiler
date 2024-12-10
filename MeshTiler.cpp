/**
* @file MeshTiler.cpp
* @brief this defines the `MeshTiler` class
*/

#include "STTException.h"
#include "MeshTiler.h"
#include "HeightFieldChunker.h"
#include "GDALDatasetReader.h"

using namespace stt;

////////////////////////////////////////////////////////////////////////////////

/**
* implementation of stt::chunk::mesh for stt::Mesh class
*/
class WrapperMesh : public stt::chunk::mesh
{
private:
    CRSBounds &mBounds;
    Mesh &mMesh;
    double mCellSizeX;
    double mCellSizeY;

    std::map<int, int> mIndicesMap;
    Coordinate<int> mTriangles[3];
    bool mTriOddOrder;
    int mTriIndex;

public:
    WrapperMesh(CRSBounds &bounds, Mesh &mesh, i_tile tileSizeX, i_tile tileSizeY):
        mMesh(mesh),
        mBounds(bounds),
        mTriOddOrder(false),
        mTriIndex(0)
    {
        mCellSizeX = (bounds.getMaxX() - bounds.getMinX()) / (double)(tileSizeX - 1);
        mCellSizeY = (bounds.getMaxY() - bounds.getMinY()) / (double)(tileSizeY - 1);
    }

    virtual void clear() {
        mMesh.vertices.clear();
        mMesh.indices.clear();
        mIndicesMap.clear();
        mTriOddOrder = false;
        mTriIndex = 0;
    }

    virtual void emit_vertex(const stt::chunk::heightfield &heightfield, int x, int y) {
        mTriangles[mTriIndex].x = x;
        mTriangles[mTriIndex].y = y;
        mTriIndex++;

        if (mTriIndex == 3) {
            mTriOddOrder = !mTriOddOrder;

            if (mTriOddOrder) {
                appendVertex(heightfield, mTriangles[0].x, mTriangles[0].y);
                appendVertex(heightfield, mTriangles[1].x, mTriangles[1].y);
                appendVertex(heightfield, mTriangles[2].x, mTriangles[2].y);
            } else {
                appendVertex(heightfield, mTriangles[1].x, mTriangles[1].y);
                appendVertex(heightfield, mTriangles[0].x, mTriangles[0].y);
                appendVertex(heightfield, mTriangles[2].x, mTriangles[2].y);
            }
            mTriangles[0].x = mTriangles[1].x;
            mTriangles[0].y = mTriangles[1].y;
            mTriangles[1].x = mTriangles[2].x;
            mTriangles[1].y = mTriangles[2].y;
            mTriIndex--;
        }
    }

    void appendVertex(const stt::chunk::heightfield &heightfield, int x, int y) {
        int iv;
        int index = heightfield.indexOfGridCoordinate(x, y);

        std::map<int, int>::iterator it = mIndicesMap.find(index);

        if (it == mIndicesMap.end()) {
            iv = mMesh.vertices.size();

            double xmin = mBounds.getMinX();
            double ymax = mBounds.getMaxY();
            double height = heightfield.height(x, y);

            mMesh.vertices.push_back(CRSVertex(xmin + (x * mCellSizeX), ymax - (y * mCellSizeY), height));
            mIndicesMap.insert(std::make_pair(index, iv));
        } else {
            iv = it->second;
        }
        mMesh.indices.push_back(iv);
    }
};

////////////////////////////////////////////////////////////////////////////////

void stt::MeshTiler::prepareSettingsOfTile(MeshTile *terrainTile, GDALDataset *dataset,
    const TileCoordinate &coord, float *rasterHeights, stt::i_tile tileSizeX,
    stt::i_tile tileSizeY) const
{
    const stt::i_tile TILE_SIZE = tileSizeX;

    // number of tiles in the horizontal direction at tile level zero
    double resolutionAtLevelZero = mGrid.resolution(0);
    int numberOfTilesAtLevelZero = (int)(mGrid.getExtent().getWidth() / (tileSizeX * resolutionAtLevelZero));

    // default quality of terrain created from heightmaps (TerrainProvider.js).
    double heightmapTerrainQuality = 0.25;

    // earth semi-major-axis in meters
    const double semiMajorAxis = 6378137.0;

    // appropriate geometric error estimate when the geometry comes from a
    // heightmap (TerrainProvider.js).
    double maximumGeometricError = MeshTiler::getEstimatedLevelZeroGeometricErrorForAHeightmap(
        semiMajorAxis,
        heightmapTerrainQuality * mMeshQualityFactor,
        TILE_SIZE,
        numberOfTilesAtLevelZero
    );

    // geometric error for current level
    maximumGeometricError /= (double)(1 << coord.zoom);

    // convert the raster grid into an irregular mesh applying the
    // Chunked LOD strategy by 'Thatcher Ulrich'.
    // http://tulrich.com/geekstuff/chunklod.html

    stt::chunk::heightfield heightfield(rasterHeights, TILE_SIZE);
    heightfield.applyGeometricError(maximumGeometricError, coord.zoom <= 6);

    // propagate the geometric error of neighbors to avoid gaps in borders.
    if (coord.zoom > 6) {
        stt::CRSBounds datasetBounds = bounds();

        for (int borderIndex = 0; borderIndex < 4; borderIndex++) {
            bool okNeighborCoord = true;
            stt::TileCoordinate neighborCoord = stt::chunk::heightfield::neighborCoord(
                mGrid,
                coord,
                borderIndex,
                okNeighborCoord
            );

            if (!okNeighborCoord)
                continue;

            stt::CRSBounds neighborBounds = mGrid.tileBounds(neighborCoord);

            if (datasetBounds.overlaps(neighborBounds)) {
                float *neighborHeights = stt::GDALDatasetReader::readRasterHeights(
                    *this,
                    dataset,
                    neighborCoord,
                    mGrid.tileSize(),
                    mGrid.tileSize()
                );

                stt::chunk::heightfield neighborHeightfield(neighborHeights, TILE_SIZE);
                neighborHeightfield.applyGeometricError(maximumGeometricError);
                heightfield.applyBorderActivationState(neighborHeightfield, borderIndex);

                CPLFree(neighborHeights);
            }
        }
    }

    stt::CRSBounds mGridBounds = mGrid.tileBounds(coord);
    Mesh &tileMesh = terrainTile->getMesh();
    WrapperMesh mesh(mGridBounds, tileMesh, tileSizeX, tileSizeY);
    heightfield.generateMesh(mesh, 0);
    heightfield.clear();

    // if we are not at the maximum zoom level we need to set child flags on
    // the tile where child tiles overlap the dataset bounds.
    if (coord.zoom != maxZoomLevel()) {
        CRSBounds tileBounds = mGrid.tileBounds(coord);

        if (! (bounds().overlaps(tileBounds))) {
            terrainTile->setAllChildren(false);
        } else {
            if (bounds().overlaps(tileBounds.getSW())) {
                terrainTile->setChildSW();
            }
            if (bounds().overlaps(tileBounds.getNW())) {
                terrainTile->setChildNW();
            }
            if (bounds().overlaps(tileBounds.getNE())) {
                terrainTile->setChildNE();
            }
            if (bounds().overlaps(tileBounds.getSE())) {
                terrainTile->setChildSE();
            }
        }
    }
}

MeshTile * stt::MeshTiler::createMesh(GDALDataset *dataset, const TileCoordinate &coord) const
{
    // copy the raster data into an array
    float *rasterHeights = stt::GDALDatasetReader::readRasterHeights(
        *this,
        dataset,
        coord,
        mGrid.tileSize(),
        mGrid.tileSize()
    );

    // get a mesh tile represented by the tile coordinate
    MeshTile *terrainTile = new MeshTile(coord);
    prepareSettingsOfTile(terrainTile, dataset, coord, rasterHeights, mGrid.tileSize(), mGrid.tileSize());
    CPLFree(rasterHeights);

    return terrainTile;

}

MeshTile * stt::MeshTiler::createMesh(
    GDALDataset *dataset,
    const TileCoordinate &coord,
    stt::GDALDatasetReader *reader) const
{
    // copy the raster data into an array
    float *rasterHeights = reader->readRasterHeights(
        dataset,
        coord,
        mGrid.tileSize(),
        mGrid.tileSize()
    );

    // get a mesh tile represented by the tile coordinate
    MeshTile *terrainTile = new MeshTile(coord);
    prepareSettingsOfTile(terrainTile, dataset, coord, rasterHeights, mGrid.tileSize(), mGrid.tileSize());
    CPLFree(rasterHeights);

    return terrainTile;
}

MeshTiler & stt::MeshTiler::operator=(const MeshTiler &other)
{
    TerrainTiler::operator=(other);

    return *this;
}

double stt::MeshTiler::getEstimatedLevelZeroGeometricErrorForAHeightmap(
    double maximumRadius,
    double heightmapTerrainQuality,
    int tileWidth,
    int numberOfTilesAtLevelZero)
{
    double error = maximumRadius * 2 * M_PI * heightmapTerrainQuality;
    error /= (double)(tileWidth * numberOfTilesAtLevelZero);

    return error;
}

