#include <Commons/GDALRegister.h>
#include <Commons/Version.h>
#include <ConvertApp/CLIParse.h>
#include <OSGConvert/OSGFolder.h>
#include <OSGConvert/OSGVirtualFolder.h>

#include <QCoreApplication>
#include <QTime>

int main(int argc, char** argv) {
    qSetMessagePattern("[%{type} %{time yyyy-MM-dd h:mm:ss.zzz}] %{file}:%{line} %{message}");
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

        scially::OSGFolder folder(parser.input);
        if (!folder.load("file:" + parser.output))
            return -1;

        auto b3dms = folder.to3DTiles(0);
        if (b3dms.isEmpty())
            return -1;

        if (parser.mergeTop) {
            scially::OSGVirtualFolder vFolder(
                b3dms,
                folder.outSRS(),
                *folder.transform(),
                *folder.storage());

            const auto nodes = vFolder.to3DTiles(1);
            if (nodes.isEmpty())
                return -1;
        }
    }

    qInfo("finish convert in %d s", beginTime.secsTo(QTime::currentTime()));
    return 0;
}