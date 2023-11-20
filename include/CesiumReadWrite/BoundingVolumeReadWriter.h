#pragma once

#include <Cesium3DTiles/BoundingVolume.h>

#include <QJsonArray>
#include <QJsonObject>

namespace scially {
    class BoundingVolumeReadWriter {
    public:
        BoundingVolume readFromJson(const QJsonValue& value) const {
            QJsonObject object = value.toObject();
            BoundingVolume bounding;
            
            QString type = object.keys()[0];
            QJsonArray array = object.value(type).toArray();

            if (type == BoundingVolumeBox::TypeName) {
                BoundingVolumeBox box;
                box.centerX =     array[0].toDouble(), box.centerY =     array[1].toDouble(), box.centerZ =     array[2].toDouble();
                box.directionX0 = array[4].toDouble(), box.directionX1 = array[5].toDouble(), box.halfXLength = array[3].toDouble();
                box.directionY0 = array[6].toDouble(), box.directionY1 = array[8].toDouble(), box.halfYLength = array[7].toDouble();
                box.directionZ0 = array[9].toDouble(), box.directionZ1 = array[10].toDouble(),box.halfZLength = array[11].toDouble();
                bounding.box = box;                                                           
            }
            else if(type == BoundingVolumeRegion::TypeName){
                BoundingVolumeRegion region;
                region.west = array[0].toDouble();
                region.south = array[1].toDouble();
                region.east = array[2].toDouble();
                region.north = array[3].toDouble();
                region.minHeight = array[4].toDouble();
                region.maxHeight = array[5].toDouble();
                bounding.region = region;
            }
            else if(type == BoundingVolumeSphere::TypeName){
                BoundingVolumeSphere sphere;
                sphere.centerX = array[0].toDouble();
                sphere.centerY = array[1].toDouble();
                sphere.centerZ = array[2].toDouble();
                sphere.radius =  array[3].toDouble();

                bounding.sphere = sphere;
            }

            return bounding;
        }

        QJsonObject writeToJson(const BoundingVolume& bounding) const {
            QJsonObject object;
            QJsonArray boudingArray;

            if (bounding.box.has_value()) {
                const BoundingVolumeBox& box = bounding.box.value();
                boudingArray.append(box.centerX);      boudingArray.append(box.centerY);      boudingArray.append(box.centerZ);
                boudingArray.append(box.halfXLength);  boudingArray.append(box.directionX0);  boudingArray.append(box.directionX1);
                boudingArray.append(box.directionY0);  boudingArray.append(box.halfYLength);  boudingArray.append(box.directionY1);
                boudingArray.append(box.directionZ0);  boudingArray.append(box.directionZ1);  boudingArray.append(box.halfZLength);
                object[box.TypeName] = boudingArray;
            }
            else if (bounding.region.has_value()) {
                const BoundingVolumeRegion& region = bounding.region.value();
                boudingArray.append(region.west);
                boudingArray.append(region.south);
                boudingArray.append(region.east);
                boudingArray.append(region.north);
                boudingArray.append(region.minHeight);
                boudingArray.append(region.maxHeight);
                object[region.TypeName] = boudingArray;
            }
            else if (bounding.sphere.has_value()) {
                const BoundingVolumeSphere& sphere = bounding.sphere.value();
                boudingArray.append(sphere.centerX);
                boudingArray.append(sphere.centerY);
                boudingArray.append(sphere.centerZ);
                boudingArray.append(sphere.radius);
                object[sphere.TypeName] = boudingArray;
            }
            
            return object;
        }
    };
}