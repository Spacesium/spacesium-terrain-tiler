#ifndef GDAL_TILER_H_
#define GDAL_TILER_H_

/**
 * @file GDALTiler.h
 * @brief this declares the `GDALTiler` class
 */

#include <string>
#include "gdalwarper.h"

#include "TileCoordinate.h"
#include "GlobalGeodetic.h"
#include "GDALTile.h"
#include "Bounds.h"

namespace stt {
    struct TilerOptions;
    class GDALTiler;
    class GDALDatasetReader; // forward declaration
}

/// options passed to a `GDALTiler`
struct stt::TilerOptions {
    /// the error threshold in pixels passed to the approximation transformer
    float errorThreshold = 0.125;  // the `gdalwarp` default
    /// the memory limit of the warper in bytes
    double warpMemoryLimit = 0.0;  // default to GDAL internal setting
    /// the warp resampling algorithm
    GDALResampleAlg resampleAlg = GRA_Average; // recommended by GDAL maintainer
};

/**
 * @brief create raster tiles from GDAL Dataset
 *
 * this abstract base class is associated with a GDAL dataset from which it
 * determines the maximum zoom level (see `GDALTiler::maxZoomLevel`) and tile
 * extents for a particular zoom level (see `GDALTiler::tileBoundsForZoom`).
 * this information can be used to create `TileCoordinate` instances which in
 * turn can be used to create raster representations of a tile coverage (see
 * `GDALTiler::createRasterTile`). this mechanism is intentded to be leveraged
 * by derived classes to override the `GDALTiler::createTile` method.
 *
 * the GDAL dataset assigned to the tiler has its references count incremented
 * when a tiler is instantiated or copied, meaning that the dataset is shared
 * with any other handles that may also be in use. when the tiler is destroyed
 * the reference count is decremented and, if it reaches `0`, the dataset is
 * closed.
 */
class STT_DLL stt::GDALTiler
{
public:

    /// instantiate a tiler with all required arguments
    GDALTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options);

    /// instantiate a tiler with an empty GDAL dataset
    GDALTiler(): GDALTiler(NULL, GlobalGeodetic()) {}

    /// instantiate a tiler with a dataset and grid but no options
    GDALTiler(GDALDataset *poDataset, const Grid &grid):
        GDALTiler(poDataset, grid, TilerOptions()) {}

    /// the const copy constructor
    GDALTiler(const GDALTiler &other);

    /// the non const copy constructor
    GDALTiler(GDALTiler &other);

    /// overload the assignment operator
    GDALTiler &operator=(const GDALTiler &other);

    /// the destructor
    ~GDALTiler();

    /// create a tile from a tile coordinate
    virtual Tile *
    createTile(GDALDataset *dataset, const TileCoordinate &coord) const = 0;

    /// get the maximum zoom level for the dataset
    inline i_zoom
    maxZoomLevel() const {
        return mGrid.zoomForResolution(resolution());
    }

    /// get the lower left tile for a particular zoom level
    inline TileCoordinate
    lowerLeftTile(i_zoom zoom) const {
        return mGrid.crsToTile(mBounds.getLowerLeft(), zoom);
    }

    /// get the upper right tile for a particular zoom level
    inline TileCoordinate
    upperRightTile(i_zoom zoom) const {
        return mGrid.crsToTile(mBounds.getUpperRight(), zoom);
    }

    /// get the tile bounds for a particular zoom level
    inline TileBounds
    tileBoundsForZoom(i_zoom zoom) const {
        TileCoordinate ll = mGrid.crsToTile(mBounds.getLowerLeft(), zoom);
        TileCoordinate ur = mGrid.crsToTile(mBounds.getUpperRight(), zoom);

        return TileBounds(ll, ur);
    }

    /// get the resolution of the underlying GDAL dataset
    inline double
    resolution() const {
        return mResolution;
    }

    /// get the associated GDAL dataset
    inline GDALDataset *
    dataset() const {
        return poDataset;
    }

    /// get the associated grid
    inline const Grid &
    grid() const {
        return mGrid;
    }

    /// get the dataset bounds in EPSG:4326 coordinates
    inline const CRSBounds &
    bounds() const {
        return const_cast<const CRSBounds &>(mBounds);
    }

    /// does the dataset require reprojecting to EPSG:4326?
    inline bool
    requiresReprojection() const {
        return crsWKT.size() > 0;
    }

protected:
    friend class GDALDatasetReader;

    /// close the underlying dataset
    void closeDataset();

    /// create a raster tiel from a tile coordinate
    virtual GDALTile *
    createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const;

    /// create raster tile from a geotransform
    virtual GDALTile *
    createRasterTile(GDALDataset *dataset, double (&adfGeoTransform)[6]) const;

    /// the grid used for generating tiles
    Grid mGrid;

    /// the dataset from which to generate tiles
    GDALDataset *poDataset;

    TilerOptions options;

    /// the extent of the underlying dataset in latitude and longitude
    CRSBounds mBounds;

    /// the cell resolution of the underlying dataset
    double mResolution;

    /**
     * @brief the dataset projection in well known text format
     *
     * this is only set if the underlying dataset does not match the coordiante
     * reference system of the grid being used.
     */
    std::string crsWKT;
};

#endif /* GDAL_TILER_H_ */
