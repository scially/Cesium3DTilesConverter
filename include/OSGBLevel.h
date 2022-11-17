#pragma once

#include <Cesium3DTiles/RootTile.h>
#include <Cesium3DTiles/BaseTile.h>
#include <OSGBRegion.h>
#include <OSGBuildState.h>
#include <OSGBPageLodVisitor.h>

#include <QFileInfo>

#include <limits>

namespace scially {
    class OSGLevel {
    public:
        static constexpr const char* OSGBEXTENSION = ".osgb";
        static constexpr const char* B3DMEXTENSION = ".b3dm";
        static constexpr int MAXLEVEL = 28;

        OSGLevel(const QString& absoluteLocation): nodeLocation_(absoluteLocation){
            QFileInfo osgFile(absoluteLocation);
            nodeName_ = osgFile.baseName();
            nodePath_ = osgFile.path();
        }

        /// <summary>
        /// get Tile_+154_+018_L22_0000320.osgb _L level
        /// </summary>
        int getLevelNumber() const;
        QString getTileName() const;
        
        bool convertTiles(BaseTile &tile, const QString& output, int maxLevel = std::numeric_limits<int>::max());
        bool convertTiles(RootTile &root, const QString& output);
        
        OSGBRegion region() const { return region_; }
        double geometricError() const { return geometricError_; }
        QString nodeName() const { return nodeName_; }
        QString nodePath() const { return nodePath_; }
        QVector<OSGLevel> subNodes() const { return subNodes_; }
        osg::Vec3d bBoxMax() const { return bBoxMax_; }
        osg::Vec3d bBoxMin() const { return bBoxMin_; }
        bool yUpAxis() const { return yUpAxis_; }
        void setYUpAxis(bool y) { yUpAxis_ = y; }

    protected:
        void createDir(const QString& output) const;
        bool getAllOSGBLevels(int maxLevel);
        void updateGeometryError(RootTile& root);

    private:
        double              geometricError_;
        OSGBRegion          region_;
        QString             nodeName_;
        QString             nodePath_;
        QString             nodeLocation_;
        QVector<OSGLevel>   subNodes_;
        osg::Vec3d          bBoxMax_;
        osg::Vec3d          bBoxMin_;
        bool                yUpAxis_ = false;
    };
}
