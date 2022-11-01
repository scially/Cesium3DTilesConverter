#include <QString>
#include <OSGBPageLodVisitor.h>

namespace scially {
 
    void OSGBPageLodVisitor::apply(osg::Geometry& geometry) {
        geometryArray.append(&geometry);
        if (auto ss = geometry.getStateSet()) {
            osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
            if (tex != nullptr) {
                textureArray.insert(tex);
                textureMap[&geometry] = tex;
            }
        }
    }
    
    void OSGBPageLodVisitor::apply(osg::PagedLOD& node) {
        unsigned int n = node.getNumFileNames();
        for (unsigned int i = 1; i < n; i++)
        {
            QString fileName = path + "/" + QString::fromLocal8Bit(node.getFileName(i).c_str());
            subNodeNames.append(fileName);
        }
        traverse(node);
    }

}
