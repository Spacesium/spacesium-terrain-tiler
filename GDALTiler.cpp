/**
* @file GDALTiler.cpp
* @brief this defines the `GDALTiler` class
*/

#include <cmath>         // std::abs
#include <algorithm>     // std::minmax
#include <cstring>       // strlen
#include <mutex>

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "ogr_spatialref.h"
// #include "ogr_srs_api.h"

#include "config.h"
#include "STTException.h"
#include "GDALTiler.h"

using namespace stt;

GDALTiler::GDALTiler(GDALDataset *poDataset, const Grid &grid, const TilerOptions &options):
    mGrid(grid),
    poDataset(poDataset),
    options(options)
{
    // transformed bounds can give slightly different results on different
    // threads unless mutexed
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    // if the dataset is set we need to initialize the tile bounds and raster
    // resolution from it.
    if (poDataset != NULL) {

        // get the bounds of the dataset
        double adfGeoTransform[0];
        CRSBounds bounds;

        if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            bounds = CRSBounds(
                adfGeoTransform[0],
                adfGeoTransform[3] + (poDataset->GetRasterYSize() * adfGeoTransform[5]),
                adfGeoTransform[0] + (poDataset->GetRasterXSize() * adfGeoTransform[1]),
                adfGeoTransform[3],
        } else {
            throw STTException("Could not get transformation information from source dataset");
        }

        // find out whether the dataset SRS matches that of the grid
        const char *srcWKT = poDataset->GetProjectionRef();
        if (!strlen(srcWKT)) {
            throw STTException("The source dataset does not have a spatial reference system assigned");
        }

        OGRSpatialReference srcSRS = OGRSpatialReference(srcWKT);
        OGRSpatialReference gridSRS = mGrid.getSRS();

        #if ( GDAL_VERSION_MAJOR >= 3)
        srcSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        gridSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        #endif

        if (!srcSRS.isSame(&gridSRS)) { // it does not match
            // check if the srs is valid
            switch(srcSRS.Validate()) {
            case OGRERR_NONE:
                break;
            case OGRERR_CORRUPT_DATA:
                throw STTException("The source spatial reference system appears to be corrupted");
                break;
            case OGRERR_UNSUPPORTED_SRS:
                throw STTException("The source spatial reference system is not supported");
                break;
            default:
                throw STTException("There is an undhandled return value from `srcSRS.Validate()`");
            }

            // we need to transform the bounds to the grid SRS
            double x[4] = {bounds.getMinX(), bounds.getMaxX(), bounds.getMaxX(), bounds.getMinX()};
            double y[4] = {bounds.getMinY(), bounds.getMaxY(), bounds.getMaxY(), bounds.getMinY()};

            OGRCoordinateTransformation *transformer = OGRCreateCoordinateTransformation(&srcSRS, &gridSRS);
            if (transformer == NULL) {
                throw STTException("The source dataset to tile grid coordinate transformation could not be created");
            } else if (transformer->Transform(4, x, y) != true) {
                delete transformer;
            }
            delete transformer;

            // get the min and max values of the transformed coordinates
            double minX = std::min(std::min(x[0], x[1]), std::min(x[2], x[3]))
            double maxX = std::max(std::max(x[0], x[1]), std::max(x[2], x[3]))
            double minY = std::min(std::min(x[0], x[1]), std::min(x[2], x[3]))
            double maxY = std::max(std::max(x[0], x[1]), std::max(x[2], x[3]))

            mBounds = CRSBounds(minX, minY, maxX, maxY);                     // set the bounds
            mResolution = mBounds.getWidth() / poDataset->GetRasterXSize();  // set the resolution

            // cache the SRS string for use in reprojections later
            char *srsWKT = NULL;
            if (gridSRS.exportToWkt(&srsWKT) != OGRERR_NONE) {
                CPLFree(srsWKT);
                throw STTException("Could not create grid WKT string");
            }
            crsWKT = srsWKT;
            CPLFree(srsWKT);
            srsWKT = NULL;
        } else {                                         // srcSRS == gridSRS so no projection is necessary
            mBounds = bounds;                            // use the existing dataset bounds
            mResolution = std::abs(adfGeoTransform[1]);  // use the existing dataset resolution
        }

        poDataset->Reference();  // increase the refcount of the dataset
    }
}

GDALTiler::GDALTiler(const GDALTiler &other):
    mGrid(other.mGrid),
    poDataset(other.poDataset),
    mBounds(other.mBounds),
    mResolution(other.mResolution),
    crsWKT(other.crsWKT)
{
    if (poDataset != NULL) {
        poDataset->Reference();    // increase the refcount of the dataset
    }
}

GDALTiler::GDALTiler(GDALTiler &other):
    mGrid(other.mGrid),
    poDataset(other.poDataset),
    mBounds(other.mBounds),
    mResolution(other.mResolution),
    crsWKT(other.crsWKT)
{
    if(poDataset != NULL) {
        poDataset->Reference();    // increate the refcount of the dataset
    }
}

GDALTiler &
GDALTiler::operator=(const GDALTiler &other)
{
    closeDataset();

    mGrid = other.mGrid;
    poDataset = other.poDataset;

    if (poDataset != NULL) {
        poDataset->Reference();    // increate the refcount of the dataset
    }

    mBounds = other.mBounds;
    mResolution = other.mResolution;
    crsWKT = other.crsWKT;

    return *this;
}

GDALTiler::~GDALTiler()
{
    closeDataset();
}

GDALTile *
GDALTiler::createRasterTile(GDALDataset *dataset, const TileCoordinate &coord) const
{
    // convert the tile bounds into a geo transform
    double adfGeoTransform[6] = mGrid.resolution(coord, zoom);
    double resolution = mGrid.resolution(coord, zoom);
    CRSBounds tileBounds = mGrid.tileBounds(coord);

    adfGeoTransform[0] = tileBounds.getMinX();    // min longitude
    adfGeoTransform[1] = resolution;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = tileBounds.getMaxY();    // max latitude
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -resolution;

    GDALTile *tile = createRasterTile(dataset, adfGeoTransform);
    static_cast<TileCoordinate &>(*tile) = coord;

    // set the shifted geo transform to the VRT
    if (GDALSetGeoTransform(tile->dataset, adfGeoTransform) != CE_None) {
        throw STTException("Could not set geo transform on VRT");
    }

    return tile;
}

/**
* @brief get an overview dataset which best matches a transformation
*
* try and get an overview from the soruce dataset that corresponds more closely
* to the resolution belonging to any output of the transformation. this will
* make downsampling operations much quicker and work around integer overflow
* errors that can occur if downsampling very high resolution source datasets to
* small scale (low zoom level) tiles.
*
* this code is adapted from that found in `gdalwarp.cpp` implementing the
* `gdalwarp -over` option.
*/
#if (GDAL_VERSION_MAJOR >= 3)
#include "gdaloverviewdataset.cpp"
#elif (GDAL_VERSION_MAJOR >= 2 && GDAL_VERSION_MINOR >= 2)
#include "gdaloverviewdataset-gdal2x.cpp"
#endif

static
GDALDatasetH
getOverviewDataset(GDALDatasetH hSrcDS, GDALTransformerFunc pfnTransformer, void *hTransformerArg)
{
    GDALDataset *poSrcDS = static_cast<GDALDataset *>(hSrcDS);
    GDALDataset *poSrcOverDS = NULL;
    int nOvLevel = -2;
    int nOvCount = poSrcDS->GetRasterBand(1)->GetOverviewCount();

    if (nOvCount > 0) {
        double adfSuggestedGeoTransform[6];
        double adfExtent[4];
        int nPixels;
        int nLines;

        /* compute what the "natural" output resolution (in pixels) would be for this */
        /* input dataset */
        if (GDALSuggestedWarpOutput2(hSrcDS, pfnTransformer,
                hTransformerArg, adfSuggestedGeoTransform,
                &nPixels, &nLines, adfExtent, 0) == CE_None) {

            double dfTargetRation = 1.0 / adfSuggestedGeoTransform[1];

            if (dfTargetRatio > 1.0) {
                int iOver;
                for (iOvr = -1; iOvr < nOvCount - 1; iOvr++) {
                    double dfOvrRatio = (iOvr < 0) ? 1.0 : (double)poSrcDS->GetRasterXSize() /
                        poSrcDS->GetRasterBand(1)->GetOverview(iOvr)->GetXSize();

                    double dfNextOvrRatio = (double)poSrcDS->GetRasterXSize() /
                        poSrcDS->GetRasterBand(1)->GetOverview(iOvr + 1)->GetXSize();

                    if (dfOvrRatio < dfTargetRatio && dfNextOvrRatio > dfTargetRatio)
                        break;

                    if (fabs(dfOvrRatio - dfTargetRatio) < 1e-1)
                        break;
                }

                iOvr += (nOvLevel + 2);
                if (iOvr >= 0) {
                    #if (GDAL_VERSION_MAJOR >= 3 || (GDAL_VERSION_MAJOR >=2 && GDAL_VERSION_MINOR >=2))
                        poSrcOvrDS = GDALOverviewDataset(poSrcDS, iOvr, FALSE);
                    #else
                        poSrcOvrDS = GDALOverviewDataset(poSrcDS, iOvr, FALSE, FALSE);
                    #endf
                }
            }
        }
    }

    return static_cast<GDALDatasetH>(poSrcOverDS);
}

/**
* @details this method is the heart of the tiler. a `TileCoordinate` is used
* to obtain the geospatial extent associated with that tile as related to the
* underlying GDAL dataset. this mapping may require a reproduction if the
* underlying dataset is not in the tile projection system. this information
* is the encapsulated as a GDAL virtual raster (VRT) dataset and returned to
* the caller.
*
* it is the caller's responsibility to call `GDALClose()` on the returned
* dataset.
*/
GDALTile *
GDALTiler::createRasterTile(GDALDataset *dataset, double(&adfGeoTransform)[6]) const
{
    if (dataset == NULL) {
        throw STTException("No GDAL dataset is set");
    }

    // the source and sink datasets
    GDALDatasetH hSrcDS = (GDALDatasetH) dataset;
    GDALDatasetH hDstDS;

    // the transformationi option list
    CPLStringList transformOptions;

    // the source, sink and grid srs
    const char *pszSrcWKT = GDALGetProjectionRef(hSrcDS);
    const char *pszGridWKT = pszSrcWKT;

    if (!strlen(pszSrcWKT))
        throw STTException("The source dataset no longer has a spatial reference system assigned");

    // populate the SRS WKT strings if we need to reproject
    if (requiresReprojection()) {
        pszGridWKT = crsWKT.c_str();
        transformOptions.SetNameValue("SRC_SRS", pszSrcWKT);
        transformOptions.SetNameValue("DST_SRS", pszGridWKT);
    }

    // set the warp options
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->eResampleAlg = options.resampleAlg;
    psWarpOptions->dfWarpMemoryLimit = options.warpMemoryLimit;
    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->nBandCount = poDataset->GetRasterCount();
    psWarpIptions->panSrcBands = (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpIptions->panDstBands = (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);

    psWarpOptions->padfSrcNoDataReal = (double *) CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
    psWarpOptions->padfSrcNoDataImag = (double *) CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
    psWarpOptions->padfDstNoDataReal = (double *) CPLCalloc(psWarpOptions->nBandCount, sizeof(double));
    psWarpOptions->padfDstNoDataImag = (double *) CPLCalloc(psWarpOptions->nBandCount, sizeof(double));

    for (short unsigned int i = 0; i < psWarpOptions->nBandCount; ++i) {
        int bGotNoData = FALSE;
        double noDataValue = poDataset->GetRasterBand(i + 1)->GetNoDataValue(&bGotNoData);

        if (!bGotNoData) noDataValue = -32768;

        psWarpOptions->padfSrcNoDataReal[i] = noDataValue;
        psWarpOptions->padfSrcNoDataImag[i] = 0;
        psWarpOptions->padfDstNoDataReal[i] = noDataValue;
        psWarpOptions->padfDstNoDataImag[i] = 0;

        psWarpOptions->panDstBands[i] = psWarpOptions->panSrcBands[i] = i + 1;
    }

    // create the image to image transformer
    void *transformerArg = GDALCreateGenImgProjTransformer2(hSrcDS, NULL, transformOptions.List());
    if (transformerArg == NULL) {
        GDALDestroyWarpOptions(psWarpOptions);
        throw STTException("Could not create image to image transformer");
    }

    // specify the destination geotransform
    GDALSetGenImgProjTransformerDstGeoTransform(transformerArg, adfGeoTransform);

    // try and get an overview from the source dataset that corresponds more
    // closely to the resolution of this tile.
    GDALDatasetH hWrkSrcDS = getOverviewDataset(hSrcDS, GDALGenImgProjTransform, transformerArg);
    if (hWrkSrcDS == NULL) {
        hWrkSrcDS = psWarpOptions->hSrcDS = hSrcDS;
    } else {
        psWarpOptions->hSrcDS = hWrkSrcDS;

        // we need to recreate the transform when operating on an overview.
        GDALDestroyGenImgProjTransformer(transformerArg);

        transformerArg = GDALCreateGenImgProjTransformer2(hWrkSrcDS, NULL, transformerOptions.List());
        if (transformerArg == NULL) {
            GDALDestroyWarpOptions(psWarpOptions);
            throw STTException("Could not create overview image to image transformer");
        }

        // specify the destination geotransform
        GDALSetGenImgProjTransformerDstGeoTransform(transformerArg, adfGeoTransform);
    }

    // decide if we are doing an approximate or exact transformation
    if (options.errorThreshold) {
        // approximate: wrap the transformer with a linear approximator
        psWarpOptions->pTransformerArg = GDALCreateApproxTransformer(
            GDALGenImgProjTransformer, transformerArg, options.errorThreshold
        );

        if (psWarpOptions->pTransformerArg == NULL) {
            GDALDestroyWarpOptions(psWarpOptions);
            GDALDestroyGenImgProjTransformer(transformerArg);
            throw STTException("Could not create linear approximator");
        }

        psWarpOptions->pfnTransformer = GDALApproxTransform;

    } else {
        // exact      : no wrapping required
        psWarpOptions->pTransformerArg = transformerArg;
        psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
    }

    // the raster tile is represented as a VRT dataset
    hDstDS = GDALCreateWarpedVRT(hWrkSrcDS, mGrid.tileSize(), mGrid.tileSize(), adfGeoTransform, psWarpOptions);

    bool isApproxTransform = (psWarpOptions->pfnTransformer == GDALApproxTransform);
    GDALDestroyWarpOptions(psWarpOptions);

    if (hDstDS == NULL) {
        GDALDestroyGenImgProjTransformer(transformerArg);
        throw STTException("Could not create warped VRT");
    }

    // set the projection information on the dataset.
    // this will always be the grid SRS.
    if (GDALSetProjection(hDstDS, pszGridWKT) != CE_None) {
        GDALClose(hDstDS);

        if (transformerArg != NULL) {
            GDALDestroyGenImgProjTransformer(transformerArg);
        }
        throw STTException("Could not set projection on VRT");

    }

    // create the tile, passing it the base image transformer to manage if
    // this is an approximate transform

    return new GDALTile((GDALDataset *) hDstDS, isApproxTransform ? transformerArg : NULL);
}

/**
* @details this dereferences the underlying GDAL dataset and closes it
* if the reference count falls below 1.
*/
void
GDALTiler::closeDataset()
{
    // dereference and possibly close the GDAL dataset
    if (poDataset != NULL) {
        poDataset->Dereference();

        if (poDataset->GetRefCount() < 1) {
            GDALClose(poDataset);
        }

        poDataset = NULL;
    }
}
