#include <BoundingVolumeSphere.h>
#include <QJsonValue>
#include <QDebug>

namespace gzpi {

    QJsonValue BoundingVolumeSphere::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(centerX);
        array.append(centerY);
        array.append(centerZ);
        array.append(radius);

        object["sphere"] = array;
        return object;
    }

    void BoundingVolumeSphere::read(const QJsonValue& object) {
        if(!object.isArray()){
            qCritical() << "parse BoundingVolumeSphere: " << object << "failed\n";
            return;
        }

        centerX = object[0].toDouble();
        centerY = object[1].toDouble();
        centerZ = object[2].toDouble();
        radius = object[3].toDouble();
    }

}
