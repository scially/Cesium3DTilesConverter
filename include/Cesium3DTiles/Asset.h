#pragma once

#include <QVariantMap>
#include <QString>

namespace scially {
    struct Asset final {
        static constexpr const char* TypeName = "asset";

        QString version = "1.0";
        QString generagetool = "Cesium3DTilesConverter(imhwang@126.com)";
        QVariantMap assets;
    };
}