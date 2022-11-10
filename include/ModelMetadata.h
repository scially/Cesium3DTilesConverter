#pragma once

#include <QString>

namespace scially {
    class ModelMetadata {
    public:
        QString srs;
        QString srsOrigin;
        QString version;

        void parse(const QString &input);
        void getCoordinate(double &lon, double &lat);
    };
}
