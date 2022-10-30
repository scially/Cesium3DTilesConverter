#pragma once

#include <QDebug>
#include <string>

namespace scially {

    class ShpConvertJob{
    public:
        ShpConvertJob(const std::string& input, const std::string& layerName, 
                      const std::string &output, const std::string& height)
            :input(QString::fromStdString(input)), output(QString::fromStdString(output)), 
             height(QString::fromStdString(height)), layerName(QString::fromStdString(layerName)) {}

        void run();

    private:
        QString input;
        QString output;
        QString height;
        QString layerName;
    };

   
}
