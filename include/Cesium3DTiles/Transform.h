#pragma once

#include <osg/Matrixd>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

namespace scially {
    /// <summary>
    /// The transform property is a 4x4 affine transformation matrix, stored in column-major order, that 
    /// transforms from the tile's local coordinate system to the parent tile's coordinate system or
    /// the tileset's coordinate system in the case of the root tile.
    /// </summary>
    struct Transform : public osg::Matrixd {
        static constexpr const char* TypeName = "transform";
        Transform(): Matrixd(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1) {}

        Transform(double m11, double m12, double m13, double m14,
            double m21, double m22, double m23, double m24,
            double m31, double m32, double m33, double m34,
            double m41, double m42, double m43, double m44)
            : Matrixd(
                m11, m12, m13, m14,
                m21, m22, m23, m24,
                m31, m32, m33, m34,
                m41, m42, m43, m44){}

        QJsonArray write() const;
        void read(const QJsonArray& object);
        static Transform fromXYZ(double lon, double lat, double height);
    };


}
