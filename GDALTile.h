#ifndef GDALTILE_H_
#define GDALTILE_H_

/**
 * @file GDALTile.h
 * @brief this declares the `GDALTile` class
 */

#include "gdal_priv.h"

#include "config.h"    // for STT_DLL
#include "Tile.h"

namespace stt {
    class GDALTile;
    class GDALTiler;   // forward declaration
}

/**
 * a representation of a `Tile` with a GDAL datasource
 *
 * this is composed of a GDAL VRT datasource and optionally a GDAL image
 * transformer, along with a `TileCoordinate`. the transformer handle is
 * necessary in cases where the VRT is warped using a linear approximation
 * (`GDALApproxTransform`). in this case there is the top level transformer (the
 * linear approximation) which wraps an image transformer. the VRT owns any top
 * level transformer, but we are responsible for the wrapped image transformer.
 */

class STT_DLL stt::GDALTile: public Tile
{
public:
    /// take ownership of a dataset and optional transformer
    GDALTile(GDALDataset *dataset, void *transformer):
        Tile(), dataset(dataset), transformer(transformer)
    {}

    ~GDALTile();

    GDALDataset *dataset;

    /// detach the underlying GDAL dataset
    GDALDataset *detach();

protected:
    friend class GDALTiler;

    /// the image to image transformer
    void *transformer;
};

#endif /* GDALTILE_H_ */

