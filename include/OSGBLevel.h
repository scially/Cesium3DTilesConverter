#pragma once

#include <OSGBuildState.h>
#include <OSGBPageLodVisitor.h>
#include <QSharedPointer>
#include <QString>
#include <QDir>
#include <QVector>

namespace gzpi {
    class OSGBLevel {
    public:
        using OSGBLevelPtr = QSharedPointer<OSGBLevel>;
        struct OSGBMesh
        {
            QString name;
            osg::Vec3d min;
            osg::Vec3d max;
        };



        OSGBLevel() {}
        OSGBLevel(const QString& name, const QString& path) :
            nodeName(name), nodePath(path) {}
        OSGBLevel(const QString& absoluteLocation) {

        }

        double              geometricError;
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
        int getLevelNumber();

        bool getAllOSGBLevels(int maxLevel);

        /// <summary>
        /// 瓦片及PageLOD下的所有子节点转B3DM
        /// </summary>
        /// <param name="outLocation"></param>
        /// <returns></returns>
        bool writeB3DM(const QString& outLocation);

        /// <summary>
        /// OSGB文件转B3DM
        /// </summary>
        /// <param name="path"></param>
        /// <param name="tile_box"></param>
        /// <returns></returns>
        bool convertB3DM(QByteArray& b3dmBuffer);

        bool convertGLB(QByteArray& glbBuffer, OSGBMesh& mesh);

    private:
        tinygltf::Material makeColorMaterialFromRGB(double r, double g, double b) {
            tinygltf::Material material;
            material.name = "default";
            tinygltf::Parameter baseColorFactor;
            baseColorFactor.number_array = { r, g, b, 1.0 };
            material.values["baseColorFactor"] = baseColorFactor;

            tinygltf::Parameter metallicFactor;
            metallicFactor.number_value = new double(0);
            material.values["metallicFactor"] = metallicFactor;
            tinygltf::Parameter roughnessFactor;
            roughnessFactor.number_value = new double(1);
            material.values["roughnessFactor"] = roughnessFactor;
            //
            return material;
        }
    };
}