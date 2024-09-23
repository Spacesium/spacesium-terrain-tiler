/**
* @file GDALTile.cpp
* @brief this defines the `GDALTile` class
*/

#include "gdalwarper.h"
#include "GDALTile.h"

using namespace stt;

GDALTile::~GDALTile() {
    if (dataset != NULL) {
        GDALClose(dataset);

        if (transformer != NULL) {
            GDALDestroyGenImgProjTransformer(transformer);
        }
    }
}

/// detach the underlying GDAL dataset
GDALDataset *GDALTile::detach() {
    if (dataset != NULL) {
        GDALDataset *poDataset = dataset;
        dataset = NULL;

        if (transformer != NULL) {
            GDALDestroyGenImgProjTransformer(transformer);
            transformer = NULL;
        }
        return poDataset;
    }
    return NULL;
}
