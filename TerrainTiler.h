#ifndef TERRAINTILER_H_
#define TERRAINTILER_H_

/**
 * @file TerrainTiler.h
 * @brief this declares the `TerrainTiler` class
 */

namespace stt {
    class TerrainTiler;
}

/**
 * @brief create `TerrainTiler`s from a GDAL Dataset
 *
 * this class derives from `GDALTiler` and adds the `GDALTiler::createTerrainTile`
 * method enabling `TerrainTile`s to be created for a specific `TileCoordiante`.
 */
class STT_DLL stt::TerrainTiler: public GDALTiler
{
public:
    /// instantiate a tiler with all required arguments
    TerrainTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
        GDALTiler(poDataset, grid, options)
    {}

    /// instantiate a tiler with an empty GDAL dataset
    TerrainTiler(): GDALTiler() {}

    /// instantiate a tiler with a dataset and grid but no options
    TerrainTiler(GDALDataset *poDataset, const Grid &grid):
        TerrainTiler(poDataset, grid, TilerOptions())
    {}

    /// overload the assignment operator
    TerrainTiler &
    operator=(const TerrainTiler &other);

    /// override to return a covariant data type
    TerrainTile *
    createTile(GDALDataset *dataset, const TileCoordinate &coord) const override;

    /// create a tile from a tile coordinate
    TerrainTile *
    createTile(GDALDataset *dataset, const TileCoordiante &coord, GDALDatasetReader *reader) const;

protected:
    /// create a `GDALTile` representing the rquired terrain tile data
    virtual GDALTile *
    createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const override;

    /**
     * @brief get terrain bounds shifted to introduce a pixel overlap
     *
     * given a `TileCoordinate`, this sets the resolution and returns latitude
     * and longitude bounds for a tile which include a pixel's worth of data
     * outside the actual tile bounds to both the east and the north. this is
     * used to satisfy the terrain heightmap specification of terrain tiles
     * including a pixel's workth of data from surrounding tiles.
     *
     * @param coord the tile coordinate identifying the tile in question
     * @param resolution the resolution of the modified extent is set here
     */
    inline CRSBounds
    terrainTileBounds(const TileCoordinate &coord, double &resolution) const {
        // the actual tile size accounting for a border
        i_tile lTileSize = mGrid.tileSize() - 1;
        CRSBounds tile = mGrid.tileBounds(coord); // the actual tile bounds

        // get the resolution for the dataset without a border
        resolution = (tile.getMax() - tile.getMinX()) / lTileSize;

        // extend the easting by one pixel's worth
        tile.setMinX(tile.getMinX() - resolution);

        // extend the northing by one pixel's worth
        tile.setMaxY(tile.getMaxY() - resolution);

        return tile;
    }

    /// assigns settings of Tile just to use.
    void prepareSettingsOfTile(
        TerrainTile *tile,
        const TileCoordinate &coord,
        float *rasterHeights,
        stt::i_tile tileSizeX,
        stt::i_tile tileSizeY
    ) const;
};

#endif /* TERRAINTILER_H_ */
