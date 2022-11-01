#pragma once

#include <QDebug>

namespace scially {
    class ShpConvertJob{
    public:
        ShpConvertJob(const QString& input, const QString& layerName, const QString &output, const QString& height)
            :input(input), output(output), height(height), layerName(layerName) {}

        void run();

    private:
        QString input;
        QString output;
        QString height;
        QString layerName;
    };

   
}
