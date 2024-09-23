#ifndef STTGRID_H_
#define STTGRID_H_

/**
 * @file Grid.h
 * @brief this defines and declares the `Grid` class
 */

#include <cmath>
#include <ogr_spatialref.h>

#include "types.h"
#include "TileCoordinate.h"

namespace stt {
    class Grid;
}

class stt::Grid
{
public:
    /// an empty grid
    Grid() {}

    /// initialize a grid tile
    Grid(
        i_tile tileSize,
        const CRSBounds extent,
        const OGRSpatialReference srs,
        unsigned short int rootTiles = 1,
        float zoomFactor = 2
    ):
        mTileSize(tileSize),
        mExtent(extent),
        mSRS(srs),
        mInitialResolution((extent.getWidth() / rootTiles) / tileSize),
        mXOriginShift(extent.getWidth() / 2),
        mYOriginShift(extent.getWidth() / 2),
        mZoomFactor(zoomFactor)
    {
        #if (GDAL_VERSION_MAJOR >= 3)
        mSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        #endif
    }

    /// overload the assignment operator
    Grid &
    operator=(const Grid &other)
    {
        mTileSize = other.mTileSize;
        mExtent = other.mExtent;
        mSRS = other.mSRS;
        mInitialResolution = other.mInitialResolution;
        mXOriginShift = other.mXOriginShift;
        mYOriginShift = other.mYOriginShift;
        mZoomFactor = other.mZoomFactor;

        return *this;
    }

    /// override the equality operator
    bool
    operator==(const Grid &other) const {
        return mTileSize == other.mTileSize
        && mExtent == other.mExtent
        && mSRS.IsSame(&(other.mSRS))
        && mInitialResolution == other.mInitialResolution
        && mXOriginShift == other.mXOriginShift
        && mYOriginShift == other.mYOriginShift
        && mZoomFactor == other.mZoomFactor;
    }

    /// get the resolution for a particular zoom level
    inline double
    resolution(i_zoom zoom) const {
        return mInitialResolution / pow(mZoomFactor, zoom);
    }

    /**
    * @brief get the zoom level for a particular resolution
    *
    * if the resolution does not exactly match a zoom level then the zoom
    * level is 'rounded up' to the next level
    */
    inline i_zoom
    zoomForResolution(double resolution) const {
        /// if mZoomFactor == 2 the following is the same as using:
        /// log2(mInitialResolution) - log2(resolution)
        return (i_zoom) ceil(
              (log(mInitialResolution) / log(mZoomFactor)) - 
              (log(resolution) / log(mZoomFactor))
        );
    }

    /// get the tile covering a pixel location
    inline TilePoint
    pixelsToTile(const PixelPoint &pixel) const {
        i_tile tx = (i_tile) (pixel.x / mTileSize);
        i_tile ty = (i_tile) (pixel.y / mTileSize);

        return TilePoint(tx, ty);
    }

    /// convert pixel coordinates at a given zoom level to CRS coordinates
    inline CRSPoint
    pixelsToCrs(const PixelPoint &pixel, i_zoom zoom) const {
        double res = resolution(zoom);

        return CRSPoint(
            (pixel.x * res) - mXOriginShift,
            (pixel.y * res) - mYOriginShift
        );
    }

    /// get the pixel location represented by a CRS point and zoom level
    inline PixelPoint
    crsToPixels(const CRSPoint &coord, i_zoom zoom) const {
        double res = resolution(zoom);
        i_pixel px = (mXOriginShift + coord.x) / res;
        i_pixel py = (mYOriginShift + coord.y) / res;

        return PixelPoint(px, py);
    }

    /// get the tile coordinate in which a location falls at specific zoom level
    inline TileCoordinate
    crsToTile(const CRSPoint &coord, i_zoom zoom) const {
        const PixelPoint pixel = crsToPixels(coord, zoom);
        TilePoint tile = pixelsToTile(pixel);

        return TileCoordinate(zoom, tile);
    }

    /// get the CRS bounds of a particular tile
    inline CRSBounds
    tileBounds(const TileCoordinate &coord) const {
        /// get the pixels coordinates representing the tile bounds
        const PixelPoint pxLowerLeft(
            coord.x * mTileSize,
            coord.y * mTileSize
        );
        const PixelPoint pxUpperRight(
            (coord.x + 1) * mTileSize,
            (coord.y + 1) * mTileSize
        );

        /// convert pixels to native coordinates
        const CRSPoint lowerLeft = pixelsToCrs(pxLowerLeft, coord.zoom);
        const CRSPoint upperRight = pixelsToCrs(pxUpperRight, coord.zoom);

        return CRSBounds(lowerLeft, upperRight);
    }

    /// get the tile size associated with this grid
    inline i_tile
    tileSize() const {
        return mTileSize;
    }

    /// get the srs associated with this grid
    inline const OGRSpatialReference &
    getSRS() const {
        return mSRS;
    }

    /// get the extent covered by the grid in CRS coordinates
    inline const CRSBounds &
    getExtent() const {
        return mExtent;
    }

    /// get the extent covered by the grid in tile coordinates for a zoom level
    inline TileBounds
    getTileExtent(i_zoom zoom) const {
        TileCoordinate ll = crsToTile(mExtent.getLowerLeft(), zoom);
        TileCoordinate ur = crsToTile(mExtent.getUpperRight(), zoom);

        return TileBounds(ll, ur);
    }

protected:
    /// the tile size associated with this grid
    i_tile mTileSize;

    /// the area covered by the grid
    CRSBounds mExtent;

    /// the spatial reference system covered by the grid
    OGRSpatialReference mSRS;

    double mInitialResolution; /// the initial resolution of this particular profile
    double mXOriginShift;      /// the shift in CRS coordinates to get to the origin from minx
    double mYOriginShift;      /// the shift in CRS coordinates to get to the origin from miny

    /// by what factor will the scale increase at each zoom level
    float mZoomFactor;
};

#endif /* STTGRID_H_ */
