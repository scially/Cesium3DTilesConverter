#pragma once

#include <QObject>
#include <QException>
#include <QString>

namespace scially {
    class TilesParseException : public QException {
    public:
        TilesParseException(const char* err) : err(err) {}
        virtual const char* what() const noexcept override {
            return err;
        }
    private:
        const char* err;
    };
}
