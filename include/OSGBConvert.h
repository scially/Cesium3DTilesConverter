#pragma once

#include <Cesium3DTiles/BoundingVolumeRegion.h>
#include <Cesium3DTiles/BoundingVolumeBox.h>

#include <OSGBuildState.h>
#include <OSGBPageLodVisitor.h>
#include <QSharedPointer>
#include <QString>
#include <QDir>
#include <QVector>


namespace scially {
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

        bool yUpAxis = false;

    private:
        tinygltf::Material makeColorMaterialFromRGB(double r, double g, double b);
        QByteArray convertGLB();
        QString absoluteLocation() const;

        QString              nodeName;
        QString              nodePath;

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
