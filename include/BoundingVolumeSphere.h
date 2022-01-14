#pragma once

#include <BoundingVolume.h>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
namespace gzpi {
    /// <summary>
    /// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
    /// The first three elements define the x, y, and z values 
    /// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
    /// The last element (with index 3) defines the radius in meters.
    /// </summary>
    class BoundingVolumeSphere : public BoundingVolume {
    public:
        double centerX;
        double centerY;
        double centerZ;
        double radius;

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
    };
}