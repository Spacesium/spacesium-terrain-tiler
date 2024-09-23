/**
* @file GlobalGeodetic.cpp
* @brief this defines the `GlobalGeodetic` class
*/

#include "GlobalGeodetic.h"

using namespace stt;

// set the spatial reference
static OGRSpatialReference setSRS(void)
{
    OGRSpatialReference srs;

    #if (GDAL_VERSION_MAJOR >= 3)
    srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    #endif

    srs.importFromEPSG(4326);
    return srs;
}

const OGRSpatialReference GlobalGeodetic::cSRS = setSRS();
