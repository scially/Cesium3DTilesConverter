#pragma once

#include <tiny_gltf/tiny_gltf.h>
#include <tiny_gltf/stb_image_write.h>
#include <DxtImage.h>

#include <QVector>
#include <QSet>
#include <QMap>

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

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
        QMap<osg::Geometry*, osg::Texture*> textureMap;
        QVector<QString>                    subNodeNames;
    };
}
