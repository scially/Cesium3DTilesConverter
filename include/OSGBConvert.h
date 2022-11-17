#pragma once

#include <OSGBRegion.h>
#include <OSGBuildState.h>
#include <OSGBPageLodVisitor.h>

#include <QFileInfo>
#include <QDir>

namespace scially {
    class OSGBConvert {
    public:
        OSGBConvert() {}
        OSGBConvert(const QString& name, const QString& path) :
            nodeName_(name), nodePath_(path) {
            nodeLocation_ = QDir::cleanPath(path + "/" + name + ".osgb");
        }

        OSGBConvert(const QString& absoluteLocation) : nodeLocation_(absoluteLocation){
            QFileInfo location(nodeLocation_);
            nodeName_ = location.baseName();
            nodePath_ = location.path();
        }

        bool writeB3DM(const QByteArray& buffer, const QString& outLocation);
        bool toB3DM(QByteArray& buffer);

        void setYUpAxis(bool yUpAxis) noexcept { yUpAxis_ = yUpAxis;  }
        OSGBRegion region() const noexcept { return region_;  }
        bool yUpAxis() const noexcept { return yUpAxis_; }
        QString nodeName() const noexcept { return nodeName_; }
        QString nodePath() const noexcept { return nodePath_; }
    private:
        tinygltf::Material makeColorMaterialFromRGB(double r, double g, double b);
        bool convertGLB(QByteArray& buffer);

        OSGBRegion region_;
        bool yUpAxis_ = false;
        QString nodeName_;
        QString nodePath_;
        QString nodeLocation_;
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
