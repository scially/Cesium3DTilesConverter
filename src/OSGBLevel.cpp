#include <OSGBLevel.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDataStream>
#include <DxtImage.h>
#include <OSGBConvert.h>
#include <BaseTile.h>
#include <TilesConvertException.h>
#include <QDebug>

namespace scially {

    QString OSGBLevel::absoluteLocation() const {
        return QDir(nodePath).filePath(nodeName) + OSGBEXTENSION;
    }

    QString OSGBLevel::getTileName() const {
        int p0 = nodeName.indexOf("_L");
        if (p0 < 0)
            return nodeName;
        return nodeName.left(p0);
    }

    void OSGBLevel::createDir(const QString& output) const {
        for (int i = 0; i < subNodes.size(); i++) {
            QDir createDir(output + '/' + subNodes[i].getTileName());
            if (!createDir.exists()) {
                if (!createDir.mkpath("."))
                    throw TilesConvertException("Can't create dir " + createDir.absolutePath());
            }
        }
    }

    int OSGBLevel::getLevelNumber() const {
        int p0 = nodeName.indexOf("_L");
        if (p0 < 0)
            return 0;
        int p1 = nodeName.indexOf("_", p0 + 2);
        if (p1 < 0)
            return 0;
        return nodeName.mid(p0 + 2, p1 - p0 - 2).toInt();
    }

    bool OSGBLevel::getAllOSGBLevels(int maxLevel) {
        if (getLevelNumber() < 0 || getLevelNumber() >= maxLevel)
            return false;

        OSGBPageLodVisitor lodVisitor(nodePath);

        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString() };
        osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(rootOSGBLocation);

        if (root == nullptr)
            return false;

        root->accept(lodVisitor);

        for (int i = 0; i < lodVisitor.subNodeNames.size(); i++) {
            OSGBLevel subLevel(lodVisitor.subNodeNames[i]);
            if (subLevel.getAllOSGBLevels(maxLevel)) {
                subNodes.append(subLevel);
            }
        }
        return true;
    }

    bool OSGBLevel::convertTiles(BaseTile &tile, const QString& output, int maxLevel) {
        //
        if (!getAllOSGBLevels(maxLevel)) {
            return false;
        }

        createDir(output);

        RootTile childTile;
        if(!convertTiles(childTile, output)){
            return false;
        }

        //update geometry error
        updateGeometryError(childTile);


        // update this region
        this->region = childTile.boundingVolume.box.value();

        // update root tile
        tile.geometricError = 2000;
        tile.asset.assets["gltfUpAxis"] = "Z";
        tile.asset.assets["version"] = "1.0";

        tile.root.children.append(childTile);
        tile.root.boundingVolume = this->region;
        tile.root.geometricError = 1000;
        QJsonDocument jsonDoc(tile.write().toObject());
        QByteArray json = jsonDoc.toJson(QJsonDocument::Indented);
        QFile tilesetFile(output + "/" + getTileName() + "/tileset.json");
        
        if (!tilesetFile.open(QIODevice::WriteOnly)) {
            qCritical() << "Can't Write tileset.json in " << tilesetFile.fileName();
            return false;
        }

        tilesetFile.write(json);
        return true;
    }

    bool OSGBLevel::convertTiles(RootTile &root, const QString& output) {
        OSGBConvert convert(absoluteLocation());
        QByteArray b3dmBuffer = convert.toB3DM();
        if (b3dmBuffer.isEmpty())
            return false;
        //
        QString outputLocation = QDir(output + '/' + getTileName()).absolutePath();
        int writeBytes = convert.writeB3DM(b3dmBuffer, outputLocation);
        if (writeBytes <= 0)
            return false;
        
        ContentTile content;
        content.uri = "./" + nodeName + B3DMEXTENSION;
        content.boundingVolume = BoundingVolumeBox(convert.region);
       
        root.refine = "REPLACE";
        root.content = content;
        root.boundingVolume = BoundingVolumeBox(convert.region);

        for(int i = 0; i < subNodes.size(); i++){
            RootTile child;
            subNodes[i].convertTiles(child, output);
            root.children.append(child);
            root.boundingVolume = root.boundingVolume.box->merge(child.boundingVolume.box.value());  
        }

        return true;
    }

    void OSGBLevel::updateGeometryError(RootTile &root){
        if(root.children.isEmpty()){
            root.geometricError = 0;
            return;
        }
        else{
            for(auto& tile : root.children)
                updateGeometryError(tile);
            root.geometricError = root.children[0].boundingVolume.box->geometricError() * 2;
        }

    }

}
