#pragma once

#include <QDebug>
#include <QException>
#include <QSharedPointer>
#include <ogrsf_frmts.h>

namespace gzpi {
    namespace internal {
        class GDALDriverWrapper {
        public:
            GDALDriverWrapper() {
                GDALAllRegister();
            }
        };
    }
    class GDALWrapperError: public QException{
    public:
        GDALWrapperError() = default;
        GDALWrapperError(const QString &m): msg(m) {}
        virtual const char* what() const override {
            return msg.isEmpty() ? "GDAL Unknown error" : msg.toStdString().data();
        }
    private:
        QString msg;
    };

   class OGRFeatureWrapper {
   public:
       OGRFeatureWrapper(const OGRFeatureWrapper &) = delete;
       OGRFeatureWrapper(OGRFeatureWrapper &&) = default;
       OGRFeatureWrapper& operator=(const OGRFeatureWrapper& other) = delete;
       OGRFeatureWrapper& operator=(OGRFeatureWrapper&& other) = default;

       QSharedPointer<OGRGeometry> GetGeometryRef() {
           OGRGeometry *geom = feature->GetGeometryRef();
           return QSharedPointer<OGRGeometry>(geom);
       }
       double GetFieldAsDouble(GIntBig fid){
           return feature->GetFieldAsDouble(fid);
       }

       bool isValid() {
           return feature != nullptr;
       }
       GIntBig GetFID(){
           return feature->GetFID();
       }

   private:
       explicit OGRFeatureWrapper(OGRFeature *feature):feature(feature) {}
       QSharedPointer<OGRFeature> feature;
   };

   class OGRFeatureDefnWrapper{
   public:
       OGRFeatureDefnWrapper(const OGRFeatureDefnWrapper &) = delete;
       OGRFeatureDefnWrapper(OGRFeatureDefnWrapper &&) = default;
       OGRFeatureDefnWrapper& operator=(const OGRFeatureDefnWrapper& other) = delete;
       OGRFeatureDefnWrapper& operator=(OGRFeatureDefnWrapper&& other) = default;

       int GetFieldIndex(const char* name){
           int index = def->GetFieldIndex(name);
           if(index == -1)
               throw GDALWrapperError(QString("No Field ") + name + "in Feature Define");
           return index;
       }

   private:
       explicit OGRFeatureDefnWrapper(OGRFeatureDefn* d): def(d){}
       QSharedPointer<OGRFeatureDefn> def;
   };

   class OGRLayerWrapper {
   public:
       friend class OGRFeatureWrapper;
       friend class OGRFeatureDefnWrapper;

       OGRLayerWrapper(const OGRLayerWrapper &) = delete;
       OGRLayerWrapper(OGRLayerWrapper &&) = default;
       OGRLayerWrapper& operator=(const OGRLayerWrapper& other) = delete;
       OGRLayerWrapper& operator=(OGRLayerWrapper&& other) = default;

       QSharedPointer<OGRFeatureWrapper> GetNextFeature(){
           OGRFeature* feature = layer->GetNextFeature();
           return QSharedPointer<OGRFeatureWrapper>::create(feature);
       }

       OGRFeatureWrapper GetFeature(GIntBig fid){
           OGRFeature* feature = layer->GetFeature(fid);
           if(feature == nullptr){
               throw GDALWrapperError(QString("No feature ") + fid + "in Layer");
           }
           return OGRFeatureWrapper(feature);
       }

       bool isValid() const {
           return true;
       }

       void ResetReading(){
           layer->ResetReading();
       }

       OGREnvelope GetExtent(){
           OGREnvelope e;
           layer->GetExtent(&e);
           return e;
       }

       OGRFeatureDefnWrapper GetLayerDefn(){
           OGRFeatureDefn* def = layer->GetLayerDefn();
           return OGRFeatureDefnWrapper(def);
       }

   private:
       explicit OGRLayerWrapper(OGRLayer *layer): layer(layer){
           Q_ASSERT(layer != nullptr);
       }
       QSharedPointer<OGRLayer> layer;
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
           return GDALDatasetWrapper(dataset);
       }

       OGRLayerWrapper GetLayer(int iLayer){
            OGRLayer* layer = ds->GetLayer(iLayer);
            if(layer == nullptr){
                throw GDALWrapperError();
            }
            return OGRLayerWrapper(layer);
       }

       OGRLayerWrapper GetLayerByName(const char *nLayer){
            OGRLayer* layer = ds->GetLayerByName(nLayer);
            if(layer == nullptr){
                throw GDALWrapperError();
            }
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
           Q_ASSERT(dataset != nullptr);
           ds = QSharedPointer<GDALDataset>(nullptr, GDALDatasetDeleteWrapper());
       }
       QSharedPointer<GDALDataset> ds;
   };
}
