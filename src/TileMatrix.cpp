#include <TileMatrix.h>
#include <TilesParseException.h>

namespace gzpi {

    QJsonValue TileMatrix::write() {
        QJsonArray array;
        array.append((*this)(0, 0));  array.append((*this)(1, 0));  array.append((*this)(2, 0));  array.append((*this)(3, 0));
        array.append((*this)(0, 1));  array.append((*this)(1, 1));  array.append((*this)(2, 1));  array.append((*this)(3, 1));
        array.append((*this)(0, 2));  array.append((*this)(1, 2));  array.append((*this)(2, 2));  array.append((*this)(3, 2));
        array.append((*this)(0, 3));  array.append((*this)(1, 3));  array.append((*this)(2, 3));  array.append((*this)(3, 3));

        return array;
    }

    void TileMatrix::read(const QJsonValue& object) {
        if (!required(object, QJsonValue::Array))
            throw TilesParseException("transform is required");

        (*this)(0, 0) = object[0].toDouble();  (*this)(1, 0) = object[1].toDouble();  (*this)(2, 0) = object[2].toDouble();  (*this)(3, 0) = object[3].toDouble();
        (*this)(0, 1) = object[4].toDouble();  (*this)(1, 1) = object[5].toDouble();  (*this)(2, 1) = object[6].toDouble();  (*this)(3, 1) = object[7].toDouble();
        (*this)(0, 2) = object[8].toDouble();  (*this)(1, 2) = object[9].toDouble();  (*this)(2, 2) = object[10].toDouble();  (*this)(3, 2) = object[11].toDouble();
        (*this)(0, 3) = object[12].toDouble();  (*this)(1, 3) = object[13].toDouble();  (*this)(2, 3) = object[14].toDouble();  (*this)(3, 3) = object[15].toDouble();
    }

}