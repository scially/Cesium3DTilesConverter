#include <Cesium3DTiles/AssetProperties.h>

namespace scially {
    QJsonObject AssetProperties::write() const {
        QJsonObject object = QJsonObject::fromVariantMap(assets);
        return object;
    }

    void AssetProperties::read(const QJsonObject& object) {
        assets.clear();
     
        for (const auto& key : object.keys()) {
            assets[key] = object[key].toString();
        }
    }
}
