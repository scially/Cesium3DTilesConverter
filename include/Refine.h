#pragma once

#include <QObject>
#include <BaseObject.h>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaEnum>

namespace scially {
    /// <summary>
    /// Refinement determines the process by which a lower resolution parent tile renders
    /// when its higher resolution children are selected to be rendered.
    /// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
    /// </summary>
    class Refine : public BaseObject {
    public:
        virtual void read(const QJsonValue& object) override;
        virtual QJsonValue write() override;
        virtual QString typeName() override {
            return "refine";
        }
        Refine& operator =(const QString &type) {
            this->type = type;
            return *this;
        }

        Refine() = default;
        QString type = "REPLACE";
    };

}
