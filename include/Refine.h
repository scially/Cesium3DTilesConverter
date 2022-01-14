#pragma once

#include <QObject>
#include <BaseObject>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaEnum>

namespace gzpi {
    /// <summary>
       /// Refinement determines the process by which a lower resolution parent tile renders 
       /// when its higher resolution children are selected to be rendered. 
       /// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
       /// </summary>
    class Refine : public QObject, public BaseObject {
        Q_OBJECT

    public:
        using RefinePtr = QSharedPointer<Refine>;
        enum RefineType {
            ADD,
            REPLACE
        };
        Q_ENUM(RefineType);

        virtual void read(const QJsonValue& object) override;
        virtual QJsonValue write() override;
        virtual QString typeName() override {
            return "refine";
        }
        QString toString() const;

        Refine(RefineType type = ADD) : type_(type) {}

        RefineType type_;
    private:
        QMetaEnum typeMeta = QMetaEnum::fromType<RefineType>();
    };

}