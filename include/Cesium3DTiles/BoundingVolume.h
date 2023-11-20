#pragma once

#include <optional>

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
    struct BoundingVolumeBox {
        static constexpr const char* TypeName = "box";

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

    /// <summary>
    /// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
    /// with latitude, longitude, and height coordinates with the order 
    /// [west, south, east, north, minimum height, maximum height]. 
    /// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
    /// Heights are in meters above (or below) the WGS 84 ellipsoid.
    /// </summary>
    struct BoundingVolumeRegion {
        static constexpr const char* TypeName = "region";

        double west = 0;
        double south = 0;
        double east = 0;
        double north = 0;
        double minHeight = 0;
        double maxHeight = 0;
    };

    /// <summary>
    /// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
    /// The first three elements define the x, y, and z values 
    /// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
    /// The last element (with index 3) defines the radius in meters.
    /// </summary>
    struct BoundingVolumeSphere {
        static constexpr const char* TypeName = "sphere";

        double centerX = 0;
        double centerY = 0;
        double centerZ = 0;
        double radius = 0;
    };

    struct BoundingVolume {
        static constexpr const char* TypeName = "boundingVolume";

        std::optional<BoundingVolumeBox> box;
        std::optional<BoundingVolumeRegion> region;
        std::optional<BoundingVolumeSphere> sphere;
    };
}

