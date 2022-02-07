#include <ContentTile.h>
#include <TilesParseException.h>

namespace gzpi {

    QJsonValue ContentTile::write() {
        QJsonObject object;
        if (boundingVolume.has_value())
            object["boundingVolume"] = boundingVolume->write();
        object["uri"] = uri;
        return object;
    }

    void ContentTile::read(const QJsonValue& object) {
       if(!object.isObject())
            return;
       uri = object["uri"].toString();

       QJsonValue boundingVolumeJson = object["boundingVolume"];
       if(boundingVolumeJson.isObject()){
           BoundingVolume boundingResult;
           boundingResult.read(boundingVolumeJson);
           boundingVolume = boundingResult;
       }

    }
}
