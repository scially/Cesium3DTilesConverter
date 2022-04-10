#include <OSGBConvertJob.h>
#include <ShpConvertJob.h>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTime>

int main(int argc, char** argv){
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Cesium3DTilesConverter");
    QCoreApplication::setApplicationVersion("1.2");
    QTime beginTime = QTime::currentTime();
    QCommandLineParser parser;
    parser.setApplicationDescription("Convert OSGB, GDAL to Cesium 3DTiles");
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption inputOption("input", "input path", "input");
    parser.addOption(inputOption);
    const QCommandLineOption outputOption("output", "output path", "output");
    parser.addOption(outputOption);
    const QCommandLineOption formatOption("format", "<OSGB,GDAL>", "format");
    parser.addOption(formatOption);
    const QCommandLineOption maxLvlOption("level", "max level", "level", "-1");
    parser.addOption(maxLvlOption);
    const QCommandLineOption heightOption("offset", "height offset", "offset", "0");
    parser.addOption(heightOption);
    const QCommandLineOption fieldOption("field", "height field name", "filed");
    parser.addOption(fieldOption);
    const QCommandLineOption layerOption("layer", "layer name", "layer");
    parser.addOption(layerOption);
    const QCommandLineOption threadOption("thread", "thread count", "thread", "4");
    parser.addOption(threadOption);
    const QCommandLineOption yUpAxis("yUpAxis", "y up axis", "yUpAxis");
    parser.addOption(yUpAxis);

    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.process(app);

    if(!parser.isSet(formatOption)){
        qCritical() << "Commandline format is requested";
        return 1;
    }
    if(!parser.isSet(inputOption)){
        qCritical() << "Commandline input is requested";
        return 1;
    }
    if(!parser.isSet(outputOption)){
        qCritical() << "Commandline output is requested";
        return 1;
    }
    const int thread = parser.value(threadOption).toInt();
    const QString input = parser.value(inputOption);
    const QString output= parser.value(outputOption);
    const QString format = parser.value(formatOption).toUpper();
    if(format == "OSGB"){
        const int maxLevel = parser.value(maxLvlOption) == "-1" ? std::numeric_limits<int>::max(): parser.value(maxLvlOption).toInt();
        const double height = parser.value(heightOption) == "0" ? 0 : parser.value(heightOption).toDouble();
        scially::OSGBConvertJob osgbConvert(input, output);
        if(parser.isSet(yUpAxis))
            osgbConvert.setYUpAxis(true);

        osgbConvert.setMaxLevel(thread);
        osgbConvert.setHeight(height);
        osgbConvert.setMaxLevel(maxLevel);
        osgbConvert.run();
    }
    else if (format == "GDAL"){
        if(!parser.isSet(fieldOption)){
            qCritical() << "Commandline field is requested";
            return 1;
        }
        if (!parser.isSet(layerOption)) {
            qCritical() << "Commandline layer is requested";
            return 1;
        }
        const QString fieldName = parser.value(fieldOption);
        const QString layerName = parser.value(layerOption);

        scially::ShpConvertJob shpConvert(input, layerName, output, fieldName);
        shpConvert.run();
    }
    else{
        qCritical() << "Only support OSGB and GDAL format";
        return 1;
    }
    qInfo() << "Finish convert in " << beginTime.secsTo( QTime::currentTime()) << "s";
    return 0;
}
