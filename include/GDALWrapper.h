#pragma once

#include <spdlog/spdlog.h>
#include <ogrsf_frmts.h>
#include <fmt/core.h>

#include <QCoreApplication>
#include <QSharedPointer>
#include <QDir>

#include <string>
#include <exception>
#include <stdexcept>

template <> 
struct fmt::formatter<QString>: fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const QString& c, FormatContext& ctx) const {
        std::string platformString = c.toLocal8Bit();
        return fmt::formatter<std::string>::format(platformString, ctx);
    }
};


namespace scially {
    class GDALIniter {
    public:
        GDALIniter() {
            CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
            QString basePath = qApp->applicationDirPath();
            QString gdalData = basePath + "/GDAL_DATA";
            QString projData = basePath + "/PROJ_LIB";
            gdalData_ = std::string(gdalData.toUtf8());
            projData_ = std::string(projData.toUtf8());
            init();
        }
    private:
        std::string gdalData_;
        std::string projData_;

        void init() {
            CPLSetConfigOption("GDAL_DATA", gdalData_.c_str());
            const char *const proj_lib_path[] = {projData_.c_str(), nullptr};
            OSRSetPROJSearchPaths(proj_lib_path);
            GDALAllRegister();
        }
    };

    class OGRException : public std::exception {
	public:
		OGRException(const QString& err) : error(err) {}
		OGRException(OGRErr err) {
			switch (err) {
			case OGRERR_NOT_ENOUGH_DATA:
				error =  "Not enough data";
				break;
			case OGRERR_NOT_ENOUGH_MEMORY:
				error =  "Not enough memory";
				break;
			case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
				error = "Unsupported geometry type";
				break;
			case OGRERR_UNSUPPORTED_OPERATION:
				error = "Unsupported operation";
				break;
			case OGRERR_CORRUPT_DATA:
				error = "Corrupt data";
				break;
			case OGRERR_FAILURE:
				error = "Failure";
				break;
			case OGRERR_UNSUPPORTED_SRS:
				error = "Unsupported srs";
				break;
			case OGRERR_INVALID_HANDLE:
				error = "Invalid handle";
				break;
			case OGRERR_NON_EXISTING_FEATURE:
				error = "Non existing feataure";
				break;
			default:
				error = "Unkonwn error";
			}
		}
		
        virtual const char* what() const noexcept override {
            return "";
		}
	private:
		QString error;
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

       double GetFieldAsDouble(GIntBig fid){
           return feature->GetFieldAsDouble(fid);
       }

       bool isValid() const{
           return feature != nullptr;
       }

       GIntBig GetFID(){
           return feature->GetFID();
       }

   private:
       explicit OGRFeatureWrapper(OGRFeature *f){
           feature = QSharedPointer<OGRFeature>(f, &OGRFeature::DestroyFeature);
       }

       QSharedPointer<OGRFeature> feature;
   };


   class OGRLayerWrapper {
   public:
       friend class GDALDatasetWrapper;

       OGRFeatureWrapper GetNextFeature(){
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

       OGRFeatureWrapper GetFeature(GIntBig fid){
           OGRFeature* feature = layer->GetFeature(fid);
           if(feature == nullptr){
               throw OGRException(QString("No feature ") + fid + "in Layer");
           }
           return OGRFeatureWrapper(feature);
       }

       bool isValid() const {
           return true;
       }

       void ResetReading(){
           layer->ResetReading();
       }

       OGRSpatialReference* GetSpatialRef() {
           return layer->GetSpatialRef();
       }

       OGRFeatureDefn* GetLayerDefn(){
           return layer->GetLayerDefn();
       }

   private:
       explicit OGRLayerWrapper(OGRLayer *layer): layer(layer){}
       OGRLayer* layer;  // will auto dispose when ds close.
   };

   class GDALDatasetWrapper {
   public:
        friend class OGRLayerWrapper;
        static inline GDALDatasetWrapper open(const char* pszFilename,
                                             unsigned int nOpenFlags,
                                             const char* const* papszAllowedDrivers = nullptr,
                                             const char* const* papszOpenOptions = nullptr,
                                             const char* const* papszSiblingFiles = nullptr){
           GDALDataset *dataset = (GDALDataset*)GDALOpenEx(pszFilename,
                                                           nOpenFlags,
                                                           papszAllowedDrivers,
                                                           papszOpenOptions,
                                                           papszSiblingFiles);
           if (dataset == nullptr)
               throw OGRException(QString("Can't open dataset %1").arg(pszFilename));

           return GDALDatasetWrapper(dataset);
       }

       OGRLayerWrapper GetLayer(int iLayer){
            OGRLayer* layer = ds->GetLayer(iLayer);
            return OGRLayerWrapper(layer);
       }

       OGRLayerWrapper GetLayerByName(const char *nLayer){
            OGRLayer* layer = ds->GetLayerByName(nLayer);
            return OGRLayerWrapper(layer);
       }
       bool isValid() const{
           return true;
       }

   private:
       struct GDALDatasetDeleteWrapper{
           void operator() (GDALDataset *dataset){
               if(dataset != nullptr)
                   GDALClose(dataset);
           }
       };
       // dataset 将被 GDALDatasetWrapper 接管
       explicit GDALDatasetWrapper(GDALDataset *dataset){
           ds = QSharedPointer<GDALDataset>(dataset, GDALDatasetDeleteWrapper());
       }
       QSharedPointer<GDALDataset> ds;
   };
}
