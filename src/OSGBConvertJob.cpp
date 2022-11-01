#include <OSGBConvertJob.h>
#include <ModelMetadata.h>
#include <TilesConvertException.h>

#include <spdlog/spdlog.h>
#include <QDir>
#include <QThreadPool>
#include <QJsonDocument>
#include <QDomDocument>

namespace scially {

    void OSGBConverter::run() {
        // 解析XML中的坐标
        ModelMetadata metadata;
        metadata.loadFromFile(input_ + "/metadata.xml");
        
        // 遍历Data
        QDir dataDir(input_ + "/Data");
        if(!dataDir.exists()){
            spdlog::error( "can't find Data in {}", input_);
            return;
        }

        QThreadPool *threadPool = new QThreadPool(this);
        QVector<OSGBTileConverterTask*> tasks;
        threadPool->setMaxThreadCount(threadCount_);
        spdlog::info("set process thread to {}", threadPool->maxThreadCount());

        QFileInfoList tileDirs = dataDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot , QDir::Name);
        for(auto iter = tileDirs.constBegin(); iter != tileDirs.constEnd(); iter++){
            QString tileLocation = iter->absoluteFilePath() + "/" + iter->fileName() + OSGBLevel::OSGBEXTENSION;
            auto task =new OSGBTileConverterTask(this);
            task->setTileLocation(tileLocation);
            task->setOutput(output_);
            task->setMaxLevel(maxLevel_);

            tasks.append(task);
            threadPool->start(task);
        }
        threadPool->waitForDone();

        // 合并子节点
        BaseTile baseTile;
        baseTile.geometricError = 2000;
        baseTile.asset.assets["gltfUpAxis"] = "Y";
        baseTile.asset.assets["version"] = "1.0";
        baseTile.root.transform = TileMatrix::fromXYZ(metadata.srsOrigin().y(), metadata.srsOrigin().x(), height_);
        baseTile.root.geometricError = 1000;
        BoundingVolumeBox mergeBox;
        for(auto iter = tasks.constBegin(); iter != tasks.constEnd(); ++iter){
            auto task = *iter;
            if(task->isSuccess()){
                // for this child
                RootTile childTile = task->baseTile().root;
                mergeBox = mergeBox.merge(task->baseTile().root.boundingVolume.box.value());
                ContentTile content;
                content.uri = "./" + task->osgbLevel().getTileName() + "/tileset.json";
                childTile.content = content;
                childTile.children.clear();
                baseTile.root.children.append(childTile);
            }
        }
        baseTile.root.boundingVolume = mergeBox;
        QJsonDocument doc(baseTile.write().toObject());
        QFile tilesetjsonFile(output_ + "/tileset.json");
        if(!tilesetjsonFile.open(QIODevice::WriteOnly)){
            spdlog::error("can't not write tileset.json in {}", output_);
            return;
        }
        int writeBytes = tilesetjsonFile.write(doc.toJson(QJsonDocument::Indented));
        if(writeBytes <= 0){
            spdlog::error("can't not write tileset.json in {}", output_);
            return;
        }
    }
}
