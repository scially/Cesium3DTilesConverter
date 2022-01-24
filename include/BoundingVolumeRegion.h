#pragma once

#include <limits>

#include <osg/Vec3d>
#include <BoundingVolume.h>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <TilesParseException.h>

namespace gzpi {

    /// <summary>
    /// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
    /// with latitude, longitude, and height coordinates with the order 
    /// [west, south, east, north, minimum height, maximum height]. 
    /// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
    /// Heights are in meters above (or below) the WGS 84 ellipsoid.
    /// </summary>
    class BoundingVolumeRegion : public BoundingVolume {

    public:
        double west = std::numeric_limits<double>::max();
        double south = std::numeric_limits<double>::max();
        double east = std::numeric_limits<double>::min();
        double north = std::numeric_limits<double>::min();
        double minHeight = std::numeric_limits<double>::max();
        double maxHeight = std::numeric_limits<double>::min();

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;

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

        /*纬度变化一度，球面南北方向距离变化：πR/180 ........111.7km
          经度变化一度，球面东西方向距离变化：πR/180*cosB ....111.7*cosB*/
        //BoundingVolumeRegion toRadin(double lon, double lat) const;
        
    };
}