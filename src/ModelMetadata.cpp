#include <ModelMetadata.h>
#include <TilesConvertException.h>

#include <QFile>
#include <QDomDocument>
#include <ogrsf_frmts.h>

namespace scially {
    void ModelMetadata::loadFromFile(const QString &input){
        QFile metaDataFile(input);
        if(!metaDataFile.exists()){
            throw TilesConvertException("can't find metadata.xml file");
        }

        QDomDocument metaDataDom;
        if(!metaDataDom.setContent(&metaDataFile)){
            throw TilesConvertException("can't parse metadata.xml file");
        }

        QDomElement rootElement = metaDataDom.documentElement();
        if(rootElement.tagName() != "ModelMetadata"){
            throw TilesConvertException("can't find ModelMetaData node in metadata.xml");
        }

        QString srs = rootElement.firstChildElement("SRS").text();
        QString srsOrigin = "0,0,0";
        if(!rootElement.firstChildElement("SRSOrigin").isNull()){
            srsOrigin = rootElement.firstChildElement("SRSOrigin").text();
        }
        srs_ = srs;
        const QStringList split = srsOrigin.split(",");
        if(split.size() < 3){
            throw TilesConvertException("SRSOrigin is not valid");
        }
        srsOrigin_.x() = split[0].toDouble();
        srsOrigin_.y() = split[1].toDouble();
        srsOrigin_.z() = split[2].toDouble();

        transformToWGS();
    }

    void ModelMetadata::transformToWGS(){
        if(srs_.left(3) == "ENU"){
            srs_ = "EPSG:4326";
            QStringList coordinates = srs_.mid(4).split(",");
            if(coordinates.size() < 2){
                throw TilesConvertException("ENU format is not valid");
            }
            srsOrigin_.x() = coordinates[0].toDouble();
            srsOrigin_.y() = coordinates[1].toDouble();
            if(coordinates.size() == 3)
                srsOrigin_.z() = coordinates[2].toDouble();
            return;
        }

        OGRSpatialReference sourceSRS, targetSRS;
        double tX = srsOrigin_.x(), tY = srsOrigin_.y(), tZ = srsOrigin_.z();
        if(srs_.left(4) == "EPSG"){ // EPSG:4326
            sourceSRS.importFromEPSG(srs_.mid(5).toInt());
        } else {
            sourceSRS.importFromWkt(srs_.toUtf8());
        }
        
        targetSRS.importFromEPSG(4326);
        std::unique_ptr<OGRCoordinateTransformation> transform(OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS));
        if(transform->Transform(1, &tX, &tY)){
            srsOrigin_.x() = tX;
            srsOrigin_.y() = tY;
            srsOrigin_.z() = tZ;
            srs_ = "EPSG:4326";
        } else {
            throw TilesConvertException(QString("can't transform from %1 to WGS84").arg(srs_));
        }
    }
}
