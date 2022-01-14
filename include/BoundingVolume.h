#pragma once

#include <BaseObject.h>
#include <QSharedPointer>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

namespace gzpi {
    class BoundingVolumeBox;
    class BoundingVolumeRegion;
    class BoundingVolumeSphere;

    class BoundingVolume : public BaseObject {
    public:
        using BoundingVolumePtr = QSharedPointer<BoundingVolume>;
        virtual QString typeName() override {
            return "boundingVolume";
        }

        static BoundingVolumePtr create(const QJsonValue& object);
    };
}