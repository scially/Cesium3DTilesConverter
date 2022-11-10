#include <Cesium3DTiles/BoundingVolumeRegion.h>
#include <Utils.h>

namespace scially {

    QJsonArray BoundingVolumeRegion::write() const {
        QJsonArray array;
        array.append(west);
        array.append(south);
        array.append(east);
        array.append(north);
        array.append(minHeight);
        array.append(maxHeight);
        return array;
    }

    void BoundingVolumeRegion::read(const QJsonArray& object) {
        west = object[0].toDouble();
        south = object[1].toDouble();
        east = object[2].toDouble();
        north = object[3].toDouble();
        minHeight = object[4].toDouble();
        maxHeight = object[5].toDouble();
    }


    osg::Vec3d BoundingVolumeRegion::getMax() const {
        return osg::Vec3d(west, south, minHeight);
    }
    osg::Vec3d BoundingVolumeRegion::getMin() const {
        return osg::Vec3d(east, north, maxHeight);
    }

    void BoundingVolumeRegion::setMax(const osg::Vec3d& max) {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void BoundingVolumeRegion::setMin(const osg::Vec3d& min) {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void BoundingVolumeRegion::setMax(const osg::Vec3f& max) {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void BoundingVolumeRegion::setMin(const osg::Vec3f& min) {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void BoundingVolumeRegion::mergeMax(const osg::Vec3d& max) {
        osg::Vec3d mergeValue = max < getMax() ? getMax() : max;
        setMax(mergeValue);
    }

    void BoundingVolumeRegion::mergeMin(const osg::Vec3d & min) {
        osg::Vec3d mergeValue = min < getMin() ? min : getMin();
        setMin(mergeValue);
    }

    void BoundingVolumeRegion::mergeMax(const osg::Vec3f& max) {
        osg::Vec3d _max(max[0], max[1], max[2]);
        mergeMax(_max);
    }

    void BoundingVolumeRegion::mergeMin(const osg::Vec3f& min) {
        osg::Vec3d _min(min[0], min[1], min[2]);
        mergeMin(_min);
    }

    BoundingVolumeRegion BoundingVolumeRegion::toRadin(double lon, double lat) const {
        double lonr = osg::DegreesToRadians(lon);
        double latr = osg::DegreesToRadians(lat);

        double west = lonr + meterToLon(this->west, latr);
        double south = latr + meterToLat(this->south);

        double east = lonr + meterToLon(this->east, latr);
        double north = latr + meterToLat(this->north);

        BoundingVolumeRegion region;
        region.west = west;
        region.south = south;
        region.east = east;
        region.north = north;
        region.minHeight = minHeight;
        region.maxHeight = maxHeight;
        return region;
    }

    BoundingVolumeRegion BoundingVolumeRegion::fromCenterXY(double centerX, double centerY, double xDiff, double yDiff, double minHeight, double maxHeight) {
        BoundingVolumeRegion region;
        double centerXr = osg::DegreesToRadians(centerX);
        double centerYr = osg::DegreesToRadians(centerY);
        double xDiffr = osg::DegreesToRadians(xDiff) * 1.05;
        double yDiffr = osg::DegreesToRadians(yDiff) * 1.05;
        region.west = centerXr - xDiffr / 2;
        region.east = centerXr + xDiffr / 2;  
        region.north = centerYr + yDiffr / 2;
        region.south = centerYr - yDiffr / 2;
        region.minHeight = minHeight;
        region.maxHeight = maxHeight;
        return region;  
    }

}
