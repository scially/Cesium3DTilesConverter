#pragma once

#include "BoundingVolume.h"

#include <optional>

#include <QString>

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

        QString uri;
        std::optional<BoundingVolume> boundingVolume;
    };

}
