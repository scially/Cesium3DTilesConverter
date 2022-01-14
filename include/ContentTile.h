#pragma once


#include <BaseObject.h>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <BoundingVolume.h>

namespace gzpi {
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
    class ContentTile : public BaseObject {
    public:
        using ContentTilePtr = QSharedPointer<ContentTile>;
       
        ContentTile() {}
        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "content";
        }


        BoundingVolume::BoundingVolumePtr boundingVolume;
        QString uri;
    };

}