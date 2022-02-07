#include <TileMatrix.h>
#include <TilesParseException.h>

#include <osg/Math>
#include <osg/Vec3d>
#include <osg/Matrixd>
namespace gzpi {

    QJsonValue TileMatrix::write() {
        QJsonArray array;
        array.append(_mat[0][0]);  array.append(_mat[1][0]);  array.append(_mat[2][0]);  array.append(_mat[3][0]);
        array.append(_mat[0][1]);  array.append(_mat[1][1]);  array.append(_mat[2][1]);  array.append(_mat[3][1]);
        array.append(_mat[0][2]);  array.append(_mat[1][2]);  array.append(_mat[2][2]);  array.append(_mat[3][2]);
        array.append(_mat[0][3]);  array.append(_mat[1][3]);  array.append(_mat[2][3]);  array.append(_mat[3][3]);

        return array;
    }

    void TileMatrix::read(const QJsonValue& object) {
        if(!object.isArray())
            return;

        _mat[0][0] = object[0].toDouble();  _mat[1][0] = object[1].toDouble();  _mat[2][0] = object[2].toDouble();  _mat[3][0]= object[3].toDouble();
        _mat[0][1] = object[4].toDouble();  _mat[1][1] = object[5].toDouble();  _mat[2][1] = object[6].toDouble();  _mat[3][1]= object[7].toDouble();
        _mat[0][2] = object[8].toDouble();  _mat[1][2] = object[9].toDouble();  _mat[2][2] = object[10].toDouble(); _mat[3][2] = object[11].toDouble();
        _mat[0][3] = object[12].toDouble(); _mat[1][3] = object[13].toDouble(); _mat[2][3] = object[14].toDouble(); _mat[3][3] = object[15].toDouble();
    }

    TileMatrix TileMatrix::fromXYZ(double lon, double lat, double minHeight) {
        double lonr = osg::DegreesToRadians(lon);
        double latr = osg::DegreesToRadians(lat);

        constexpr double ellipsodA = 40680631590769;
        constexpr double ellipsodB = 40680631590769;
        constexpr double ellipsodC = 40408299984661.4;

        double xn = std::cos(lonr) * std::cos(latr);
        double yn = std::sin(lonr) * std::cos(latr);
        double zn = std::sin(latr);

        double x0 = ellipsodA * xn;
        double y0 = ellipsodB * yn;
        double z0 = ellipsodC * zn;
        double gamma = std::sqrt(xn * x0 + yn * y0 + zn * z0);
        double px = x0 / gamma;
        double py = y0 / gamma;
        double pz = z0 / gamma;

        double dx = xn * minHeight;
        double dy = yn * minHeight;
        double dz = zn * minHeight;

        osg::Vec3d eastVec(-y0, x0, 0);
        osg::Vec3d northVec(
            (y0 * eastVec[2] - eastVec[1] * z0),
            (z0 * eastVec[0] - eastVec[2] * x0),
            (x0 * eastVec[1] - eastVec[0] * y0));

        osg::Vec3d eastUnit =  eastVec  / eastVec.length();
        osg::Vec3d northUnit = northVec / northVec.length();
        
        return TileMatrix(
            eastUnit[0], northUnit[0], xn, px + dx,
            eastUnit[1], northUnit[1], yn, py + dy,
            eastUnit[2], northUnit[2], zn, pz + dz,
            0,           0,            0,  1);
    }

}
