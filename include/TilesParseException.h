#pragma once

#include <QObject>
#include <QException>
#include <QString>

namespace gzpi {
    class TilesParseException : public QException {
    public:
        TilesParseException(const QString& err) : err(err) {}
        virtual const char* what() const override {
            return err.toStdString().c_str();
        }
    private:
        QString err;
    };
}