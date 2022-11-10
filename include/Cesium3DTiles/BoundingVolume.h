#pragma once

#include <Cesium3DTiles/BoundingVolumeBox.h>
#include <Cesium3DTiles/BoundingVolumeRegion.h>
#include <Cesium3DTiles/BoundingVolumeSphere.h>

#include <optional>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

namespace scially {
    struct BoundingVolume {
    public:
        static constexpr const char* TypeName = "boundingVolume";

        BoundingVolume() = default;
        BoundingVolume(const BoundingVolumeBox& b) : box(b) {}
        BoundingVolume(const BoundingVolumeRegion& r) : region(r) {}
        BoundingVolume(const BoundingVolumeSphere& s) : sphere(s) {}

        QJsonObject write() const;
        void read(const QJsonObject& object);
        bool hasValue() const {
            return box.has_value() || region.has_value() || sphere.has_value();
        }

        std::optional<BoundingVolumeBox> box;
        std::optional<BoundingVolumeRegion> region;
        std::optional<BoundingVolumeSphere> sphere;
    };
}

