#pragma once

#include <GDALWrapper.h>
#include <QuadTree.h>
#include <GeometryMesh.h>
#include <TilesConvertException.h>

#include <QByteArray>
#include <QString>
#include <QStringView>

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
