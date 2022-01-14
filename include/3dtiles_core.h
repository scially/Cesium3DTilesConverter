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

namespace gzpi {



      


    

  


        /// <summary>
        /// The transform property is a 4x4 affine transformation matrix, stored in column-major order, that 
        /// transforms from the tile's local coordinate system to the parent tile's coordinate system¡ªor 
        /// the tileset's coordinate system in the case of the root tile.
        /// </summary>
        class TileMatrix : public BaseObject, public QGenericMatrix<4, 4, double> {
        public:
            using TileMatrixPtr = QSharedPointer<TileMatrix>;

            TileMatrix(double m11, double m12, double m13, double m14,
                double m21, double m22, double m23, double m24,
                double m31, double m32, double m33, double m34,
                double m41, double m42, double m43, double m44) {
                (*this)(0, 0) = m11;  (*this)(0, 1) = m11;  (*this)(0, 2) = m11;  (*this)(0, 3) = m11;
                (*this)(1, 0) = m11;  (*this)(1, 1) = m11;  (*this)(1, 2) = m11;  (*this)(1, 3) = m11;
                (*this)(2, 0) = m11;  (*this)(2, 1) = m11;  (*this)(2, 2) = m11;  (*this)(2, 3) = m11;
                (*this)(3, 0) = m11;  (*this)(3, 1) = m11;  (*this)(3, 2) = m11;  (*this)(3, 3) = m11;
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
        class Refine : public QObject, public BaseObject {
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
            Refine::RefinePtr                 refine;
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

}
