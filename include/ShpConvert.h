#pragma once

#include <QByteArray>
#include <QString>
#include <QStringView>
#include <GDALWrapper.h>
#include <QuadTree.h>
#include <TilesConvertException.h>
namespace gzpi {
    class ShpConvert {
    public:
        ShpConvert(const QString &fileName, const QString &layerName, const QString &heightField)
            :fileName(fileName), layerName(layerName), heightField(heightField)
        {}

        QByteArray toB3DM(){
            QByteArray b3dmBuffer;
            return b3dmBuffer;
        }

        void convertTiles(const QString &output){
            GDALDatasetWrapper ds = GDALDatasetWrapper::open(fileName.toStdString().data(),1);
            OGRLayerWrapper layer = ds.GetLayerByName(layerName.toStdString().data());
            layer.ResetReading();
            tree.setEnvelope(layer.GetExtent());

            QSharedPointer<OGRFeatureWrapper> feature;
            while(feature = layer.GetNextFeature(), feature != nullptr){
                QSharedPointer<OGRGeometry> geometry = feature->GetGeometryRef();
                if(geometry == nullptr)
                    continue;
                OGREnvelope envelop;
                geometry->getEnvelope(&envelop);
                tree.add(feature->GetFID(), envelop);
            }

            int heightIndex = layer.GetLayerDefn().GetFieldIndex(heightField.toStdString().data());
            OGREnvelope allBox;
            tree.traverse([&layer, &allBox](QuadTree* root)  {
                // Calc All Geometry Envelope
                for(int i = 0; i < root->geomsSize(); i++){
                    int fid = root->getGeomFID(i);
                    OGRFeatureWrapper feature = layer.GetFeature(fid);
                    QSharedPointer<OGRGeometry> geometry = feature.GetGeometryRef();
                    OGREnvelope envelope;
                    geometry->getEnvelope(&envelope);
                    if(allBox.IsInit()){
                        allBox.Merge(envelope);
                    }else{
                        allBox = envelope;
                    }
                }

                // Build 3D Model per geometry
                for(int i = 0; i < root->geomsSize(); i++){
                    int fid = root->getGeomFID(i);
                    OGRFeatureWrapper feature = layer.GetFeature(fid);
                    QSharedPointer<OGRGeometry> geometry = feature.GetGeometryRef();
                    double height = feature.GetFieldAsDouble(fid);

                    if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
                        QSharedPointer<OGRPolygon> polygon = geometry.dynamicCast<OGRPolygon>();
                        Polygon_Mesh mesh = convert_polygon(polyon, center_x, center_y, height);
                        mesh.mesh_name = "mesh_" + std::to_string(id);
                        mesh.height = height;
                        v_meshes.push_back(mesh);
                    }
                    else if (wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon) {
                        QSharedPointer<OGRMultiPolygon> multipolygon = geometry.dynamicCast<OGRMultiPolygon>();
                        for (int j = 0; j < multipolygon->getNumGeometries(); j++) {
                            QSharedPointer<OGRPolygon> polygon = (OGRPolygon*)multipolygon->getGeometryRef(j);
                            mesh.mesh_name = "mesh_" + std::to_string(id);
                            mesh.height = height;
                            v_meshes.push_back(mesh);
                        }
                    }else{
                        throw TilesConvertException("Only support Polygon");
                    }
                }
            });
        }

    private:
        QuadTree tree;
        QString fileName;
        QString layerName;
        QString heightField;
    };
}
