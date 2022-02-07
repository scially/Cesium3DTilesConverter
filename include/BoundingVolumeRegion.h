#pragma once

#include <limits>

#include <osg/Vec3d>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <BaseObject.h>
#include <CoordinateConvert.h>
#include <osg/Math>

namespace gzpi {

    /// <summary>
    /// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
    /// with latitude, longitude, and height coordinates with the order 
    /// [west, south, east, north, minimum height, maximum height]. 
    /// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
    /// Heights are in meters above (or below) the WGS 84 ellipsoid.
    /// </summary>
    class BoundingVolumeRegion : public BaseObject {
    public:
        using BoundingVolumeRegionPtr = QSharedPointer<BoundingVolumeRegion>;
        double west = std::numeric_limits<double>::max();
        double south = std::numeric_limits<double>::max();
        double east = std::numeric_limits<double>::min();
        double north = std::numeric_limits<double>::min();
        double minHeight = std::numeric_limits<double>::max();
        double maxHeight = std::numeric_limits<double>::min();

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "region";
        };

        osg::Vec3d getMax() const;
        osg::Vec3d getMin() const;

        void setMax(const osg::Vec3d& max);
        void setMin(const osg::Vec3d& min);
        void setMax(const osg::Vec3f& max);
        void setMin(const osg::Vec3f& min);

        void mergeMax(const osg::Vec3d& max);
        void mergeMin(const osg::Vec3d& min);
        void mergeMax(const osg::Vec3f& max);
        void mergeMin(const osg::Vec3f& min);

        double geometricError() const {
            double maxErr = std::max({north - south, maxHeight - minHeight, east - west});
            return maxErr / 20.0;
        }
        
         BoundingVolumeRegion merge(BoundingVolumeRegion bounding) const {
           BoundingVolumeRegion mergeRegion;
           mergeRegion.west = std::min(bounding.west, this->west);
           mergeRegion.east = std::max(bounding.east, this->east);
           mergeRegion.north = std::max(bounding.north, this->north);
           mergeRegion.south = std::min(bounding.south, this->south);
           mergeRegion.minHeight = std::min(bounding.minHeight, this->minHeight);
           mergeRegion.maxHeight = std::max(bounding.maxHeight, this->maxHeight);
           return mergeRegion;
        }

        BoundingVolumeRegion toRadin(double lon, double lat) const;

        virtual ~BoundingVolumeRegion() {}
    };
}
