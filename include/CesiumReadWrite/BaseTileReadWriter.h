#pragma once

#include "AssetReadWriter.h"
#include "RootTileReadWriter.h"

#include <Cesium3DTiles/BaseTile.h>

#include <QJsonObject>

namespace scially {

    class BaseTileReadWriter{
    public:
        BaseTile readFromJson(const QJsonValue& value) const {
            BaseTile tile;
            tile.asset = mAssetReadWriter.readFromJson(value["asset"]);
            tile.geometricError = value["geometricError"].toDouble();
            tile.root = mRootTileReadWriter.readFromJson(value["root"]);
            return tile;
        }

        QJsonObject writeToJson(const BaseTile& tile) const {
            QJsonObject object;
            object["asset"] = mAssetReadWriter.writeToJson(tile.asset);
            object["geometricError"] = tile.geometricError;;
            object["root"] = mRootTileReadWriter.writeToJson(tile.root);
            return object;
        }
    private:
        RootTileReadWriter mRootTileReadWriter;
        AssetReadWriter mAssetReadWriter;
    };
}
