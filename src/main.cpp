#include <OSGBConvertJob.h>
#include <TilesConvertException.h>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTime>

int main(int argc, char** argv){
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Cesium3DTilesConverter");
    QCoreApplication::setApplicationVersion("0.1");
    QTime beginTime = QTime::currentTime();
    QCommandLineParser parser;
    parser.setApplicationDescription("Convert osgb,shp to Cesium 3dtiles");
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption inputOption("input", "input path", "input");
    parser.addOption(inputOption);
    const QCommandLineOption outputOption("output", "output path", "output");
    parser.addOption(outputOption);
    const QCommandLineOption formatOption("format", "<OSGB,SHAPE>", "format");
    parser.addOption(formatOption);
    const QCommandLineOption maxLvlOption("level", "max level", "level", "-1");
    parser.addOption(maxLvlOption);
    const QCommandLineOption heightOption("offset", "height offset", "offset", "0");
    parser.addOption(heightOption);
    const QCommandLineOption fieldOption("field", "height field name", "filed");
    parser.addOption(fieldOption);
    const QCommandLineOption threadOption("thread", "thread count", "thread", "4");
    parser.addOption(threadOption);

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
        gzpi::OSGBConvertJob osgbConvert(input, output);
        osgbConvert.setMaxLevel(thread);
        osgbConvert.setHeight(height);
        osgbConvert.setMaxLevel(maxLevel);
        try{
            osgbConvert.run();
        }catch(gzpi::TilesConvertException &e){
            qCritical() << e.what();
            app.exit(1);
        }catch(...){
            qCritical() << "Unknown error";
            app.exit(1);
        }
    }
    else if (format == "SHAPE"){
        if(!parser.isSet(fieldOption)){
            qCritical() << "Commandline field is requested";
            return 1;
        }
        const QString fieldName = parser.value(fieldOption);
    }
    else{
        qCritical() << "Only support osgb and shape format";
        return 1;
    }
    qInfo() << "Finish convert in " << beginTime.secsTo( QTime::currentTime()) << "s";
    return 0;
}
