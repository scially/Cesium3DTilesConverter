#include <BoundingVolume.h>
#include <BoundingVolumeRegion.h>


namespace gzpi {

    QJsonValue BoundingVolumeRegion::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(west);
        array.append(south);
        array.append(east);
        array.append(north);
        array.append(minHeight);
        array.append(maxHeight);

        object["region"] = array;
        return object;
    }

    void BoundingVolumeRegion::read(const QJsonValue& object) {
        if (!required(object, QJsonValue::Array))
            throw TilesParseException("region is required");
        west = object[0].toDouble();
        south = object[1].toDouble();
        east = object[2].toDouble();
        north = object[3].toDouble();
        minHeight = object[4].toDouble();
        maxHeight = object[5].toDouble();
    }
}