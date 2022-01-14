#pragma once

#include <QVector>
#include <QSet>
#include <QMap>

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

namespace gzpi {
	
    class OSGBPageLodVisitor : public osg::NodeVisitor
    {
    public:
        OSGBPageLodVisitor(const QString& path)
            :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), path(path)
        {}
    
        virtual ~OSGBPageLodVisitor() {}
    
        void apply(osg::Geometry& geometry) {
            geometryArray.append(&geometry);
            if (auto ss = geometry.getStateSet()) {
                osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
                if (tex) {
                    textureArray.insert(tex);
                    textureMap[&geometry] = tex;
                }
            }
        }
    
        void apply(osg::PagedLOD& node) {
            int n = node.getNumFileNames();
            for (unsigned int i = 1; i < n; i++)
            {
                QString fileName = path + "/" + QString::fromStdString(node.getFileName(i));
                subNodeNames.append(fileName);
            }
            traverse(node);
        }
    
    public:
        QString                             path;
        QVector<osg::Geometry*>             geometryArray;
        QSet<osg::Texture*>                 textureArray;
        QMap<osg::Geometry*, osg::Texture*> textureMap;  // 记录 mesh 和 texture 的关系，暂时认为一个模型最多只有一个texture
        QVector<QString>                    subNodeNames;
    };
}