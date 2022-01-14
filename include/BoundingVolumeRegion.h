#pragma once

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
        double west;
        double south;
        double east;
        double north;
        double minHeight;
        double maxHeight;

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;

    };
}