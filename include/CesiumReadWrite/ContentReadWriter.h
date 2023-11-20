#pragma once

#include <Cesium3DTiles/Content.h>
#include "BoundingVolumeReadWriter.h"

#include <QJsonObject>

namespace scially {

    class ContentReadWriter  {
    public:
        Content readFromJson(const QJsonValue& value) const {
            Content content;
            content.uri = value["uri"].toString();

            if (!value[BoundingVolume::TypeName].isUndefined() &&
                !value[BoundingVolume::TypeName].isNull()) {
                content.boundingVolume = mBoundingVolumeReadWriter.readFromJson(value[BoundingVolume::TypeName]);
            }
            
            return content;
        }

        QJsonValue writeToJson(const Content& content) const {
            QJsonObject object;
            
            if (content.boundingVolume.has_value()) {
                const BoundingVolume& bounding = content.boundingVolume.value();
                object[bounding.TypeName] = mBoundingVolumeReadWriter.writeToJson(bounding);
            }

            object["uri"] = content.uri;
            return object;
        }
    private:
        BoundingVolumeReadWriter mBoundingVolumeReadWriter;
    };

}
