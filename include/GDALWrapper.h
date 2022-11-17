#pragma once

#include <OGRException.h>

#include <ogrsf_frmts.h>

#include <QDebug>
#include <QCoreApplication>
#include <QSharedPointer>

namespace scially {
    class GDALDriverWrapper {
    public:
        GDALDriverWrapper() {
            CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
            std::string basePath = qApp->applicationDirPath().toStdString();
            gdalData = basePath + "/gdal_data";
            projData = basePath + "/proj_data";
            init();
        }
    private:
        void init() const {
            CPLSetConfigOption("GDAL_DATA", gdalData.c_str());
            const char* const projLibPath[] = { projData.c_str(), nullptr };
            OSRSetPROJSearchPaths(projLibPath);
            GDALAllRegister();
        }
        std::string gdalData;
        std::string projData;
    };


    class OGRFeatureWrapper;
    class OGRLayerWrapper;
    class GDALDatasetWrapper;


    class OGRFeatureWrapper {
    public:
        friend class OGRLayerWrapper;

        OGRGeometry* GetGeometryRef() {
            return feature->GetGeometryRef();
        }

        double GetFieldAsDouble(GIntBig fid) {
            return feature->GetFieldAsDouble(fid);
        }

        bool isValid() const {
            return feature != nullptr;
        }

        GIntBig GetFID() {
            return feature->GetFID();
        }

    private:
        explicit OGRFeatureWrapper(OGRFeature* f) {
            feature = QSharedPointer<OGRFeature>(f, &OGRFeature::DestroyFeature);
        }

        QSharedPointer<OGRFeature> feature;
    };


    class OGRLayerWrapper {
    public:
        friend class GDALDatasetWrapper;

        OGRFeatureWrapper GetNextFeature() {
            OGRFeature* feature = layer->GetNextFeature();
            return OGRFeatureWrapper(feature);
        }

        OGREnvelope GetExtent(int bForce = 1) {
            OGREnvelope envelope;
            OGRErr err = layer->GetExtent(&envelope, bForce);
            if (err != OGRERR_NONE)
                throw OGRException(err);

            return envelope;
        }

        OGRFeatureWrapper GetFeature(GIntBig fid) {
            OGRFeature* feature = layer->GetFeature(fid);
            if (feature == nullptr) {
                throw OGRException(QString("No feature ") + fid + "in Layer");
            }
            return OGRFeatureWrapper(feature);
        }

        bool isValid() const {
            return true;
        }

        void ResetReading() {
            layer->ResetReading();
        }

        OGRSpatialReference* GetSpatialRef() {
            return layer->GetSpatialRef();
        }

        OGRFeatureDefn* GetLayerDefn() {
            return layer->GetLayerDefn();
        }

    private:
        explicit OGRLayerWrapper(OGRLayer* layer) : layer(layer) {}
        OGRLayer* layer;  // will auto dispose when ds close.
    };

    class GDALDatasetWrapper {
    public:
        friend class OGRLayerWrapper;
        static inline GDALDatasetWrapper open(const char* pszFilename,
            unsigned int nOpenFlags,
            const char* const* papszAllowedDrivers = nullptr,
            const char* const* papszOpenOptions = nullptr,
            const char* const* papszSiblingFiles = nullptr) {
            GDALDataset* dataset = (GDALDataset*)GDALOpenEx(pszFilename,
                nOpenFlags,
                papszAllowedDrivers,
                papszOpenOptions,
                papszSiblingFiles);
            if (dataset == nullptr)
                throw OGRException(QString("Can't open dataset %1").arg(pszFilename));

            return GDALDatasetWrapper(dataset);
        }

        OGRLayerWrapper GetLayer(int iLayer) {
            OGRLayer* layer = ds->GetLayer(iLayer);
            return OGRLayerWrapper(layer);
        }

        OGRLayerWrapper GetLayerByName(const char* nLayer) {
            OGRLayer* layer = ds->GetLayerByName(nLayer);
            return OGRLayerWrapper(layer);
        }
        bool isValid() const {
            return true;
        }

    private:
        struct GDALDatasetDeleteWrapper {
            void operator() (GDALDataset* dataset) {
                if (dataset != nullptr)
                    GDALClose(dataset);
            }
        };
        // dataset 将被 GDALDatasetWrapper 接管
        explicit GDALDatasetWrapper(GDALDataset* dataset) {
            ds = QSharedPointer<GDALDataset>(dataset, GDALDatasetDeleteWrapper());
        }
        QSharedPointer<GDALDataset> ds;
    };
}
