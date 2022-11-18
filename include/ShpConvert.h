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
            :fileName_(fileName), layerName_(layerName), heightField_(heightField)
        {}

        bool convertTiles(const QString& output);
        bool initQuadTree(QuadTree& root);
    private:
        bool getAllFeatures();

        OGRLayerWrapper layer_;
        OGREnvelope extent_;
        QMap<GIntBig, OGRFeatureWrapper> iFeature_;
        GDALDatasetWrapper ds_;
		
        QString fileName_;
        QString layerName_;
        QString heightField_;
    };
}
