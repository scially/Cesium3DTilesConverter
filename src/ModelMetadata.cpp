#include <ModelMetadata.h>
#include <QIODevice>
#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <CoordinateConvert.h>
#include <TilesConvertException.h>

namespace scially {
    void ModelMetadata::parse(const QString &input){
        QFile metaDataFile(input);
        if(!metaDataFile.exists()){
            throw TilesConvertException("Can't find metadata.xml file");
        }

        QDomDocument metaDataDom;
        if(!metaDataDom.setContent(&metaDataFile)){
            throw TilesConvertException("Can't parse metadata.xml file");
        }

        QDomElement rootElement = metaDataDom.documentElement();
        if(rootElement.tagName() != "ModelMetadata"){
            throw TilesConvertException("Don't find ModelMetaData node in metadata.xml");
        }

        for(int i = 0; i < rootElement.childNodes().size(); i++){
            QDomNode node = rootElement.childNodes().at(i);
            QDomElement element = node.toElement();
            if(!element.isNull() && element.tagName() == "SRS"){
                srs = element.text();
            }
            if(!element.isNull() && element.tagName() == "SRSOrigin"){
                srsOrigin = element.text();
            }
        }
    }

    void ModelMetadata::getCoordinate(double &lon, double &lat){
        if(srs.left(3) == "ENU"){
            QStringList coordinates = srs.mid(4).split(",");
            if(coordinates.size() < 2){
                throw TilesConvertException("ENU format is not right");
            }
            lat = coordinates[0].toDouble();
            lon = coordinates[1].toDouble();
        }
        else if(srs.left(4) == "EPSG"){
            if(srsOrigin.isEmpty()){
                throw TilesConvertException("Don't find ModelMetaData::SRSOrigin node in metadata.xml");
            }
            QString epsg = srs.mid(5);
            QStringList coordinates = srsOrigin.split(",");
            if(coordinates.size() < 2){
                throw TilesConvertException("SRSOrigin format is not right");
            }
            double x = coordinates[0].toDouble();
            double y = coordinates[1].toDouble();
            CoordinateConvert convert(x, y);
            convert.setSourceSrs(epsg, CoordinateConvert::EPSG);
            convert.setTargetSrs("4326", CoordinateConvert::EPSG);
            convert.transform();
            lon = convert.targetX;
            lat = convert.targetY;
        }
        else{// maybe wkt
            if(srsOrigin.isEmpty()){
                throw TilesConvertException("Don't find ModelMetaData::SRSOrigin node in metadata.xml");
            }
            QStringList coordinates = srsOrigin.split(",");
            if(coordinates.size() < 2){
                throw TilesConvertException("SRSOrigin format is not right");
            }
            double x = coordinates[0].toDouble();
            double y = coordinates[1].toDouble();
            CoordinateConvert convert(x, y);
            convert.setSourceSrs(srs, CoordinateConvert::WKT);
            convert.setTargetSrs("4326", CoordinateConvert::EPSG);
            convert.transform();
            lon = convert.targetX;
            lat = convert.targetY;
        }
    }
}
