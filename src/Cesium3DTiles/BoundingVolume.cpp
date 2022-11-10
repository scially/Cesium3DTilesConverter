#include <Cesium3DTiles/BoundingVolume.h>
#include <TilesConvertException.h>

namespace scially {
    QJsonObject BoundingVolume::write() const {
        QString    key;
        QJsonArray value;

        if (box.has_value()) {
            key = box->TypeName;
            value = box->write();
        }
        else if (region.has_value()) {
            key = region->TypeName;
            value = region->write();
        }
        else if (sphere.has_value()) {
            key = sphere->TypeName;
            value = sphere->write();
        }
        else {
            throw TilesConvertException("BoundingVolume type must be box,region,sphere");
        }
        QJsonObject obj;
        obj.insert(key, value);
        return obj;
    }

    void BoundingVolume::read(const QJsonObject& object) {

        QString key = object.keys().at(0);
        QJsonArray value = object.value(key).toArray();

        if (key == "box") {
            BoundingVolumeBox boxResult;
            boxResult.read(value);
            box = boxResult;
        }
        else if (key == "region") {
            BoundingVolumeRegion regionResult;
            regionResult.read(value);
            region = regionResult;
        }
        else if (key == "sphere") {
            BoundingVolumeSphere sphereResult;
            sphereResult.read(value);
            sphere = sphereResult;
        }
        else {
            throw TilesConvertException("BoundingVolume type must be box,region,sphere");
        }

    }

}