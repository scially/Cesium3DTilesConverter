#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <initializer_list>

#include <QSharedPointer>
#include <BaseObject.h>
#include <AssetProperties.h>
#include <RootTile.h>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QJsonDocument>
#include <TilesParseException.h>

namespace gzpi {
    /// <summary>
    /// A 3D Tiles tileset.
    /// -----------------------------------------------------------------------------
    /// Name           |Description                        |Type            |Required
    /// -----------------------------------------------------------------------------
    /// asset          |Metadata about the entire tileset. |AssetProperties |Required
    /// -----------------------------------------------------------------------------
    /// geometricError |                                   |double          |Required
    /// -----------------------------------------------------------------------------
    /// root           |A tile in a 3D Tiles tileset.      |ContentTile     |Required
    /// -----------------------------------------------------------------------------
    /// </summary>
    class BaseTile : public BaseObject {
    public:
        using BaseTilePtr = QSharedPointer<BaseTile>;

        BaseTile::BaseTile() {}
        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
    
        virtual QString typeName() override {
            return "";
        }
    
        BaseTile();
        AssetProperties::AssetPropertiesPtr asset;
        double geometricError;
        RootTile::RootTilePtr root;
    };
    
}
