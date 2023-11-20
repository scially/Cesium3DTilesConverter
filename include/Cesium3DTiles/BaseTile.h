#pragma once

#include "Asset.h"
#include "RootTile.h"

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
        Asset asset;
        double geometricError;
        RootTile root;
    };
}
