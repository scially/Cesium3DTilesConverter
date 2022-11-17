#pragma once

#include <QCommandLineParser>
#include <QException>
#include <QFileInfo>

namespace scially {
    class CommandLineParseException : public QException {
    public:
        explicit CommandLineParseException(const QString& err) : err_(err) {
        }
        void raise() const override { throw* this; }
        CommandLineParseException* clone() const override { return new CommandLineParseException(*this); }
        QString error() const noexcept { return err_; }
    private:
        QString err_;
    };

    class CommandLineParse
    {
    public:
        CommandLineParse() {
            parser_.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
            parser_.setApplicationDescription("Convert OSGB, GDAL to Cesium 3DTiles");
            parser_.addHelpOption();
            parser_.addVersionOption();
        }
        void parse() {
            parser_.addOptions({
                {
                    QStringList() << "f" << "format",
                    "OSGB or GDAL(required)", "format"
                },
                {
                    QStringList() << "l" << "level",
                    "osgb max level", "level", "-1"
                },
                {
                    QStringList() << "F" << "field",
                    "height field name(required for gdal)", "field"
                },
                {
                    QStringList() << "L" << "layer",
                    "layer name(required when input isn't shapefile)", "layer"
                },
                {
                    QStringList() << "t" << "thread",
                    "thread number", "thread", "4"
                },
                {
                    QStringList() << "y" << "yUpAxis",
                    "y up axis", "yUpAxis"
                },
                {
                    QStringList() << "H" << "height",
                    "height offset(default value 0)", "height", "0"
                }
                });

            parser_.process(*qApp);
            thread_ = parser_.value("thread").toInt();
			
            if (!parser_.isSet("format"))
                throw CommandLineParseException("commandline format is requested");
            format_ = parser_.value("format").toUpper();

            if (format_ == "GDAL") {
                parseGDAL();
            }
            else if (format_ == "OSGB") {
                parseOSGB();
            }
            else
                throw CommandLineParseException("format must be GDAL or OSGB");
            parseIO();
        }
        void showHelp(int exitCode = -1) {
            parser_.showHelp();
        }
        QString format() const noexcept { return format_; }
        QString input() const noexcept { return input_; }
        QString output() const noexcept { return output_; }
        int level() const noexcept { return level_; }
        int thread() const noexcept { return thread_; }
        bool yUpAxis() const noexcept { return yUpAxis_; }
        double height() const noexcept { return height_; }
        QString field() const noexcept { return field_; }
        QString layer() const noexcept { return layer_; }

    private:
        void parseOSGB() {
            level_ = parser_.value("level") == "-1" ?
                std::numeric_limits<int>::max() : parser_.value("level").toInt();
            height_ = parser_.value("height") == "0" ? 0 : parser_.value("height").toDouble();
            yUpAxis_ = parser_.isSet("yUpAxis");
        }
        void parseGDAL() {
            if (!parser_.isSet("field"))
                throw CommandLineParseException("commandline field is requested");
            field_ = parser_.value("field");
        }

        void parseIO() {
            const QStringList args = parser_.positionalArguments();
            if (args.size() != 2) {
                throw CommandLineParseException("input and out must be set");
            }
            input_ = args[0];
            QFileInfo inputFile(input_);
            if (!inputFile.exists()) {
                throw CommandLineParseException("input file " + input_ + "not exist");
            }
            if (inputFile.fileName().indexOf(".shp") < 0 && !layer_.isEmpty()) {
                throw CommandLineParseException("layer name must be set");
            }
            if (inputFile.fileName().indexOf(".shp") >= 0 && layer_.isEmpty()) {
                layer_ = inputFile.baseName();
            }
            output_ = args[1];
        }

        QString format_;
        QString input_;
        QString output_;
        int level_;
        int thread_;
        bool yUpAxis_;
        double height_;
        QString field_;
        QString layer_;
        QCommandLineParser parser_;
    };
}