#include <cmath>
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>

#include "utils.h"

double degree2rad(double val) {
    return val * (4 * std::atan(1)) / 180.0;
}

double lati_to_meter(double diff) {
    return diff / 0.000000157891;
}

double longti_to_meter(double diff, double lati) {
    return diff / 0.000000156785 * std::cos(lati);
}

double meter_to_lati(double m) {
    return m * 0.000000157891;
}

double meter_to_longti(double m, double lati) {
    return m * 0.000000156785 / std::cos(lati);
}


bool epsg_convert(int insrs, double& x, double& y) {
    OGRSpatialReference sourceSrs;
    sourceSrs.importFromEPSG(insrs);
    OGRSpatialReference targetSrs;
    targetSrs.importFromEPSG(4326);

    OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&sourceSrs, &targetSrs);
    int r = transform->Transform(1, &x, &y);
    delete transform;
    return r;
}
bool wkt_convert(const QString& inwkt, double& x, double& y) {
    OGRSpatialReference sourceSrs;
    sourceSrs.importFromWkt(inwkt.toStdString().c_str());
    OGRSpatialReference targetSrs;
    targetSrs.importFromEPSG(4326);

    OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&sourceSrs, &targetSrs);
    int r = transform->Transform(1, &x, &y);
    delete transform;
    return r;
}

bool create_dir(const QString& path) {
    QDir dir(path);
    if (dir.exists()) 
        return true;
    else 
        return dir.mkpath(path);

}

QString get_parent_name(const QString& file_path) {
    QDir dir(file_path);
    dir.cdUp();
    return dir.absolutePath();
}

QString get_file_name(const QString& file_path) {
    QDir dir(file_path);
    return dir.dirName();
}

bool write_file(const QString& out_file, const QByteArray &bytes) {
    QFile file(out_file);

    if(!file.open(QFile::ReadWrite | QFile::Truncate))
        return false;
   
    return file.write(bytes);
}