#pragma once

#include <Cesium3DTiles/BoundingVolume.h>
#include <Cesium3DTiles/Content.h>
#include <Cesium3DTiles/Refine.h>
#include <Cesium3DTiles/Transform.h>

namespace scially {

    /// <summary>
    /// A tile in a 3D Tiles tileset.
    /// --------------------------------------------------------------------------------------------------------
    /// Name           |Description                                                  |Type            |Required
    /// --------------------------------------------------------------------------------------------------------
    /// boundingVolume |A bounding volume that encloses a tile or its content.       |BoundingVolume  |Required
    /// --------------------------------------------------------------------------------------------------------
    /// geometricError |A uri that points to the tile's content.                     |double          |Required
    /// --------------------------------------------------------------------------------------------------------
    /// refine         |                                                             |Refine          |
    /// --------------------------------------------------------------------------------------------------------
    /// transform      |default: [1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]                   |TileMatrix      |
    /// --------------------------------------------------------------------------------------------------------
    /// content        |Metadata about the tile's content and a link to the content. |ContentTile     |
    /// --------------------------------------------------------------------------------------------------------
    /// children       |An array of objects that define child tiles                  |RootTile[]      |
    /// --------------------------------------------------------------------------------------------------------
    /// </summary>
    struct RootTile{
        static constexpr const char* TypeName = "root"; 

        QJsonObject write() const;
        void read(const QJsonObject& object);

        BoundingVolume boundingVolume;
        Transform      transform;
        double         geometricError = 0;
        Refine         refine;
        std::optional<Content>  content;
        QVector<RootTile>       children;
    };

}
