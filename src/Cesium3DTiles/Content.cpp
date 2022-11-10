#include <Cesium3DTiles/Content.h>

namespace scially {
    QJsonObject Content::write() const {
        QJsonObject object;
        if (boundingVolume.hasValue())
            object[boundingVolume.TypeName] = boundingVolume.write();
        object["uri"] = uri;
        return object;
    }

    void Content::read(const QJsonObject& object) {
       uri = object["uri"].toString();
       QJsonValue value = object[BoundingVolume::TypeName];
       boundingVolume.read(value.toObject());
    }
}
