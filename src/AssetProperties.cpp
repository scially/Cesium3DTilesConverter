#include <AssetProperties.h>

namespace gzpi {
    QJsonValue AssetProperties::write() {
        QJsonObject object;
        for (auto iter = assets.constKeyValueBegin(); iter != assets.constKeyValueEnd(); iter++) {
            object[iter->first] = iter->second;
        }
        return object;
    }

    void AssetProperties::read(const QJsonValue& object) {
        assets.clear();
        if (!object.isObject())
            throw TilesParseException("asset is required");

        for (const auto& key : object.toObject().keys()) {
            assets[key] = object[key].toString();
        }
    }

}
