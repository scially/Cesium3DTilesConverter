#pragma once

#include <QCoreApplication>

#include <string>

#include <gdal_frmts.h>
#include <ogrsf_frmts.h>

namespace scially {
    class GDALRegister {
    public:
        GDALRegister() {
            CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
            std::string basePath = qApp->applicationDirPath().toStdString();
            gdalData = basePath + "/gdal_data";
            projData = basePath + "/proj_data";
            init();
        }
    private:
        void init() const {
            CPLSetConfigOption("GDAL_DATA", gdalData.c_str());
            const char* const projLibPath[] = { projData.c_str(), nullptr };
            OSRSetPROJSearchPaths(projLibPath);
            GDALAllRegister();
        }

        std::string gdalData;
        std::string projData;
    };
}
