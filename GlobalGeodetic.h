#ifndef GLOBALGEODETIC_H_
#define GLOBALGEODETIC_H_

/**
 * @file GlobalGeodetic.h
 * @brief this defines and declares the `GlobalGeodetic` class
 */

#include "config.h"
#include "Grid.h"

namespace stt {
    class GlobalGeodetic;
}

/**
 * @brief an implementation of the TMS Global Geodetic Profile
 *
 * this class models the [Tile Mapping Service Global Geodetic Profile]
 */
class STT_DLL stt::GlobalGeodetic: public Grid
{
public:
    /// initialize the profile with a specific tile size
    GlobalGeodetic(i_tile tileSize = TILE_SIZE, bool tmsCompatible = true):
        Grid(tileSize,
            CRSBounds(-180, -90, 180, 90),
            cSRS,
            (tmsCompatible) ? 2 : 1
        )
    {}

protected:
    /// the EPSG:4326 spatial reference system
    static const OGRSpatialReference cSRS;
};

#endif /* GLOBALGEODETIC_H_ */
