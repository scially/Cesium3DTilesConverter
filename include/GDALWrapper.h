#pragma once

#include <ogrsf_frmts.h>

#include <QCoreApplication>
#include <QtDebug>
#include <QException>
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

    class OGRException : QException {
    public:
        OGRException(const QString& err) : err_(err) {}
        OGRException(OGRErr err) {
            switch (err) {
            case OGRERR_NOT_ENOUGH_DATA:
                err_ = "[gdal/ogr] not enough data";
                break;
            case OGRERR_NOT_ENOUGH_MEMORY:
                err_ = "[gdal/ogr] not enough memory";
                break;
            case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
                err_ = "[gdal/ogr] unsupported geometry type";
                break;
            case OGRERR_UNSUPPORTED_OPERATION:
                err_ = "[gdal/ogr] unsupported operation";
                break;
            case OGRERR_CORRUPT_DATA:
                err_ = "[gdal/ogr] corrupt data";
                break;
            case OGRERR_FAILURE:
                err_ = "[gdal/ogr] failure";
                break;
            case OGRERR_UNSUPPORTED_SRS:
                err_ = "[gdal/ogr] unsupported srs";
                break;
            case OGRERR_INVALID_HANDLE:
                err_ = "[gdal/ogr] invalid handle";
                break;
            case OGRERR_NON_EXISTING_FEATURE:
                err_ = "[gdal/ogr] non existing feataure";
                break;
            default:
                err_ = "[gdal/ogr] unkonwn error";
            }
        }
        virtual void raise() const override {
            throw* this;
        }
        virtual OGRException* clone() const override {
            return new OGRException(*this);
        }

        QString error() const noexcept {
            return err_;
        }

    private:
        QString err_;
    };

    class OGRFeatureWrapper;
    class OGRLayerWrapper;
    class GDALDatasetWrapper;

    class OGRFeatureWrapper {
    public:
        friend class OGRLayerWrapper;

        OGRFeatureWrapper() = default;
        OGRGeometry* GetGeometryRef() {
            return feature_->GetGeometryRef();
        }

        double GetFieldAsDouble(GIntBig fid) {
            return feature_->GetFieldAsDouble(fid);
        }

        bool isValid() const {
            return feature_ != nullptr;
        }

        GIntBig GetFID() {
            return feature_->GetFID();
        }

    private:
        explicit OGRFeatureWrapper(OGRFeature* f) {
            feature_.reset(f);
        }

        QSharedPointer<OGRFeature> feature_;
    };

    class OGRLayerWrapper {
    public:
        friend class GDALDatasetWrapper;
		
        OGRLayerWrapper() = default;
        OGRFeatureWrapper GetNextFeature() {
            OGRFeature* feature = layer_->GetNextFeature();
            return OGRFeatureWrapper(feature);
        }

        OGREnvelope GetExtent(int bForce = 1) {
            OGREnvelope envelope;
            OGRErr err = layer_->GetExtent(&envelope, bForce);
            if (err != OGRERR_NONE)
                throw OGRException(err);

            return envelope;
        }

        OGRFeatureWrapper GetFeature(GIntBig fid) {
            OGRFeature* feature = layer_->GetFeature(fid);
            return OGRFeatureWrapper(feature);
        }

        bool isValid() const {
            return true;
        }

        void ResetReading() {
            layer_->ResetReading();
        }

        OGRSpatialReference* GetSpatialRef() {
            return layer_->GetSpatialRef();
        }

        OGRFeatureDefn* GetLayerDefn() {
            return layer_->GetLayerDefn();
        }

    private:
        explicit OGRLayerWrapper(OGRLayer* layer) : layer_(layer) {}
        OGRLayer* layer_;  // will auto dispose when ds close.
    };

    class GDALDatasetWrapper {
    public:
        friend class OGRLayerWrapper;
        GDALDatasetWrapper() = default;
		
        bool open(const QString &fileName, unsigned int nOpenFlags) {
            GDALDataset* dataset = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(),
                nOpenFlags,
                nullptr,
                nullptr,
                nullptr);
            ds_.reset(dataset);
            return ds_ != nullptr;
        }

        OGRLayerWrapper GetLayer(int iLayer) {
            OGRLayer* layer = ds_->GetLayer(iLayer);
            return OGRLayerWrapper(layer);
        }

        OGRLayerWrapper GetLayerByName(const QString& nLayer) {
            OGRLayer* layer = ds_->GetLayerByName(nLayer.toStdString().c_str());
            
            return OGRLayerWrapper(layer);
        }
        bool isValid() const {
            return true;
        }

    private:
        struct GDALDatasetDeleteWrapper {
            void operator()(GDALDataset* dataset) {
                if (dataset != nullptr)
                    GDALClose(dataset);
            }
        };

        GDALDatasetWrapper(GDALDataset* dataset) {
            Q_ASSERT(dataset != nullptr);
            ds_.reset(dataset, GDALDatasetDeleteWrapper());
        }
		
        QSharedPointer<GDALDataset> ds_;
    };
}
