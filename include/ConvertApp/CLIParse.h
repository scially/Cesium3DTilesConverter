#pragma once

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QtDebug>

namespace scially {
   
    class CLIParse
    {
    public:
        enum Format {
            OSGB, Vector
        };

        Format format;
        QString input;
        QString output;

        CLIParse() {
            mParser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
            mParser.setApplicationDescription("Convert OSGB to Cesium 3DTiles");
            mParser.addHelpOption();
            mParser.addVersionOption();
        }

        bool parse() {
            mParser.addOptions({
                {
                    QStringList() << "f" << "format",
                    "OSGB or Vector(required)", "format"
                },
                {
                    QStringList() << "i" << "input",
                    "data input(required)", "input"
                },
                {
                    QStringList() << "o" << "output",
                    "3dtiles output(required)", "output"
                },
                });

            mParser.process(*qApp);
 
            if (!mParser.isSet("format")) {
                qCritical() << "format is required, use --help show more information";
                return false;
            }

            QString formats = mParser.value("format").toUpper();

            if (formats == "Vector") {
                format = Vector;
                return parseVector();
            }
            else if (formats == "OSGB") {
                format = OSGB;
                return parseOSGB();
            }
            else {
                qCritical() << "format must be GDAL or OSGB, use --help show more information";
                return false;
            }    
        }
        
        void showHelp() {
            mParser.showHelp();
        }

    private:
        bool parseOSGB() {
            return parseIO();
        }

        bool parseVector() {
            qCritical() << "Vector to 3DTiles is not supported";
            return false;
        }

        bool parseIO() {
            if (!mParser.isSet("input")) {
                qCritical() << "input is required, use --help show more information";
                return false;
            }

            if (!mParser.isSet("output")) {
                qCritical() << "input is required, use --help show more information";
                return false;

            }
           
            input = mParser.value("input");
            QFileInfo inputFile(input);
            if (!inputFile.exists()) {
                qCritical() << "input file" << input << "not exist";
                return false;
            }
         
            output = mParser.value("output");
            return true;
        }
    
        QCommandLineParser mParser;
    };
}