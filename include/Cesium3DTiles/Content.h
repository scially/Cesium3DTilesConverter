#pragma once

#include <Cesium3DTiles/BoundingVolume.h>

#include <QJsonValue>
#include <QJsonObject>

namespace scially {
    /// <summary>
    /// Metadata about the tile's content and a link to the content.
    /// -------------------------------------------------------------------------------------------------
    /// Name           |Description                                            |Type            |Required
    /// -------------------------------------------------------------------------------------------------
    /// boundingVolume |A bounding volume that encloses a tile or its content. |AssetProperties |
    /// -------------------------------------------------------------------------------------------------
    /// uri            |A uri that points to the tile's content.               |string          |Required
    /// -------------------------------------------------------------------------------------------------
    /// </summary>
    struct Content {
        static constexpr const char* TypeName = "content";
       
        QJsonObject write() const;
        void read(const QJsonObject& object);

        BoundingVolume boundingVolume;
        QString uri;
    };

}
