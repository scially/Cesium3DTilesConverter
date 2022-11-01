#pragma once

#include <osg/Vec3d>
#include <osg/Matrixd>

#include <QJsonValue>
#include <QJsonObject>
#include <QSharedPointer>
#include <QString>
#include <QMap>
#include <optional>

namespace scially {
    class BaseObject {
    public:
        virtual QJsonValue write() = 0;
        virtual void read(const QJsonValue& object) = 0;
        virtual QString typeName() = 0;
        virtual ~BaseObject() {}
    protected:
        static bool required(const QJsonValue& object, QJsonValue::Type type) {
            return object.type() == type;
        }
    };

    class AssetProperties : public BaseObject {
    public:
        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;

        virtual QString typeName() override {
            return "asset";
        }
        virtual ~AssetProperties() {}
        QMap<QString, QString> assets;
    };

    /// <summary>
    /// The boundingVolume.region property is an array of six numbers that define the bounding geographic region 
    /// with latitude, longitude, and height coordinates with the order 
    /// [west, south, east, north, minimum height, maximum height]. 
    /// Latitudes and longitudes are in the WGS 84 datum as defined in EPSG 4979 and are in radians. 
    /// Heights are in meters above (or below) the WGS 84 ellipsoid.
    /// </summary>
    class BoundingVolumeRegion : public BaseObject {
    public:
        double west = std::numeric_limits<double>::max();
        double south = std::numeric_limits<double>::max();
        double east = std::numeric_limits<double>::min();
        double north = std::numeric_limits<double>::min();
        double minHeight = std::numeric_limits<double>::max();
        double maxHeight = std::numeric_limits<double>::min();

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "region";
        };

        osg::Vec3d getMax() const;
        osg::Vec3d getMin() const;

        void setMax(const osg::Vec3d& max);
        void setMin(const osg::Vec3d& min);
        void setMax(const osg::Vec3f& max);
        void setMin(const osg::Vec3f& min);

        void mergeMax(const osg::Vec3d& max);
        void mergeMin(const osg::Vec3d& min);
        void mergeMax(const osg::Vec3f& max);
        void mergeMin(const osg::Vec3f& min);

        double geometricError() const {
            double maxErr = std::max({north - south, maxHeight - minHeight, east - west});
            return maxErr / 20.0;
        }
        
         BoundingVolumeRegion merge(BoundingVolumeRegion bounding) const {
           BoundingVolumeRegion mergeRegion;
           mergeRegion.west = std::min(bounding.west, this->west);
           mergeRegion.east = std::max(bounding.east, this->east);
           mergeRegion.north = std::max(bounding.north, this->north);
           mergeRegion.south = std::min(bounding.south, this->south);
           mergeRegion.minHeight = std::min(bounding.minHeight, this->minHeight);
           mergeRegion.maxHeight = std::max(bounding.maxHeight, this->maxHeight);
           return mergeRegion;
        }

        BoundingVolumeRegion toRadin(double lon, double lat) const;
        
        static BoundingVolumeRegion fromCenterXY(double centerX, double centerY, double xLength, double yLength, double minHeight, double maxHeight);
        virtual ~BoundingVolumeRegion() {}
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
    class BoundingVolumeBox : public BaseObject {
    public:
        explicit BoundingVolumeBox(BoundingVolumeRegion r): region(r){}
        BoundingVolumeBox() = default;

        double centerX() {
            return (region.east + region.west) / 2;
        }
        double centerY() {
            return (region.north + region.south) / 2;
        }
        double centerZ() {
            return (region.minHeight + region.maxHeight) / 2;
        }

        double directionX0 () {
            return 0;
        }
        double directionX1 () {
            return 0;
        }
        double halfXLength () {
            return (region.east - region.west) / 2;
        }

        double directionY0() {
            return 0;
        }

        double directionY1() {
            return 0;
        }

        double halfYLength () {
            return (region.north - region.south) / 2;
        }

        double directionZ0 () {
            return 0;
        }

        double directionZ1() {
            return 0;
        }

        double halfZLength () {
            return (region.maxHeight - region.minHeight) / 2;
        }

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "box";
        }

        void setMax(const osg::Vec3d& max);
        void setMin(const osg::Vec3d& min);
        void setMax(const osg::Vec3f& max);
        void setMin(const osg::Vec3f& min);

        void mergeMax(const osg::Vec3d& max);
        void mergeMin(const osg::Vec3d& min);
        void mergeMax(const osg::Vec3f& max);
        void mergeMin(const osg::Vec3f& min);

        BoundingVolumeBox merge(BoundingVolumeBox bounding) const;

        double geometricError() const;
        osg::Vec3d getMax() const;
        osg::Vec3d getMin() const;

        virtual ~BoundingVolumeBox() {}
    private:
        BoundingVolumeRegion region;
    };

    /// <summary>
    /// The boundingVolume.sphere property is an array of four numbers that define a bounding sphere. 
    /// The first three elements define the x, y, and z values 
    /// for the center of the sphere in a right-handed 3-axis (x, y, z) Cartesian coordinate system where the z-axis is up. 
    /// The last element (with index 3) defines the radius in meters.
    /// </summary>
    class BoundingVolumeSphere : public BaseObject {
    public:
        double centerX = std::numeric_limits<double>::min();
        double centerY = std::numeric_limits<double>::min();
        double centerZ = std::numeric_limits<double>::min();
        double radius  = std::numeric_limits<double>::min();

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "sphere";
        }
        virtual ~BoundingVolumeSphere() {}
    };



    class BoundingVolume : public BaseObject {
    public:
        BoundingVolume() = default;
        explicit BoundingVolume(const BoundingVolumeBox &box): box(box){}
        explicit BoundingVolume(const BoundingVolumeRegion &region): region(region){}
        explicit BoundingVolume(const BoundingVolumeSphere &sphere): sphere(sphere){}

        virtual QString typeName() override {
            return "boundingVolume";
        }

        virtual QJsonValue write() override{
            if(box.has_value()){
                return box->write();
            }else if(region.has_value()){
                return region->write();
            }else if(sphere.has_value()){
                return sphere->write();
            }else{
                // TODO: BoundingVolume type must be box,region,sphere
                qCritical() << "BoundingVolume not set";
                return QJsonValue();
            }
        }

        bool hasValue() const {
            return box.has_value() || region.has_value() || sphere.has_value();
        }

        inline BoundingVolume& operator =(const BoundingVolumeBox &box) {
            this->box = box;
            return *this;
        }
        inline BoundingVolume& operator =(const BoundingVolumeRegion &region){
            this->region = region;
            return *this;
        }
        inline BoundingVolume& operator =(const BoundingVolumeSphere &sphere){
            this->sphere = sphere;
            return *this;
        }

        virtual void read(const QJsonValue& object) override{
            if (object.type() != QJsonValue::Object) {
                qCritical() << "value is not BoundingVolume\n";
                return;
            } 

            QJsonObject bounding = object.toObject();
            if(bounding.keys().size() != 1){
                qCritical() << "BoundingVolume type must specideifed \n";
                return;
            }
            QString boundingType = bounding.keys().at(0);
            if(boundingType == "box"){
                BoundingVolumeBox boxResult;
                boxResult.read(bounding[boundingType]);
                box = boxResult;
            }else if(boundingType == "region"){
                BoundingVolumeRegion regionResult;
                regionResult.read(bounding[boundingType]);
                region = regionResult;
            }else if(boundingType == "sphere"){
                BoundingVolumeSphere sphereResult;
                sphereResult.read(bounding[boundingType]);
                sphere = sphereResult;
            }else{
                qCritical() << "BoundingVolume type must be box,region,sphere\n";
            }

        }

        virtual ~BoundingVolume() {}

        std::optional<BoundingVolumeBox> box;
        std::optional<BoundingVolumeRegion> region;
        std::optional<BoundingVolumeSphere> sphere;
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
       
        ContentTile() {}
        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "content";
        }

        std::optional<BoundingVolume> boundingVolume;
        QString uri;
    };

    /// <summary>
    /// Refinement determines the process by which a lower resolution parent tile renders
    /// when its higher resolution children are selected to be rendered.
    /// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
    /// </summary>
    class Refine : public BaseObject {
    public:
        virtual void read(const QJsonValue& object) override;
        virtual QJsonValue write() override;
        virtual QString typeName() override {
            return "refine";
        }
        Refine& operator =(const QString &type) {
            this->type = type;
            return *this;
        }

        Refine() = default;
        QString type = "REPLACE";
    };

    /// <summary>
    /// The transform property is a 4x4 affine transformation matrix, stored in column-major order, that 
    /// transforms from the tile's local coordinate system to the parent tile's coordinate system or
    /// the tileset's coordinate system in the case of the root tile.
    /// </summary>
    class TileMatrix : public BaseObject, public osg::Matrixd {
    public:
        using TileMatrixPtr = QSharedPointer<TileMatrix>;

        TileMatrix(): Matrixd(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1) {}

        TileMatrix(double m11, double m12, double m13, double m14,
            double m21, double m22, double m23, double m24,
            double m31, double m32, double m33, double m34,
            double m41, double m42, double m43, double m44)
            : Matrixd(
                m11, m12, m13, m14,
                m21, m22, m23, m24,
                m31, m32, m33, m34,
                m41, m42, m43, m44){}

        virtual QJsonValue write() override;

        virtual void read(const QJsonValue& object) override;

        virtual QString typeName() override {
            return "transform";
        }

        static TileMatrix fromXYZ(double lon, double lat, double height);
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
        virtual ~RootTile() {}

        BoundingVolume boundingVolume;
        TileMatrix     transform;
        double         geometricError = 0;
        Refine         refine;
        std::optional<ContentTile>  content;
        QVector<RootTile>           children;
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
        BaseTile() {}
        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
    
        virtual QString typeName() override {
            return "";
        }
        virtual ~BaseTile() {}
        AssetProperties asset;
        double geometricError;
        RootTile root;
    };

    class Batched3DModel {
	public:
		// feature table and feature binary
		int batchLength = 0;
		QByteArray featureBinary; 
		// batch table and batch binary
		QVector<int> batchID;
		QVector<QString> names;
		QVector<double> heights;
		// glb buffer
		QByteArray glbBuffer;

		QByteArray write(bool withHeight = false) const;
	};
}

