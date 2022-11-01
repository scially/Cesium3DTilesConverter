#include <OSGBConvertJob.h>
#include <ShpConvertJob.h>
#include <GDALWrapper.h>
#include <CLI11.hpp>

#include <QTime>
#include <QCoreApplication>


int main(int argc, char** argv){
    QCoreApplication app(argc, argv);
    scially::GDALIniter initer;

    CLI::App cli{"Cesium3DTilesConverter"};
    std::string format;
    std::string input, output;
    double offset;
    int level;
    std::string field, layer;
    int thread;

    cli.add_option("-f,--format", format, "format of the input data")
        ->required();
    cli.add_option("-i,--input", input,  "input file or directory")
        ->check(CLI::ExistingPath)
        ->required();
    cli.add_option("-o,--output", output, "output file or directory")
        ->required();
    cli.add_option("-l,--level", level, "level of detail")
        ->default_val(22);
    cli.add_option("--offset", offset, "offset of the tileset height")
        ->default_val(0);
    auto fieldOption = cli.add_option("--field", field, "field of the height");
    auto layerOption = cli.add_option("--layer", layer, "layer of the gdal data");
    cli.add_option("-t,--thread", thread, "number of threads")
        ->default_val(QThread::idealThreadCount());
    
    try {                                                                                                              \
        cli.parse(argc, argv);                                                                                   \
    } catch(const CLI::ParseError &e) {                                                                                \
        app.exit(-1);                                                                              \
    }
    
    QTime beginTime = QTime::currentTime();
    
    if(format == "OSGB"){
        scially::OSGBConverter osgbConvert;
        osgbConvert.setInput(QString::fromLocal8Bit(input.c_str()));
        osgbConvert.setOutput(QString::fromLocal8Bit(output.c_str()));
        osgbConvert.setHeight(offset);
        osgbConvert.setMaxLevel(level);
        osgbConvert.setThreadCount(thread);
        osgbConvert.run();
    }
    else if (format == "GDAL"){
        if(fieldOption->empty()){
            spdlog::error("field is required for GDAL format");
            app.exit(-1);  
        }
        if(layerOption->empty()){
            spdlog::error("layer name is required for GDAL format");
            app.exit(-1);  
        }
      
        scially::ShpConvertJob shpConvert(input, layer, output, field);
        shpConvert.run();
    }
    else{
        spdlog::error("only support OSGB and GDAL format");
        app.exit(-1);  
    }
    spdlog::info("finish convert in {}s", beginTime.secsTo( QTime::currentTime()));
    return app.exec();
}
