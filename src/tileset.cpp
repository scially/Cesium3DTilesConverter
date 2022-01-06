#include "tileset.h"
#include "utils.h"

Matrix transfrom_xyz(double radian_x, double radian_y, double height_min){
    double ellipsod_a = 40680631590769;
    double ellipsod_b = 40680631590769;
    double ellipsod_c = 40408299984661.4;

    const double pi = std::acos(-1);
    double xn = std::cos(radian_x) * std::cos(radian_y);
    double yn = std::sin(radian_x) * std::cos(radian_y);
    double zn = std::sin(radian_y);

    double x0 = ellipsod_a * xn;
    double y0 = ellipsod_b * yn;
    double z0 = ellipsod_c * zn;
    double gamma = std::sqrt(xn*x0 + yn*y0 + zn*z0);
    double px = x0 / gamma;
    double py = y0 / gamma;
    double pz = z0 / gamma;
    
    double dx = xn * height_min;
    double dy = yn * height_min;
    double dz = zn * height_min;

    std::vector<double> east_mat = {-y0,x0,0};
    std::vector<double> north_mat = {
        (y0*east_mat[2] - east_mat[1]*z0),
        (z0*east_mat[0] - east_mat[2]*x0),
        (x0*east_mat[1] - east_mat[0]*y0)
    };
    double east_normal = std::sqrt(
        east_mat[0]*east_mat[0] + 
        east_mat[1]*east_mat[1] + 
        east_mat[2]*east_mat[2]
        );
    double north_normal = std::sqrt(
        north_mat[0]*north_mat[0] + 
        north_mat[1]*north_mat[1] + 
        north_mat[2]*north_mat[2]
        );

    return {
        east_mat[0] / east_normal,      east_mat[1] / east_normal,      east_mat[2] / east_normal,      0,
        north_mat[0] / north_normal,    north_mat[1] / north_normal,    north_mat[2] / north_normal,    0,
        xn,                             yn,                             zn,                             0,
        px + dx,                        py + dy,                        pz + dz,                        1
    };
}


bool write_tileset(
    double radian_x, double radian_y, 
    double tile_w, double tile_h, 
    double height_min, double height_max,
    double geometricError,
    const QString& filename, const QString& full_path) {
    
    double ellipsod_a = 40680631590769;
    double ellipsod_b = 40680631590769;
    double ellipsod_c = 40408299984661.4;

    const double pi = std::acos(-1);
    double xn = std::cos(radian_x) * std::cos(radian_y);
    double yn = std::sin(radian_x) * std::cos(radian_y);
    double zn = std::sin(radian_y);

    double x0 = ellipsod_a * xn;
    double y0 = ellipsod_b * yn;
    double z0 = ellipsod_c * zn;
    double gamma = std::sqrt(xn*x0 + yn*y0 + zn*z0);
    double px = x0 / gamma;
    double py = y0 / gamma;
    double pz = z0 / gamma;
    double dx = x0 * height_min;
    double dy = y0 * height_min;
    double dz = z0 * height_min;
    QVector<double> east_mat = {-y0,x0,0};
    QVector<double> north_mat = {
      (y0*east_mat[2] - east_mat[1]*z0),
      (z0*east_mat[0] - east_mat[2]*x0),
      (x0*east_mat[1] - east_mat[0]*y0)
    };
    double east_normal = std::sqrt(
      east_mat[0]*east_mat[0] + 
      east_mat[1]*east_mat[1] + 
      east_mat[2]*east_mat[2]
      );
    double north_normal = std::sqrt(
      north_mat[0]*north_mat[0] + 
      north_mat[1]*north_mat[1] + 
      north_mat[2]*north_mat[2]
      );

    Matrix matrix = {
      east_mat[0] / east_normal,    east_mat[1] / east_normal,      east_mat[2] / east_normal,      0,
      north_mat[0] / north_normal,  north_mat[1] / north_normal,    north_mat[2] / north_normal,    0,
      xn,                           yn,                             zn,                             0,
      px + dx,                      py + dy,                        pz + dz,                        1
    };

    Range region = {
      radian_x - meter_to_longti(tile_w / 2, radian_y),
      radian_y - meter_to_lati(tile_h / 2),
      radian_x + meter_to_longti(tile_w / 2, radian_y),
      radian_y + meter_to_lati(tile_h / 2),
      0,
      height_max
    };
    
    return write_tileset_box(matrix, region, geometricError, filename, full_path);
}

QJsonObject make_tiles_json(double geometricError, const QJsonObject& root) {

    QJsonObject tiles_json;

    QJsonObject asset_json;

    asset_json["version"] = "1.0";
    //asset_json["gltfUpAxis"] = "Z";
    tiles_json["asset"] = asset_json;
    
    tiles_json["geometricError"] = geometricError;
    tiles_json["root"] = root;
    return tiles_json;
}
