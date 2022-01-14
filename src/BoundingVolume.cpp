#include <BoundingVolume.h>

namespace gzpi {

    BoundingVolume::BoundingVolumePtr BoundingVolume::create(const QJsonValue& object) {
        BoundingVolumePtr boundingVolume;
        QString boundingVolumeType = object.toObject().keys().at(0);
        if (boundingVolumeType == "box") {
            boundingVolume = QSharedPointer<BoundingVolumeBox>::create();
            boundingVolume->read(object["box"]);
        }
        else if (boundingVolumeType == "region") {
            boundingVolume = QSharedPointer<BoundingVolumeRegion>::create();
            boundingVolume->read(object["region"]);
        }
        else if (boundingVolumeType == "sphere") {
            boundingVolume = QSharedPointer<BoundingVolumeSphere>::create();
            boundingVolume->read(object["sphere"]);
        }
        else {
            throw ThreedTilesParseException("box | region | sphere is required");
        }
        return boundingVolume;
    }
}