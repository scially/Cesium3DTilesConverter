#pragma once

#include <GDALWrapper.h>
#include <Tileset.h>
#include <tiny_gltf.h>

#include <osg/NodeVisitor>
#include <osg/PagedLOD>
#include <QSharedPointer>
#include <QString>
#include <QDir>
#include <QVector>

namespace scially {
    class OSGBPageLodVisitor : public osg::NodeVisitor
    {
    public:
        OSGBPageLodVisitor(const QString& path)
            :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), path(path)
        {}
    
        virtual ~OSGBPageLodVisitor() {}
    
        void apply(osg::Geometry& geometry);
        void apply(osg::PagedLOD& node);
    
    public:
        QString                             path;
        QVector<osg::Geometry*>             geometryArray;
        QSet<osg::Texture*>                 textureArray;
        QMap<osg::Geometry*, osg::Texture*> textureMap;  // 记录 mesh 和 texture 的关系，暂时认为一个模型最多只有一个texture
        QVector<QString>                    subNodeNames;
    };

    class OSGBLevel {
    public:
        static constexpr const char* OSGBEXTENSION = ".osgb";
        static constexpr const char* B3DMEXTENSION = ".b3dm";

        void setTileLocation(const QString& absoluteLocation) {
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
        /// ??? Tile_+154_+018_L22_0000320.osgb _L?????????
        /// ??????????? 0
        /// </summary>
        int getLevelNumber() const;
        QString getTileName() const;
        bool getAllOSGBLevels(int maxLevel);
        bool convertTiles(BaseTile &tile, const QString& output, int maxLevel = std::numeric_limits<int>::max());
        bool convertTiles(RootTile &root, const QString& output);
        void updateGeometryError(RootTile &root);

    private:
        void createDir(const QString& output) const;
    };

    class OSGBConvert {
    public:
        OSGBConvert() {}
        OSGBConvert(const QString& name, const QString& path) :
            nodeName(name), nodePath(path) {}
        OSGBConvert(const QString& absoluteLocation) {
            QDir location(absoluteLocation);
            nodeName = location.dirName();
            location.cdUp();
            nodePath = location.absolutePath();
        }

        //double geometricError = 0;
        BoundingVolumeRegion region;

        bool writeB3DM(const QByteArray& buffer, const QString& outLocation);
        QByteArray toB3DM();

    private:
        tinygltf::Material makeColorMaterialFromRGB(double r, double g, double b);
        QByteArray convertGLB();
        QString absoluteLocation() const;

        QString  nodeName;
        QString  nodePath;
    };
}

namespace internal {
    // STB_IMAGE
    struct Color {
        int r;
        int g;
        int b;
    };

    void resizeImage(std::vector<unsigned char>& jpeg_buf, int width, int height, int new_w, int new_h);
    void fill4BitImage(std::vector<unsigned char>& jpeg_buf, osg::Image* img, int& width, int& height);
}
