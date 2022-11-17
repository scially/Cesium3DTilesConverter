#pragma once

#include <QObject>
#include <QException>
#include <QString>

namespace scially {
    class TilesConvertException : public QException {
    public:
        TilesConvertException(const QString &err) : err(err) {}
        virtual const char* what() const noexcept override {
            return err.toStdString().data();
        }
    private:
        QString err;
    };

   
}
