#pragma once

#include <Cesium3DTiles/BoundingVolumeRegion.h>

#include <QJsonArray>

namespace scially {
    /// <summary>
    /// The boundingVolume.box property is an array of 12 numbers 
    /// that define an oriented bounding box in a right-handed 3-axis (x, y, z) Cartesian coordinate system 
    /// where the z-axis is up. 
    /// The first three elements define the x, y, and z values for the center of the box. 
    /// The next three elements (with indices 3, 4, and 5) define the x-axis direction and half-length. 
    /// The next three elements (indices 6, 7, and 8) define the y-axis direction and half-length. 
    /// The last three elements (indices 9, 10, and 11) define the z-axis direction and half-length.
    /// </summary>
    struct BoundingVolumeBox{
        static constexpr const char* TypeName = "box";

        QJsonArray write() const;
        void read(const QJsonArray& object);
        double geometricError() const noexcept;
		
        double centerX = 0;
        double centerY = 0;
        double centerZ = 0;

        double directionX0 = 0;
        double directionX1 = 0;
        double halfXLength = 0;

        double directionY0 = 0;
        double directionY1 = 0;
        double halfYLength = 0;

        double directionZ0 = 0;
        double directionZ1 = 0;
        double halfZLength = 0;
    };
}
