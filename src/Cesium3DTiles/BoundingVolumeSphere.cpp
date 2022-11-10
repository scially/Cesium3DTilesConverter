#include <Cesium3DTiles/BoundingVolumeSphere.h>

#include <QJsonValue>

namespace scially {
    QJsonArray BoundingVolumeSphere::write() const {
        QJsonArray array;
        array.append(centerX);
        array.append(centerY);
        array.append(centerZ);
        array.append(radius);
        return array;
    }

    void BoundingVolumeSphere::read(const QJsonArray& object) {
        centerX = object[0].toDouble();
        centerY = object[1].toDouble();
        centerZ = object[2].toDouble();
        radius = object[3].toDouble();
    }

}
