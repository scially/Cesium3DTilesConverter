#include <Cesium3DTiles/BaseTile.h>
#include <OSGBLevel.h>
#include <DxtImage.h>
#include <OSGBConvert.h>

#include <QJsonDocument>
#include <QDir>

namespace scially {
    QString OSGLevel::getTileName() const {
        int p0 = nodeName_.indexOf("_L");
        if (p0 < 0)
            return nodeName_;
        return nodeName_.left(p0);
    }

    int OSGLevel::getLevelNumber() const {
        int p0 = nodeName_.indexOf("_L");
        if (p0 < 0)
            return 0;
        int p1 = nodeName_.indexOf("_", p0 + 1);
        if (p1 < 0)
            return 0;
        return nodeName_.mid(p0 + 2, p1 - p0 - 2).toInt();
    }

    void OSGLevel::createDir(const QString& output) const {
        for (int i = 0; i < subNodes_.size(); i++) {
            QDir createDir(output + '/' + subNodes_[i].getTileName());
            if (!createDir.exists()) {
                if (!createDir.mkpath("."))
                    qWarning("can't create dir: %s", createDir.absolutePath());
            }
        }
    }

    bool OSGLevel::getAllOSGBLevels(int maxLevel) {
        if (getLevelNumber() < 0 || getLevelNumber() >= maxLevel)
            return false;

        OSGBPageLodVisitor lodVisitor(nodePath_);
        osg::ref_ptr<osg::Node> root = osgDB::readNodeFile(nodeLocation_.toStdString());
        if (root == nullptr) {
            qWarning() << "can't read osgb file:" << nodeLocation_;
            return false;
        }
            
        root->accept(lodVisitor);
        for (int i = 0; i < lodVisitor.subNodeNames.size(); i++) {
            OSGLevel subLevel(lodVisitor.subNodeNames[i]);
            subLevel.setYUpAxis(yUpAxis_);
            if (subLevel.getAllOSGBLevels(maxLevel)) {
                subNodes_.append(subLevel);
            }
        }
        return true;
    }

    bool OSGLevel::convertTiles(BaseTile &tile, const QString& output, int maxLevel) {
        if (!getAllOSGBLevels(maxLevel)) {
            return false;
        }

        createDir(output);

        // convert tiles and update this region
        RootTile childTile;
        if(!convertTiles(childTile, output)){
            return false;
        }

        //update geometry error
        updateGeometryError(childTile);

        // update root tile
        tile.geometricError = 2000;
        tile.asset.assets["gltfUpAxis"] = yUpAxis_ ? "Y" : "Z";
        tile.asset.assets["version"] = "1.0";

        tile.root.children.append(childTile);
        tile.root.boundingVolume.box = this->region_.toBoundingVolumeBox();
        tile.root.geometricError = 1000;

        QFile tilesetFile(output + "/" + getTileName() + "/tileset.json");
        
        if (!tilesetFile.open(QIODevice::WriteOnly)) {
            qWarning() << "can't Write tileset.json in" << tilesetFile.fileName();
            return false;
        }

        return tilesetFile.write(QJsonDocument(tile.write()).toJson(QJsonDocument::Indented)) > 0;
    }

    bool OSGLevel::convertTiles(RootTile &root, const QString& output) {
        OSGBConvert convert(nodeLocation_);
        convert.setYUpAxis(yUpAxis_);

        QByteArray b3dmBuffer;
        if (!convert.toB3DM(b3dmBuffer))
            return false;

        QString outputLocation = QDir::cleanPath(output + '/' + getTileName());
        
        if(!convert.writeB3DM(b3dmBuffer, outputLocation))
            return false;
        
        Content content;
        content.uri = "./" + nodeName_ + B3DMEXTENSION;
        content.boundingVolume.box = convert.region().toBoundingVolumeBox();
       
        root.refine.type = "REPLACE";
        root.content = content;
        root.boundingVolume.box = convert.region().toBoundingVolumeBox();

        region_ = convert.region();

        for(int i = 0; i < subNodes_.size(); i++){
            RootTile child;
            if (subNodes_[i].convertTiles(child, output)) {
                root.children.append(child);
                region_ = region_.merge(subNodes_[i].region());
            }
        }
        root.boundingVolume.box = region_.toBoundingVolumeBox();
        return true;
    }

    void OSGLevel::updateGeometryError(RootTile &root){
        if(root.children.isEmpty()){
            root.geometricError = 0;
            return;
        }
        
        for(auto& tile : root.children)
            updateGeometryError(tile);
        
        root.geometricError = root.children[0].boundingVolume.box->geometricError() * 2;
    }
}
