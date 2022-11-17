#include <OSGBRegion.h>

namespace scially {
    osg::Vec3d OSGBRegion::getMax() const noexcept {
        return osg::Vec3d(west, south, minHeight);
    }
    osg::Vec3d OSGBRegion::getMin() const noexcept {
        return osg::Vec3d(east, north, maxHeight);
    }

    void OSGBRegion::setMax(const osg::Vec3d& max) noexcept {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void OSGBRegion::setMin(const osg::Vec3d& min) noexcept {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void OSGBRegion::setMax(const osg::Vec3f& max) noexcept {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void OSGBRegion::setMin(const osg::Vec3f& min) noexcept {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void OSGBRegion::mergeMax(const osg::Vec3d& max) noexcept {
        osg::Vec3d mergeValue = max < getMax() ? getMax() : max;
        setMax(mergeValue);
    }

    void OSGBRegion::mergeMin(const osg::Vec3d& min) noexcept {
        osg::Vec3d mergeValue = min < getMin() ? min : getMin();
        setMin(mergeValue);
    }

    void OSGBRegion::mergeMax(const osg::Vec3f& max) noexcept {
        osg::Vec3d _max(max[0], max[1], max[2]);
        mergeMax(_max);
    }

    void OSGBRegion::mergeMin(const osg::Vec3f& min) noexcept {
        osg::Vec3d _min(min[0], min[1], min[2]);
        mergeMin(_min);
    }

    OSGBRegion OSGBRegion::merge(const OSGBRegion& bounding) const noexcept {
        OSGBRegion mergeRegion;
        mergeRegion.west = std::min(bounding.west, west);
        mergeRegion.east = std::max(bounding.east, east);
        mergeRegion.north = std::max(bounding.north, north);
        mergeRegion.south = std::min(bounding.south, south);
        mergeRegion.minHeight = std::min(bounding.minHeight, minHeight);
        mergeRegion.maxHeight = std::max(bounding.maxHeight, maxHeight);
        return mergeRegion;
    }

    BoundingVolumeBox OSGBRegion::toBoundingVolumeBox() const noexcept {
        BoundingVolumeBox box;
        box.centerX = (east + west) / 2;
        box.centerY = (north + south) / 2;
        box.centerZ = (minHeight + maxHeight) / 2;
        box.directionX0 = 0 ;
        box.directionX1 = 0;
        box.halfXLength = (east - west) / 2;
        box.directionY0 = 0;
        box.directionY1 = 0;
        box.halfYLength = (north - south) / 2;
        box.directionZ0 = 0;
        box.directionZ1 = 0;
        box.halfZLength = (maxHeight - minHeight) / 2;
        return box;
    }
}
