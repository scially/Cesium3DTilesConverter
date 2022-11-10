#include <Cesium3DTiles/BoundingVolumeBox.h>

namespace scially {
    QJsonArray BoundingVolumeBox::write() const{
        QJsonArray array;
        array.append(centerX());      array.append(centerY());      array.append(centerZ());
        array.append(halfXLength());  array.append(directionX0());  array.append(directionX1());
        array.append(directionY0());  array.append(halfYLength());  array.append(directionY1());
        array.append(directionZ0());  array.append(directionZ1());  array.append(halfZLength());
        return array;
    }

    void BoundingVolumeBox::read(const QJsonArray& object) {
        double centerX = object[0].toDouble(),        centerY = object[1].toDouble(),        centerZ = object[2].toDouble();
        double directionX0 = object[4].toDouble(),    directionX1 = object[5].toDouble(),    halfXLength = object[3].toDouble();
        double directionY0 = object[6].toDouble(),    directionY1 = object[8].toDouble(),    halfYLength = object[7].toDouble();
        double directionZ0 = object[9].toDouble(),    directionZ1 = object[10].toDouble(),   halfZLength = object[11].toDouble();

        region.east = centerX + halfXLength;
        region.west = centerX - halfXLength;
        region.north = centerY + halfYLength;
        region.south = centerY - halfYLength;
        region.maxHeight = centerZ + halfZLength;
        region.minHeight = centerZ - halfZLength;
    }

    void BoundingVolumeBox::setMax(const osg::Vec3d& max){
        region.setMax(max);
    }
    void BoundingVolumeBox::setMin(const osg::Vec3d& min){
        region.setMax(min);
    }
    void BoundingVolumeBox::setMax(const osg::Vec3f& max){
        region.setMax(max);
    }
    void BoundingVolumeBox::setMin(const osg::Vec3f& min){
        region.setMin(min);
    }
    void BoundingVolumeBox::mergeMax(const osg::Vec3d& max) {
        region.mergeMax(max);
    }

    void BoundingVolumeBox::mergeMin(const osg::Vec3d & min) {
        region.mergeMin(min);
    }

    void BoundingVolumeBox::mergeMax(const osg::Vec3f& max) {
        region.mergeMax(max);
    }

    void BoundingVolumeBox::mergeMin(const osg::Vec3f& min) {
       region.mergeMin(min);
    }

    osg::Vec3d BoundingVolumeBox::getMax() const {
        return region.getMax();
    }
    osg::Vec3d BoundingVolumeBox::getMin() const {
        return region.getMax();
    }
    double BoundingVolumeBox::geometricError() const {
        return region.geometricError();
    }

    BoundingVolumeBox BoundingVolumeBox::merge(BoundingVolumeBox bounding) const {
      BoundingVolumeBox box(region.merge(bounding.region));
      return box;
   }
}
