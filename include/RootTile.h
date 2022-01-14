#pragma once

#include <BaseObject.h>
#include <TilesParseException.h>
#include <QVector>
#include <QString>
#include <BoundingVolume.h>
#include <ContentTile.h>
#include <Refine.h>
#include <TileMatrix.h>

namespace gzpi {

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
    class RootTile : public BaseObject {
    public:
        using RootTilePtr = QSharedPointer<RootTile>;

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "root";
        }

        RootTile();
        BoundingVolume::BoundingVolumePtr boundingVolume;
        TileMatrix::TileMatrixPtr         transform;
        double                            geometricError;
        Refine::RefinePtr                 refine;
        ContentTile::ContentTilePtr       content;
        QVector<RootTilePtr>              children;
    };

}