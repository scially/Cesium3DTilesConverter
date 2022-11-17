#include <OSGBConvertJob.h>
#include <ModelMetadata.h>
#include <TilesConvertException.h>

#include <QDir>

namespace scially {

    void OSGBConvertJob::run() {
        ModelMetadata metadata;
        double lon, lat;
        metadata.parse(input_ + "/metadata.xml");
        metadata.getCoordinate(lon, lat);

        QDir dataDir(input_ + "/Data");
        if(!dataDir.exists()){
            qCritical() << "can't find Data in:" << input_;
            return;
        }

        QFileInfoList tileDirs = dataDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot , QDir::Name);
        for(auto iter = tileDirs.constBegin(); iter != tileDirs.constEnd(); iter++){
            QString tileLocation = iter->absoluteFilePath() + "/" + iter->fileName() + OSGLevel::OSGBEXTENSION;
            auto task = new OSGBConvertTask(tileLocation, output_, maxLevel_, this);
            task->setYUpAxis(yUpAxis_);
            tasks_.append(task);
            threadPool_->start(task);
        }
        threadPool_->waitForDone();

        // merge children tiles
        BaseTile baseTile;
        baseTile.geometricError = 2000;
        baseTile.asset.assets["gltfUpAxis"] = yUpAxis_ ? "Y" : "Z";
        baseTile.asset.assets["version"] = "1.0";
        baseTile.root.transform = Transform::fromXYZ(lon, lat, height_);
        baseTile.root.geometricError = 1000;
        OSGBRegion mergeBox;
        for(auto iter = tasks_.constBegin(); iter != tasks_.constEnd(); ++iter){
            OSGBConvertTask* task = *iter;
            if(task->isSuccess()){
                // for this child
                RootTile childTile = task->baseTile().root;
                mergeBox = mergeBox.merge(task->region());
                Content content;
                content.uri = "./" + task->tileName() + "/tileset.json";
                childTile.content = content;
                childTile.children.clear();
                baseTile.root.children.append(childTile);
            }
        }
        baseTile.root.boundingVolume.box = mergeBox.toBoundingVolumeBox();

        QFile tilesetjsonFile(output_ + "/tileset.json");
        if(!tilesetjsonFile.open(QIODevice::WriteOnly)){
            qCritical("can't write tileset.json in %s", output_);
            return;
        }
        
        tilesetjsonFile.write(QJsonDocument(baseTile.write()).toJson(QJsonDocument::Indented));
    }
}
