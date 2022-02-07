#pragma once

#include <BaseObject.h>
#include <QJsonValue>
#include <QJsonObject>
#include <QSharedPointer>
#include <QString>
#include <QMap>
#include <TilesParseException.h>

namespace gzpi {
    class AssetProperties : public BaseObject {
    public:
        using AssetPropertiesPtr = QSharedPointer<AssetProperties>;

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;

        virtual QString typeName() override {
            return "asset";
        }
        virtual ~AssetProperties() {}
        QMap<QString, QString> assets;
    };
}

