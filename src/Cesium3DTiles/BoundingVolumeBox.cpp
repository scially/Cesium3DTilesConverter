#include <Cesium3DTiles/BoundingVolumeBox.h>

namespace scially {
    QJsonArray BoundingVolumeBox::write() const{
        QJsonArray array;
        array.append(centerX);      array.append(centerY);      array.append(centerZ);
        array.append(halfXLength);  array.append(directionX0);  array.append(directionX1);
        array.append(directionY0);  array.append(halfYLength);  array.append(directionY1);
        array.append(directionZ0);  array.append(directionZ1);  array.append(halfZLength);
        return array;
    }

    void BoundingVolumeBox::read(const QJsonArray& object) {
        centerX = object[0].toDouble(),        centerY = object[1].toDouble(),        centerZ = object[2].toDouble();
        directionX0 = object[4].toDouble(),    directionX1 = object[5].toDouble(),    halfXLength = object[3].toDouble();
        directionY0 = object[6].toDouble(),    directionY1 = object[8].toDouble(),    halfYLength = object[7].toDouble();
        directionZ0 = object[9].toDouble(),    directionZ1 = object[10].toDouble(),   halfZLength = object[11].toDouble();
    }

   
    double BoundingVolumeBox::geometricError() const noexcept {
        double east = centerX + halfXLength;
        double west = centerX - halfXLength;
        double north = centerY + halfYLength;
        double south = centerY - halfYLength;
        double maxHeight = centerZ + halfZLength;
        double minHeight = centerZ - halfZLength;

        double maxErr = std::max({ north - south, maxHeight - minHeight, east - west });
        return maxErr / 20.0;
    }
}
