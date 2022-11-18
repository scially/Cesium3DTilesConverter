#include <Config.h>
#include <OSGBConvertJob.h>
#include <ShpConvertJob.h>
#include <GDALWrapper.h>
#include <CommandLineParse.h>

#include <QCoreApplication>
#include <QTime>

int main(int argc, char** argv){
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(PROJECT_VERSION);
    app.setApplicationName(PROJECT_NAME);

    scially::GDALDriverWrapper init;
    scially::CommandLineParse cmd;
    try {
        cmd.parse();
    }
    catch (scially::CommandLineParseException& e) {
        qCritical() << e.error();
        cmd.showHelp();
    }
    
    QTime beginTime = QTime::currentTime();
    if (cmd.format() == "OSGB") {
        scially::OSGBConvertJob osgbConvert(cmd.input(), cmd.output());
        osgbConvert.setYUpAxis(cmd.yUpAxis());
        osgbConvert.setMaxLevel(cmd.thread());
        osgbConvert.setHeight(cmd.height());
        osgbConvert.setMaxLevel(cmd.level());
        osgbConvert.run();
    }
	
    if (cmd.format() == "GDAL") {
        scially::ShpConvertJob shpConvert(cmd.input(), cmd.layer(), 
            cmd.output(), cmd.field());
        shpConvert.run();
    }
    qInfo() << "finish convert in " << beginTime.secsTo(QTime::currentTime()) << "s";
    
    return 0;
}
