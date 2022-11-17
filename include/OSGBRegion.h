#pragma once

#include <Cesium3DTiles/BoundingVolume.h>

#include <osg/Math>
#include <osg/Vec3d>

#include <limits>

namespace scially {
    struct OSGBRegion {
        osg::Vec3d getMax() const noexcept;
        osg::Vec3d getMin() const noexcept;

        void setMax(const osg::Vec3d& max) noexcept;
        void setMin(const osg::Vec3d& min) noexcept;
        void setMax(const osg::Vec3f& max) noexcept;
        void setMin(const osg::Vec3f& min) noexcept;

        void mergeMax(const osg::Vec3d& max) noexcept;
        void mergeMin(const osg::Vec3d& min) noexcept;
        void mergeMax(const osg::Vec3f& max) noexcept;
        void mergeMin(const osg::Vec3f& min) noexcept;

        double geometricError() const noexcept {
            double maxErr = std::max({ north - south, maxHeight - minHeight, east - west });
            return maxErr / 20.0;
        }

        OSGBRegion merge(const OSGBRegion& bounding) const noexcept;
        BoundingVolumeBox toBoundingVolumeBox() const noexcept;

        double west = std::numeric_limits<double>::max();
        double south = std::numeric_limits<double>::max();
        double east = std::numeric_limits<double>::min();
        double north = std::numeric_limits<double>::min();
        double minHeight = std::numeric_limits<double>::max();
        double maxHeight = std::numeric_limits<double>::min();
    };
}
