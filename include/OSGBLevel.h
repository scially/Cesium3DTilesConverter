#pragma once

#include <OSGBuildState.h>
#include <OSGBPageLodVisitor.h>
#include <RootTile.h>
#include <BaseTile.h>
#include <QSharedPointer>
#include <QString>
#include <QDir>
#include <QVector>

#include <limits>

namespace scially {
    class OSGBLevel {
    public:
        static constexpr const char* OSGBEXTENSION = ".osgb";
        static constexpr const char* B3DMEXTENSION = ".b3dm";

        using OSGBLevelPtr = QSharedPointer<OSGBLevel>;

        OSGBLevel() = delete;
        OSGBLevel(const QString& absoluteLocation) {
            QDir location(absoluteLocation);
            QString dirName = location.dirName();
            nodeName = dirName.left(dirName.lastIndexOf("."));
            location.cdUp();
            nodePath = location.absolutePath();
        }

        double              geometricError;
        BoundingVolumeBox   region;
        QString             nodeName;
        QString             nodePath;
        QVector<OSGBLevel>  subNodes;
        osg::Vec3d          bBoxMax;
        osg::Vec3d          bBoxMin;

        QString absoluteLocation() const;

        /// <summary>
        /// 提取 Tile_+154_+018_L22_0000320.osgb _L后面的数字
        /// 提取失败，返回 0
        /// </summary>
        int getLevelNumber() const;
        QString getTileName() const;
        bool getAllOSGBLevels(int maxLevel);
        bool convertTiles(BaseTile &tile, const QString& output, int maxLevel = std::numeric_limits<int>::max());
        bool convertTiles(RootTile &root, const QString& output);


    private:
        void createDir(const QString& output) const;
    };
}
