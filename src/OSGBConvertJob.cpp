#include <OSGBConvertJob.h>
#include <ModelMetadata.h>
#include <TilesConvertException.h>
#include <QDir>
#include <QDebug>
#include <QDomDocument>

namespace scially {

    void OSGBConvertJob::run() {
        // 解析XML中的坐标
        ModelMetadata metadata;
        metadata.parse(input + "/metadata.xml");

        double lon, lat;
        metadata.getCoordinate(lon, lat);


        // 遍历Data
        QDir dataDir(input + "/Data");
        if(!dataDir.exists()){
            throw TilesConvertException("Can't find Data dir in " + input);
        }

        QFileInfoList tileDirs = dataDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot , QDir::Name);
        for(auto iter = tileDirs.constBegin(); iter != tileDirs.constEnd(); iter++){
            QString tileLocation = iter->absoluteFilePath() + "/" + iter->fileName() + OSGBLevel::OSGBEXTENSION;
            auto task = QSharedPointer<OSGBConvertTask>::create(tileLocation, output, maxLevel);
            tasks.append(task);
            threadPool->start(task.get());
        }
        threadPool->waitForDone();

        // 合并子节点
        BaseTile baseTile;
        baseTile.geometricError = 2000;
        baseTile.asset.assets["gltfUpAxis"] = "Z";
        baseTile.asset.assets["version"] = "1.0";
        baseTile.root.transform = TileMatrix::fromXYZ(lon, lat, height);
        baseTile.root.geometricError = 1000;
        BoundingVolumeBox mergeBox;
        for(auto iter = tasks.constBegin(); iter != tasks.constEnd(); ++iter){
            QSharedPointer<OSGBConvertTask> task = *iter;
            if(task->isSucceed){
                // for this child
                RootTile childTile = task->tile.root;
                mergeBox = mergeBox.merge(task->tile.root.boundingVolume.box.value());
                ContentTile content;
                content.uri = "./" + task->osgbLevel.getTileName() + "/tileset.json";
                childTile.content = content;
                childTile.children.clear();
                baseTile.root.children.append(childTile);
            }
        }
        baseTile.root.boundingVolume = mergeBox;
        QJsonDocument doc(baseTile.write().toObject());
        QFile tilesetjsonFile(output + "/tileset.json");
        if(!tilesetjsonFile.open(QIODevice::WriteOnly)){
            throw TilesConvertException("Can't not write tileset.json in " + output);
        }
        int writeBytes = tilesetjsonFile.write(doc.toJson(QJsonDocument::Indented));
        if(writeBytes <= 0){
            throw TilesConvertException("Can't not write tileset.json in "  + output);
        }
    }
}
