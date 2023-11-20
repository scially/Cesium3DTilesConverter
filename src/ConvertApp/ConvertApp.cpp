#include <OSGConvert/OSGFolder.h>
#include <ConvertApp/CLIParse.h>
#include <Commons/GDALRegister.h>
#include <Commons/Version.h>

#include <QTime>
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(SCIALLY_PROJECT_VERSION);
    app.setApplicationName(SCIALLY_PROJECT_NAME);

    // register gdal and proj data, and register gdal driver
    const scially::GDALRegister __gdal__;

    scially::CLIParse parser;
    if (!parser.parse())
        return -1;

    QTime beginTime = QTime::currentTime();
    if (parser.format == scially::CLIParse::Vector)
    {
        return -1;
    }
    else if (parser.format == scially::CLIParse::OSGB)
    {
        scially::OSGConvertOption options;
        options.input = parser.input;
        options.output = "file:" + parser.output;
        scially::OSGFolder dataFolder;
        if (!dataFolder.load(options))
            return -1;
        
        if(!dataFolder.toB3DMPerTile(options))
            return -1;
    }

    qInfo() << "finish convert in" << beginTime.secsTo(QTime::currentTime()) << "s";
    return 0;
}