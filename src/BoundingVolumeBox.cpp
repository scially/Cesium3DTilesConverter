#include <BoundingVolumeBox.h>


namespace gzpi {

    QJsonValue BoundingVolumeBox::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(centerX);      array.append(centerY);      array.append(centerZ);
        array.append(directionX0);  array.append(directionX1);  array.append(halfXLength);
        array.append(directionY0);  array.append(directionY1);  array.append(halfYLength);
        array.append(directionZ0);  array.append(directionZ1);  array.append(halfZLength);

        object["box"] = array;
        return object;
    }

    void BoundingVolumeBox::read(const QJsonValue& object) {
        if (!required(object, QJsonValue::Array))
            throw TilesParseException("box is required");

        centerX = object[0].toDouble();        centerY = object[1].toDouble();        centerZ = object[2].toDouble();
        directionX0 = object[3].toDouble();    directionX1 = object[4].toDouble();    halfXLength = object[5].toDouble();
        directionY0 = object[6].toDouble();    directionY1 = object[7].toDouble();    halfYLength = object[8].toDouble();
        directionZ0 = object[9].toDouble();    directionZ1 = object[10].toDouble();   halfZLength = object[11].toDouble();
    }
}