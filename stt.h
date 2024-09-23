#ifndef STT_H_
#define STT_H_

/**
 * @file stt.h
 * @brief all required definitions for working with `libstt`
 *
 * @mainpage Spacesium Terrain Tiler (libstt)
 *
 * `libstt` is a C++ library used to create terrain tiles for use in the
 * [Cesium Jabascript Library](https://cesium.com/platform/cesiumjs/).
 * 
 * the library does not provide a way of serving up or storing the resulting
 * tiles: this is application specific. its aim is simply to take a
 * [GDAL](https://gdal.org/en/latest/) compatible raster representing
 * a digital terrain model (DTM) and convert this to terrain tiles.
 * see the tools provided with the library for example on how the library
 * is used to achieve this.
 *
 * to use the library include `stt.h` e.g.
 *
 * \code
 * // test.cpp
 * #include <iostream>
 * #include "stt.h"
 *
 * using namespace std;
 *
 * int main() {
 *   cout << "using libstt version "
 *        << stt::version.major << "."
 *        << stt::version.minor << "."
 *        << stt::version.patch << endl;
 *
 *   return 0;
 * }
 * \endcode
 *
 * assuming the library is installed on the system and you are using the `g++`
 * compiler, the aboce can be compiled using:
 *
 * \code{.sh}
 * g++ -lstt -o test test.cpp
 * \endcode
 *
 * ## implementation overview
 *
 * the concept of a grid is implemented in the `stt:Grid` class. the TMS
 * Global Geodetic and Global Mercator profiles are specialisations of the grid
 * implemented in the `stt:GlobalGeodetic` and `stt:GlobalMercator` classes.
 * these classes define the tiling scheme which is the used to cut up GDAL
 * rasters into the output tiles.
 *
 * the `stt::GDALTiler` and `stt::TerrainTiler` classes composes an instance of
 * a grid with a GDAL raster dataset. they use the dataset to determine the
 * native raster resolution and extent. once this is known the appropriate zoom
 * levels and tile coverage can be calculated from the grid. for each tile an
 * in memory GDAL [Virtual Raster](https://gdal.org/en/latest/drivers/raster/vrt.html)
 * (VRT) can then be generated. this is a lightweight representation of the
 * relevant underlying data necessary to create populate the tile. the VRT
 * can then be used to generate an actual `stt::TerrainTile` instance or
 * raster dataset which can then be stored as required by the application.
 *
 * there are various iterator classes providing convenient iteration over
 * tilesets created by grids and tilers. for instance the
 * `stt::TerrainIterator` class provides a simple interface for iterating over
 * all valid tiles represented by a `stt::TerrainTiler`, and likewise the
 * `stt:RasterIterator` over a `stt::GDALTiler` instance.
 *
 * see the `README.md` file distributed with the source code for further details.
 */

#include "stt/Bounds.h"
#include "stt/Coordinate.h"
#include "stt/STTException.h"
#include "stt/GDALTile.h"
#include "stt/GDALTiler.h"
#include "stt/GlobalGeodetic.h"
#include "stt/GlobalMercator.h"
#include "stt/Grid.h"
#include "stt/GridIterator.h"
#include "stt/RasterIterator.h"
#include "stt/RasterTiler.h"
#include "stt/TerrainIterator.h"
#include "stt/TerrainTile.h"
#include "stt/TerrainTiler.h"
#include "stt/TileCoordinate.h"
#include "stt/Tile.h"
#include "stt/TilerIterator.h"
#include "stt/types.h"

#endif /* STT_H_ */
