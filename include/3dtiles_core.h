#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <initializer_list>

#include <QObject>
#include <QException>
#include <QMetaEnum>
#include <QSharedPointer>
#include <QVector>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QGenericMatrix>
#include <QHash>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class ThreedTilesParseException : public QException {
public:
    ThreedTilesParseException(const QString& err) : err(err) {}
    virtual const char* what() const override{
        return err.toStdString().c_str();
    }
private:
    QString err;
};

class BaseObject {
public:
    using BasePtr = QSharedPointer<BaseObject>;

    virtual QJsonValue write() = 0;
    virtual void read(const QJsonValue& object) = 0;
    virtual QString typeName() = 0;
protected:
    static bool required(const QJsonValue& object, QJsonValue::Type type) {
        return object.type() == type;
    }
};

class BoundingVolume : public BaseObject {
public:
    using BoundingVolumePtr = QSharedPointer<BoundingVolume>;
    virtual QString typeName() override {
        return "boundingVolume";
    }

    static BoundingVolumePtr create(const QJsonValue& object);
};

class AssetProperties : public BaseObject {
public:
    using AssetPropertiesPtr = QSharedPointer<AssetProperties>;
    
    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;

    virtual QString typeName() override {
        return "asset";
    }

    QMap<QString, QString> assets;
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
class BoundingVolumeBox : public BoundingVolume {
public:
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

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;
};

/// <summary>
/// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
/// with latitude, longitude, and height coordinates with the order 
/// [west, south, east, north, minimum height, maximum height]. 
/// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
/// Heights are in meters above (or below) the WGS 84 ellipsoid.
/// </summary>
class BoundingVolumeRegion : public BoundingVolume {

public:
    double west;
    double south;
    double east;
    double north;
    double minHeight;
    double maxHeight;

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;

};

/// <summary>
/// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
/// The first three elements define the x, y, and z values 
/// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
/// The last element (with index 3) defines the radius in meters.
/// </summary>
class BoundingVolumeSphere : public BoundingVolume {
public:
    double centerX;
    double centerY;
    double centerZ;
    double radius;

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;
};

/// <summary>
/// The transform property is a 4x4 affine transformation matrix, stored in column-major order, that 
/// transforms from the tile's local coordinate system to the parent tile's coordinate system¡ªor 
/// the tileset's coordinate system in the case of the root tile.
/// </summary>
class TileMatrix : public BaseObject, public QGenericMatrix<4,4,double>{
public:
    using TileMatrixPtr = QSharedPointer<TileMatrix>;

    TileMatrix(double m11, double m12, double m13, double m14, 
        double m21, double m22, double m23, double m24, 
        double m31, double m32, double m33, double m34,
        double m41, double m42, double m43, double m44) {
        (*this)(0,0) = m11;  (*this)(0,1) = m11;  (*this)(0,2) = m11;  (*this)(0,3) = m11;
        (*this)(1,0) = m11;  (*this)(1,1) = m11;  (*this)(1,2) = m11;  (*this)(1,3) = m11;
        (*this)(2,0) = m11;  (*this)(2,1) = m11;  (*this)(2,2) = m11;  (*this)(2,3) = m11;
        (*this)(3,0) = m11;  (*this)(3,1) = m11;  (*this)(3,2) = m11;  (*this)(3,3) = m11;
    }

    virtual QJsonValue write() override;

    virtual void read(const QJsonValue& object) override;

    virtual QString typeName() override {
        return "transform";
    }

    TileMatrix();
};


/// <summary>
/// Refinement determines the process by which a lower resolution parent tile renders 
/// when its higher resolution children are selected to be rendered. 
/// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
/// </summary>
class Refine: public QObject, public BaseObject {
   Q_OBJECT
   
public:
    using RefinePtr = QSharedPointer<Refine>;
    enum RefineType {
        ADD,
        REPLACE
    };
    Q_ENUM(RefineType);

    virtual void read(const QJsonValue& object) override;
    virtual QJsonValue write() override;
    virtual QString typeName() override {
        return "refine";
    }
    QString toString() const;

    Refine(RefineType type = ADD) : type_(type) {}

    RefineType type_;
private:
    QMetaEnum typeMeta = QMetaEnum::fromType<RefineType>();
};


/// <summary>
/// Metadata about the tile's content and a link to the content.
/// -------------------------------------------------------------------------------------------------
/// Name           |Description                                            |Type            |Required
/// -------------------------------------------------------------------------------------------------
/// boundingVolume |A bounding volume that encloses a tile or its content. |AssetProperties |
/// -------------------------------------------------------------------------------------------------
/// uri            |A uri that points to the tile's content.               |string          |Required
/// -------------------------------------------------------------------------------------------------
/// </summary>
class ContentTile : public BaseObject {
public:
    using ContentTilePtr = QSharedPointer<ContentTile>;

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;
    virtual QString typeName() override {
        return "content";
    }

    ContentTile();

    BoundingVolume::BoundingVolumePtr boundingVolume;
    QString uri;
};

/// <summary>
/// A tile in a 3D Tiles tileset.
/// --------------------------------------------------------------------------------------------------------
/// Name           |Description                                                  |Type            |Required
/// --------------------------------------------------------------------------------------------------------
/// boundingVolume |A bounding volume that encloses a tile or its content.       |BoundingVolume  |Required
/// --------------------------------------------------------------------------------------------------------
/// geometricError |A uri that points to the tile's content.                     |double          |Required
/// --------------------------------------------------------------------------------------------------------
/// refine         |                                                             |Refine          |
/// --------------------------------------------------------------------------------------------------------
/// transform      |default: [1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]                   |TileMatrix      |
/// --------------------------------------------------------------------------------------------------------
/// content        |Metadata about the tile's content and a link to the content. |ContentTile     |
/// --------------------------------------------------------------------------------------------------------
/// children       |An array of objects that define child tiles                  |RootTile[]      |
/// --------------------------------------------------------------------------------------------------------
/// </summary>
class RootTile : public BaseObject {
public:
    using RootTilePtr = QSharedPointer<RootTile>;

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;
    virtual QString typeName() override {
        return "root";
    }

    RootTile();
    BoundingVolume::BoundingVolumePtr boundingVolume;
    TileMatrix::TileMatrixPtr         transform;
    double                            geometricError;
    Refine::RefinePtr                 refine ;
    ContentTile::ContentTilePtr       content;
    QVector<RootTilePtr>              children;
};


/// <summary>
/// A 3D Tiles tileset.
/// -----------------------------------------------------------------------------
/// Name           |Description                        |Type            |Required
/// -----------------------------------------------------------------------------
/// asset          |Metadata about the entire tileset. |AssetProperties |Required
/// -----------------------------------------------------------------------------
/// geometricError |                                   |double          |Required
/// -----------------------------------------------------------------------------
/// root           |A tile in a 3D Tiles tileset.      |ContentTile     |Required
/// -----------------------------------------------------------------------------
/// </summary>
class BaseTile : public BaseObject {
public:
    using BaseTilePtr = QSharedPointer<BaseTile>;

    virtual QJsonValue write() override;
    virtual void read(const QJsonValue& object) override;

    virtual QString typeName() override {
        return "";
    }

    BaseTile();
    AssetProperties::AssetPropertiesPtr asset;
    double geometricError;
    RootTile::RootTilePtr root;
};
