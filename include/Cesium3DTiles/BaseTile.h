#pragma once

#include <Cesium3DTiles/AssetProperties.h>
#include <Cesium3DTiles/RootTile.h>

#include <QJsonObject>
#include <QJsonDocument>

namespace scially {
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
    struct BaseTile{
        QJsonObject write() const;
        void read(const QJsonObject& object);
    
        AssetProperties asset;
        double geometricError;
        RootTile root;
    };
}
