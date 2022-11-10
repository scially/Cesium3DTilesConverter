#include <Cesium3DTiles/BaseTile.h>
#include <ShpConvert.h>

#include <QtDebug>
#include <QDir>
#include <QFile>

namespace scially {

    void ShpConvert::convertTiles(const QString& output) {
        QuadTree tree;
        GDALDatasetWrapper ds = GDALDatasetWrapper::open(fileName.toStdString().data(), 1);
        OGRLayerWrapper layer = ds.GetLayerByName(layerName.toStdString().data());

        layer.ResetReading();
        tree.setEnvelope(layer.GetExtent());
        
        OGRFeatureWrapper feature = layer.GetNextFeature();
        OGREnvelope layerEnvelope = layer.GetExtent();
        
        while (feature.isValid()) {
            OGREnvelope envelope;
            OGRGeometry* geometry = feature.GetGeometryRef();
            if (geometry == nullptr)
                continue;

            geometry->getEnvelope(&envelope);
          
            tree.add(feature.GetFID(), envelope);
            feature = layer.GetNextFeature();
        }

        int heightIndex = layer.GetLayerDefn()->GetFieldIndex(heightField.toStdString().data());
        if (heightIndex == -1)
            throw TilesConvertException(heightField + "not found in layer");

        BaseTile tile;
        double layerMaxHeight = 0;
        tree.traverse([&layer, &output, &tile, &layerMaxHeight, heightIndex](QuadTree* root) {
            OGREnvelope nodeBox;
            
            // Calc All Geometry Envelope
            {
                for (int i = 0; i < root->geomsSize(); i++) {
                    int fid = root->getGeomFID(i);
                    OGRFeatureWrapper feature = layer.GetFeature(fid);
                    OGRGeometry* geometry = feature.GetGeometryRef();
                    OGREnvelope envelope;
                    geometry->getEnvelope(&envelope);
                    if (nodeBox.IsInit()) {
                        nodeBox.Merge(envelope);
                    }
                    else {
                        nodeBox = envelope;
                    }
                }
            }
          
            // Build 3D Model per geometry
            double centerX = (nodeBox.MinX + nodeBox.MaxX) / 2;
            double centerY = (nodeBox.MinY + nodeBox.MaxY) / 2;
            double boxWidth =  (nodeBox.MaxX - nodeBox.MinX);
            double boxHeight = (nodeBox.MaxY - nodeBox.MinY);
            double maxHeight = 0;
            QDir outputLocation = QString("%1/tile/%2/%3").
                arg(output).
                arg(root->getLevel()).
                arg(root->getRow());
            if (!outputLocation.exists())
                outputLocation.mkpath(".");

            GeometryMesh meshes;
            for (int i = 0; i < root->geomsSize(); i++) {
                int fid = root->getGeomFID(i);
                OGRFeatureWrapper feature = layer.GetFeature(fid);
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
                    qWarning() << "Only support Polygon(MultiPolygon)";
                }
            }
            QByteArray b3dmBuffer = meshes.toB3DM(true);
            QFile b3dmFile = QString("%1/tile/%2/%3/%4.b3dm").
                arg(output).
                arg(root->getLevel()).
                arg(root->getRow()).
                arg(root->getCol());
            if (!b3dmFile.open(QIODevice::WriteOnly)){
                qWarning() << "Can't write file: " << b3dmFile.fileName();
                return;
            }

            int writeBytes = b3dmFile.write(b3dmBuffer);
            if (writeBytes <= 0){
                qWarning() << "Can't write file: " << b3dmFile.fileName();
                return;
            }

            RootTile child;
            child.boundingVolume = BoundingVolumeRegion::fromCenterXY(
                centerX, centerY,
                nodeBox.MaxX - nodeBox.MinX, nodeBox.MaxY - nodeBox.MinY,
                0, maxHeight);
            child.transform = Transform::fromXYZ(centerX, centerY, 0);
            child.content.emplace();
            child.content->uri = QString("./tile/%1/%2/%3.b3dm")
                .arg(root->getLevel())
                .arg(root->getRow())
                .arg(root->getCol());
            tile.root.children.append(child);
        });

        tile.asset.assets["version"] = "1.0";
        tile.asset.assets["gltfUpAxis"] = "Z";
        tile.geometricError = 200;
        tile.root.geometricError = 200;
        BoundingVolumeRegion rootBounding;
        rootBounding.west = osg::DegreesToRadians(layerEnvelope.MinX);
        rootBounding.east = osg::DegreesToRadians(layerEnvelope.MaxX);
        rootBounding.south = osg::DegreesToRadians(layerEnvelope.MinY);
        rootBounding.north = osg::DegreesToRadians(layerEnvelope.MaxY);
        rootBounding.minHeight = 0;
        rootBounding.maxHeight = layerMaxHeight;
        tile.root.boundingVolume = rootBounding;
        QByteArray tileBuffer = QJsonDocument(tile.write()).toJson(); 
        QFile tileFile(output + "/tileset.json");
        if (!tileFile.open(QIODevice::WriteOnly)){
             qWarning() << "Can't write file: " << tileFile.fileName();
             return;
        }
        int writeBytes = tileFile.write(tileBuffer);
        if (writeBytes <= 0){
            qWarning() << "Can't write file: " << tileFile.fileName();
            return;
        }
    }
}
