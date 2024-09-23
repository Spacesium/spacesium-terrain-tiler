/**
* @file GlobalMercator.cpp
* @brief this defines the `GlobalMercator` class
*/

#define _USE_MATH_DEFINES

#include "GlobalMercator.h"

using namespace stt;

// set the class level properties
const unsigned int GlobalMercator::cSemiMajorAxis = 6378137;
const double GlobalMercator::cEarthCircumference = 2 * M_PI * GlobalMercator::cSemiMajorAxis;
const double GlobalMercator::cOriginShift = GlobalMercator::cEarthCircumference / 2.0;

// set the spatial reference
static OGRSpatialReference
setSRS(void)
{
    OGRSpatialReference srs;

    #if (GDAL_VERSION_MAJOR >= 3)
    srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    #endif

    srs.importFromEPSG(3857);
    return srs;
}

const OGRSpatialReference GlobalMercator::cSRS = setSRS();
