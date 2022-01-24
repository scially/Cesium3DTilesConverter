#pragma once

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <BoundingVolume.h>
#include <TilesParseException.h>

namespace gzpi {
    /// <summary>
    /// The boundingVolume.box property is an array of 12 numbers 
    /// that define an oriented bounding box in a right-handed 3-axis (x, y, z) Cartesian coordinate system 
    /// where the z-axis is up. 
    /// The first three elements define the x, y, and z values for the center of the box. 
    /// The next three elements (with indices 3, 4, and 5) define the x-axis direction and half-length. 
    /// The next three elements (indices 6, 7, and 8) define the y-axis direction and half-length. 
    /// The last three elements (indices 9, 10, and 11) define the z-axis direction and half-length.
    /// </summary>
    class BoundingVolumeBox : public BoundingVolume {
    public:
        double centerX;
        double centerY;
        double centerZ;

        double directionX0;
        double directionX1;
        double halfXLength;

        double directionY0;
        double directionY1;
        double halfYLength;

        double directionZ0;
        double directionZ1;
        double halfZLength;

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual ~BoundingVolumeBox() {}
    };
}