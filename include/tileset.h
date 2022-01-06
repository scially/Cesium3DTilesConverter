#pragma once

#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>

#include <cmath>
#include <algorithm>
#include <array>
#include <optional>

#include <osg/Matrixd>

#include <QSharedPointer>
#include <QVector>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QHash>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

typedef std::array<double, 12> Box;
typedef std::array<double, 6> Range; // minx, miny, min_height, maxx, maxy, max_height


class BaseTile {
public:
    using BaseTilePtr = QSharedPointer<BaseTile>;
    virtual QJsonValue toJson() = 0;
    virtual QString typeName() = 0;
};

class TileBox : public BaseTile {
public:
    double maxX;
    double maxY;
    double maxZ;
    
    double minX;
    double minY;
    double minZ;

    void extend(double ratio) {
        ratio /= 2;
        double x = maxX - minX;
        double y = maxY - minY;
        double z = maxZ - minZ;
        maxX += x * ratio;
        maxY += y * ratio;
        maxZ += z * ratio;

        minX -= x * ratio;
        minY -= y * ratio;
        minZ -= z * ratio;
    }
};

/// <summary>
/// The boundingVolume.box property is an array of 12 numbers 
/// that define an oriented bounding box in a right-handed 3-axis (x, y, z) Cartesian coordinate system 
/// where the z-axis is up. 
/// The first three elements define the x, y, and z values for the center of the box. 
/// The next three elements (with indices 3, 4, and 5) define the x-axis direction and half-length. 
/// The next three elements (indices 6, 7, and 8) define the y-axis direction and half-length. 
/// The last three elements (indices 9, 10, and 11) define the z-axis direction and half-length.
/// </summary>
class TileBox : public BaseTile {
    double centerX;
    double centerY;
    double centerZ;

    double directionX0;
    double directionX1;
    double halfXLength;

    double directionY0;
    double directionY1;
    double halfYLength;
    
    double directionZ0;
    double directionZ1;
    double halfZLength;

    virtual QJsonValue toJson() override {
        QJsonArray arr = {centerX, centerY, centerZ,
                          directionX0, directionX1, halfXLength,
                          directionY0, directionY1, halfYLength,
                          directionZ0, directionZ1, halfZLength};
        
        return arr;
    }

    virtual QString typeName() override {
        return "box";
    }
};

/// <summary>
/// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
/// with latitude, longitude, and height coordinates with the order 
/// [west, south, east, north, minimum height, maximum height]. 
/// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
/// Heights are in meters above (or below) the WGS 84 ellipsoid.
/// </summary>
class TileRegion : public BaseTile {
public:
    double west;
    double south;
    double east;
    double north;
    double minHeight;
    double maxHeight;

    virtual QJsonValue toJson() override {
        QJsonArray arr = {west, south, east, north, minHeight, maxHeight};
        return arr;
    }

    virtual QString typeName() override {
        return "region";
    }
    
};

/// <summary>
/// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
/// The first three elements define the x, y, and z values 
/// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
/// The last element (with index 3) defines the radius in meters.
/// </summary>
class TileSphere : public BaseTile {
public:
    double centerX;
    double centerY;
    double centerZ;
    double radius;
    
    virtual QJsonValue toJson() {
        QJsonArray arr = { centerX, centerY, centerZ, radius };
        return arr;
    }

    virtual QString typeName() override{
        return "sphere";
    }
};


class TileMatrix : public BaseTile{
public:
    TileMatrix(std::initializer_list<double> args) {
        assert(args.size() == 16);
        std::copy(args.begin(), args.end(), mData.begin());
    }

    TileMatrix(double a00, double a01, double a02, double a03,
        double a10, double a11, double a12, double a13,
        double a20, double a21, double a22, double a23,
        double a30, double a31, double a32, double a33) {
        mData = { a00, a01, a02, a03,
                  a10, a11, a12, a13,
                  a20, a21, a22, a23,
                  a30, a31, a32, a33 };
    }

    double& operator()(int i, int j) {
        return mData[i * 4 + j];
    }

    double operator()(int i, int j) const {
        return mData[i * 4 + j];
    }

    
    virtual QJsonValue toJson() override {
        QJsonArray arr;
        // 列主序
        for(int j = 0; j < 4; j++)
            for (int i = 0; i < 4; i++) {
                arr.append(arr[i * 4 + j]);
            }
        return arr;
    }

    virtual QString typeName() override {
        return "transform";
    }
private:
    std::array<double, 16> mData;
};

struct Transform
{
    double radian_x;
    double radian_y;
    double min_height;
};

struct TileBox
{
    std::array<double, 3> max = { 0,0,0 };
    std::array<double, 3> min = { 0,0,0};

    void extend(double ratio) {
        ratio /= 2;
        double x = max[0] - min[0];
        double y = max[1] - min[1];
        double z = max[2] - min[2];
        max[0] += x * ratio;
        max[1] += y * ratio;
        max[2] += z * ratio;

        min[0] -= x * ratio;
        min[1] -= y * ratio;
        min[2] -= z * ratio;
    }
};

template<typename RangeMode, typename Type=double>
QJsonArray array2json(const RangeMode &range) {
    QJsonArray json_array;
    std::for_each(begin(range), end(range), [&json_array](Type t) {
        json_array.append(t);
    });
    return json_array;
}


Matrix transfrom_xyz(double radian_x, double radian_y, double height_min);

template <typename RangeMode>
bool write_tileset_box(
    const Matrix& matrix, const RangeMode& box, double geometricError,
    const QString& b3dm_file, const QString& json_file) {
   
    QJsonObject tileset_json;
    QJsonObject asset_json;
    asset_json["version"] = 0.0;
    asset_json["gltfUpAxis"] = "Y";
    tileset_json["asset"] = asset_json;

    tileset_json["geometricError"] = geometricError;
    QJsonObject root_json;


    root_json["transform"] = array2json(matrix);
    root_json["boundingVolume"] = array2json(box);
    root_json["geometricError"] = geometricError;
    root_json["refine"] = "REPLACE";
    QJsonObject uri_json;
    uri_json["uri"] = b3dm_file;
    root_json["content"] = uri_json;

    tileset_json["root"] = root_json;
    return write_file(json_file, QJsonDocument(tileset_json).toJson());
}

template <typename RangeMode>
bool write_tileset_box(
    Transform* trans, const RangeMode& box, double geometricError,
    const QString& b3dm_file, const QString& json_file) {
    Matrix matrix;
    if (trans != nullptr)
        matrix = transfrom_xyz(trans->radian_x, trans->radian_y, trans->min_height);
    else  // default transform matrix
        matrix = {
        1,  0,  0,  0,
        0,  1,  0,  0
        0,  0,  1,  0
        0,  0,  0,  1};
    return write_tileset_box(matrix, box, geometricError, b3dm_file, json_file);
}


/**
根据经纬度，生成tileset
*/
bool write_tileset(
    double radian_x, double radian_y,
    double tile_w, double tile_h,
    double height_min, double height_max,
    double geometricError,
    const QString &filename, const QString &full_path);


/// <summary>
/// 根据Tile生成JSON
/// </summary>
template <typename Range>
QJsonObject make_tile_json(double geometricError, const Range& range, const QJsonArray &children) {
   
    QJsonObject root_json;

    root_json["geometricError"] = geometricError;

    QJsonObject root_json;
    root_json["transform"] = array2json(matrix);
    
    QJsonObject box;
    box["box"] = array2json(range);

    root_json["boundingVolume"] = box;
    root_json["children"] = children;

    return root_json;
}

/// <summary>
/// 根据Tile生成JSON
/// </summary>
template <typename Range>
QJsonObject make_tile_json(double geometricError, const Matrix &matrix, const Range& range, 
    std::optional<QString> uri,
    std::optional<QJsonArray> children) {

    QJsonObject root_json;

    root_json["geometricError"] = geometricError;

    root_json["transform"] = array2json(matrix);

    QJsonObject box;
    box["box"] = array2json(range);

    root_json["boundingVolume"] = box;

    QJsonObject content;
    if (uri.has_value()) {
        content["uri"] = uri.value();
        root_json["content"] = content;
    }
    
    if(children.has_value())
        root_json["children"] = children.value();
    return root_json;
}

QJsonObject make_tiles_json(double geometricError, const QJsonObject& root);
