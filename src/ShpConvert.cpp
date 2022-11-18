#include <Cesium3DTiles/BaseTile.h>
#include <ShpConvert.h>

#include <QtDebug>
#include <QDir>
#include <QFile>

namespace scially {
    bool ShpConvert::getAllFeatures() {
        OGRSpatialReference wgsSRS;
        wgsSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        wgsSRS.importFromEPSG(4326);

        if (!ds_.open(fileName_, 1)) {
            qCritical() << "can't open" << fileName_;
            return false;
        }
        
        layer_ = ds_.GetLayerByName(layerName_);
        if (!layer_.isValid()) {
            qCritical() << "can't load layer" << layerName_ << "in" << fileName_;
            return false;
        }
        layer_.ResetReading();
        OGRFeatureWrapper feature = layer_.GetNextFeature();
        while (feature.isValid()) {
            OGRGeometry* geometry = feature.GetGeometryRef();
            OGRErr err = geometry->transformTo(&wgsSRS);
            if (err != OGRERR_NONE) {
                qCritical() << "can't transform geometry to WGS84";
                return false;
            }

            OGREnvelope extent;
            geometry->getEnvelope(&extent);
            if (!extent_.IsInit()) {
                extent_ = extent;
            }
            else {
                extent_.Merge(extent);
            }
            iFeature_[feature.GetFID()] = feature;
            feature = layer_.GetNextFeature();
        }
        return true;
    }

    bool ShpConvert::initQuadTree(QuadTree& tree) {
        if (!getAllFeatures()) {
            return false;
        }

        tree.setEnvelope(extent_);

        QMap<GIntBig, OGRFeatureWrapper>::iterator iter = iFeature_.begin();
        while (iter != iFeature_.end()) {
            OGREnvelope extent;
            auto geometry = iter.value().GetGeometryRef();
            geometry->getEnvelope(&extent);
            tree.add(iter.key(), extent);
            ++iter;
        }
        return true;
    }

    bool ShpConvert::convertTiles(const QString& output) {
        QuadTree tree;
        if (!initQuadTree(tree)) {
            return false;
        }
            
        int heightIndex = layer_.GetLayerDefn()->GetFieldIndex(heightField_.toStdString().data());
        if (heightIndex == -1) {
            qCritical() << heightField_ << "not found in layer" << layerName_;
            return false;
        }

        BaseTile tile;
        double layerMaxHeight = 0;
        tree.traverse([this, &output, &tile, &layerMaxHeight, heightIndex](QuadTree* root) {
            // Build 3D Model per geometry
            double centerX =   (extent_.MinX + extent_.MaxX) / 2;
            double centerY =   (extent_.MinY + extent_.MaxY) / 2;
            double boxWidth =  (extent_.MaxX - extent_.MinX);
            double boxHeight = (extent_.MaxY - extent_.MinY);
            double maxHeight = 0;
            QDir outputLocation = QString("%1/tile/%2/%3").
                arg(output).
                arg(root->level()).
                arg(root->row());
            if (!outputLocation.exists())
                outputLocation.mkpath(".");

            GeometryMesh meshes;
            for (int i = 0; i < root->geomSize(); i++) {
                int fid = root->geomFID(i);
                OGRFeatureWrapper feature = iFeature_[fid];
                OGRGeometry* geometry = feature.GetGeometryRef();
                double height = feature.GetFieldAsDouble(heightIndex);
                maxHeight = std::max(height, maxHeight);
                layerMaxHeight = std::max(layerMaxHeight, maxHeight);

                if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
                    OGRPolygon* polygon = (OGRPolygon*)geometry;
                    meshes.add(centerX, centerY, height, polygon);
                }
                else if (wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon) {
                    OGRMultiPolygon* multipolygon = (OGRMultiPolygon*)geometry;
                    for (int j = 0; j < multipolygon->getNumGeometries(); j++) {
                        OGRPolygon* polygon = (OGRPolygon*)multipolygon->getGeometryRef(j);
                        meshes.add(centerX, centerY, height, polygon);;
                    }
                }
                else {
                    qWarning() << "only support Polygon(MultiPolygon)";
                }
            }
            QByteArray b3dmBuffer = meshes.toB3DM(true);
            QFile b3dmFile = QString("%1/tile/%2/%3/%4.b3dm").
                arg(output).
                arg(root->level()).
                arg(root->row()).
                arg(root->col());
            if (!b3dmFile.open(QIODevice::WriteOnly)){
                qCritical() << "can't write file" << b3dmFile.fileName();
                return;
            }

            int writeBytes = b3dmFile.write(b3dmBuffer);
            if (writeBytes <= 0){
                qCritical() << "can't write file" << b3dmFile.fileName();
                return;
            }

            RootTile child;
            child.boundingVolume = BoundingVolumeRegion::fromCenterXY(
                centerX, centerY,
                boxWidth, boxHeight,
                0, maxHeight);
            child.transform = Transform::fromXYZ(centerX, centerY, 0);
            child.content.emplace();
            child.content->uri = QString("./tile/%1/%2/%3.b3dm")
                .arg(root->level())
                .arg(root->row())
                .arg(root->col());
            tile.root.children.append(child);
        });

        tile.asset.assets["version"] = "1.0";
        tile.asset.assets["gltfUpAxis"] = "Z";
        tile.geometricError = 200;
        tile.root.geometricError = 200;
        BoundingVolumeRegion rootBounding;
        rootBounding.west = osg::DegreesToRadians(extent_.MinX);
        rootBounding.east = osg::DegreesToRadians(extent_.MaxX);
        rootBounding.south = osg::DegreesToRadians(extent_.MinY);
        rootBounding.north = osg::DegreesToRadians(extent_.MaxY);
        rootBounding.minHeight = 0;
        rootBounding.maxHeight = layerMaxHeight;
        tile.root.boundingVolume = rootBounding;
        QByteArray tileBuffer = QJsonDocument(tile.write()).toJson(); 
        QFile tileFile(output + "/tileset.json");
        if (!tileFile.open(QIODevice::WriteOnly)){
            qCritical() << "can't write file:" << tileFile.fileName();
             return false;
        }
        int writeBytes = tileFile.write(tileBuffer);
        if (writeBytes <= 0){
            qCritical() << "can't write file:" << tileFile.fileName();
            return false;
        }
        return true;
    } 
}
