#pragma once

#include <Cesium3DTiles/Asset.h>

#include <QJsonValue>
#include <QJsonObject>

namespace scially {
    class AssetReadWriter {
    public:
        Asset readFromJson(const QJsonValue& value) const {
            QJsonObject assetJsonValue = value.toObject();
            Asset asset;
            
            asset.version = assetJsonValue["version"].toString();

            for (const auto& key : assetJsonValue.keys()) {
                asset.assets[key] = assetJsonValue[key].toString();
            }

            return asset;
        }

        QJsonObject writeToJson(const Asset& asset) const {
            QJsonObject object = QJsonObject::fromVariantMap(asset.assets);
            object["version"] = asset.version;
            object["generagetool"] = asset.generagetool;
            return object;
        }
    };
}