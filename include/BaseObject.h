#pragma once

#include <QSharedPointer>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
namespace scially {
    class BaseObject {
    public:
        using BasePtr = QSharedPointer<BaseObject>;

        virtual QJsonValue write() = 0;
        virtual void read(const QJsonValue& object) = 0;
        virtual QString typeName() = 0;
        virtual ~BaseObject() {}
    protected:
        static bool required(const QJsonValue& object, QJsonValue::Type type) {
            return object.type() == type;
        }
    };
}