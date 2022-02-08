#pragma once

#include <QByteArray>
#include <QString>
#include <QStringView>
#include <GDALWrapper.h>
#include <QuadTree.h>
#include <GeometryMesh.h>
#include <TilesConvertException.h>

namespace scially {
    class ShpConvert {
    public:
        ShpConvert(const QString &fileName, const QString &layerName, const QString &heightField)
            :fileName(fileName), layerName(layerName), heightField(heightField)
        {}

        void convertTiles(const QString& output);

    private:
        QString fileName;
        QString layerName;
        QString heightField;
    };
}
