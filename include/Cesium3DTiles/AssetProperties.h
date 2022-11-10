#pragma once

#include <QJsonObject>
#include <QVariantMap>

namespace scially {
    struct AssetProperties {
        static constexpr const char* TypeName = "asset";

        QJsonObject write() const;
        void read(const QJsonObject& object);

        QVariantMap assets;
    };
}

