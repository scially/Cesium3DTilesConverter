#include <OSGBConvertJob.h>
#include <ShpConvertJob.h>

#include <CLI11.hpp>
#include <spdlog/spdlog.h>
#include <QTime>
#include <string>

int main(int argc, char** argv){
    CLI::App app{"Cesium3DTilesConverter"};

    std::string format;
    std::string input, output;
    double offset;
    int level;
    std::string field, layer;
    int thread;
    bool yUpAxis;

    app.add_option("-f,--format", format, "format of the input data")
        ->required();
    app.add_option("-i,--input", input,  "input file or directory")
        ->required();
    app.add_option("-o,--output", input, "output file or directory")
        ->required();
    app.add_option("-l,--level", level, "level of detail")
        ->default_val(-1);
    app.add_option("--offset", offset, "offset of the tileset height")
        ->default_val(0);
    auto fieldOption = app.add_option("--field", field, "field of the height");
    auto layerOption = app.add_option("--layer", layer, "layer of the gdal data");
    app.add_option("-t,--thread", thread, "number of threads")
        ->default_val(0);
    app.add_option("--yUpAxis", yUpAxis, "use y as up axis")
        ->default_val(false);
    CLI11_PARSE(app, argc, argv);
    
    QTime beginTime = QTime::currentTime();
    
    if(format == "OSGB"){
        scially::OSGBConvertJob osgbConvert(input, output);
        osgbConvert.setYUpAxis(yUpAxis);
        osgbConvert.setMaxLevel(level);
        osgbConvert.setHeight(offset);
        osgbConvert.setMaxThread(thread);
        osgbConvert.run();
    }
    else if (format == "GDAL"){
        if(fieldOption->empty()){
            spdlog::error("field is required for GDAL format");
            return -1;
        }
        if(layerOption->empty()){
            spdlog::error("layer name is required for GDAL format");
            return -1;
        }
      
        scially::ShpConvertJob shpConvert(input, layer, output, field);
        shpConvert.run();
    }
    else{
        spdlog::error("only support OSGB and GDAL format");
        return 1;
    }
    spdlog::info("finish convert in {}s", beginTime.secsTo( QTime::currentTime()));
    return 0;
}
