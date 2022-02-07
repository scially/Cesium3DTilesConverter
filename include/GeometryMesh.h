#pragma once

#include <ogrsf_frmts.h>

#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <osg/Vec3f>
#include <osg/Array>
#include <osg/Geometry>


namespace gzpi {
    class GeometryMesh {
    public:
        QString meshName;
        QVector<osg::Vec3f> vertex;
        QVector<osg::Vec3f> index;
        QVector<osg::Vec3f> normal;
        float height;


        QSharedPointer<osg::Geometry> triangleMesh() {
           osg::Vec3Array *va = new osg::Vec3Array(vertex.size());
            for (int i = 0; i < vertex.size(); i++) {
                (*va)[i].set(vertex[i][0], vertex[i][1], vertex[i][2]);
            }
            osg::Vec3Array *vn = new osg::Vec3Array(normal.size());
            for (int i = 0; i < normal.size(); i++) {
                (*vn)[i].set(normal[i][0], normal[i][1], normal[i][2]);
            }
            QSharedPointer<osg::Geometry> geometry(new osg::Geometry);
            geometry->setVertexArray(va);
            geometry->setNormalArray(vn);
            osg::DrawElementsUShort* drawElements = new osg::DrawElementsUShort(osg::DrawArrays::TRIANGLES);
            for (int i = 0; i < index.size(); i++) {
                drawElements->addElement(index[i][0]);
                drawElements->addElement(index[i][1]);
                drawElements->addElement(index[i][2]);
            }
            geometry->addPrimitiveSet(drawElements);
            //osgUtil::SmoothingVisitor::smooth(*geometry);
            return geometry;
        }
        virtual ~GeometryMesh() {
            for(auto iter = meshes.begin(); iter != meshes.end(); iter++){
                if(*iter != nullptr)
                    delete *iter;
            }
        }
    private:
        void calcNormal(int baseCnt, int ptNum)
        {
            // normal stand for one triangle
            for (int i = 0; i < ptNum; i+=2) {
                osg::Vec2 nor1(vertex[baseCnt + 2 * (i + 1)][0], vertex[baseCnt + 2 * (i + 1)][1]);
                nor1 = nor1 - osg::Vec2(vertex[baseCnt + 2 * i][0], vertex[baseCnt + 2 * i][1]);
                osg::Vec3 nor3 = osg::Vec3(-nor1.y(), nor1.x(), 0);
                nor3.normalize();

                normal.push_back({ nor3.x(), nor3.y(), nor3.z() });
                normal.push_back({ nor3.x(), nor3.y(), nor3.z() });
                normal.push_back({ nor3.x(), nor3.y(), nor3.z() });
                normal.push_back({ nor3.x(), nor3.y(), nor3.z() });
            }
        }


        void init(double centerX, double centerY, double height, QSharedPointer<OGRPolygon> polygon);
        QVector<GeometryMesh*> meshes;
    };
}
