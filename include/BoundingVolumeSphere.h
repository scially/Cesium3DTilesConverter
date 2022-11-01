#pragma once

#include <limits>

#include <BaseObject.h>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>


namespace scially {
    /// <summary>
    /// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
    /// The first three elements define the x, y, and z values 
    /// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
    /// The last element (with index 3) defines the radius in meters.
    /// </summary>
    class BoundingVolumeSphere : public BaseObject {
    public:
        double centerX = std::numeric_limits<double>::min();
        double centerY = std::numeric_limits<double>::min();
        double centerZ = std::numeric_limits<double>::min();
        double radius  = std::numeric_limits<double>::min();

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "sphere";
        }
        virtual ~BoundingVolumeSphere() {}
    };
}
