#pragma once

#include <GDALWrapper.h>

#include <osg/Vec3>

#include <QString>

namespace scially {
    
    class ModelMetadata {
    public:
        QString version() const noexcept  { return version_; }
        QString srs()  const noexcept { return srs_; }
        osg::Vec3 srsOrigin() const  noexcept { return srsOrigin_; }
        
        void loadFromFile(const QString &input);
    private:
        void transformToWGS();

        QString srs_;
        osg::Vec3 srsOrigin_;
        QString version_;
    };
}
