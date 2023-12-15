#pragma once

#include "BoundingVolume.h"
#include "Content.h"

#include <optional>

#include <QVector>

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
    struct RootTile final{
        static constexpr const char* TypeName = "root"; 
        static const QVector<double> TRANSFORM;

        BoundingVolume  boundingVolume;
        QVector<double> transform = TRANSFORM;
        double          geometricError = 0;
        
        /// <summary>
        /// Refinement determines the process by which a lower resolution parent tile renders
        /// when its higher resolution children are selected to be rendered.
        /// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
        /// </summary>
        QString  refine;

        std::optional<Content>  content;
        QVector<RootTile>       children;
    };

}
