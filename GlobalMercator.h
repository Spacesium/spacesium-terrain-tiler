#ifndef GLOBALMERCATOR_H_
#define GLOBALMERCATOR_H_

/**
* @file GlobalMercator.h
* @brief this defines the `GlobalMercator` class
*/

#define _USE_MATH_DEFINES // for M_PI

#include "config.h"
#include "Grid.h"

namespace stt {
    class GlobalMercator;
}

/**
 * @brief an implementation of the TMS Global Mercator Profile
 *
 * this class models the Tile Mapping Service Global Mercator Profile
 */

class STT_DLL stt::GlobalMercator: public Grid
{
public:
    GlobalMercator(i_tile tileSize = 256):
        Grid(
            tileSize,
            CRSBounds(-cOriginShift, -cOriginShift, cOriginShift, cOriginShift),
            cSRS
        )
    {}

protected:
    /// the semi major axis of the WGS84 ellipsoid (the radius of the earth in meters)
    static const unsigned int cSemiMajorAxis;

    /// the circumference of the earth in meters
    static const double cEarthCircumference;

    /// the coordiante origin (the middle of the grid extent)
    static const double cOriginShift;

    /// the EPSG:3785 spatial reference system
    static const OGRSpatialReference cSRS;
};

#endif /* GLOBALMERCATOR_H_ */
