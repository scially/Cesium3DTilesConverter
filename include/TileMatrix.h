#pragma once


#include <QObject>
#include <BaseObject.h>
#include <QString>
#include <QGenericMatrix>

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


}