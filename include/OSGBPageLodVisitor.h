#pragma once

#include <QVector>
#include <QSet>
#include <QMap>

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <tiny_gltf.h>
#include <stb_image_write.h>
#include <DxtImage.h>

namespace gzpi {
	
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
}
