#pragma once

#include <ogrsf_frmts.h>

#include <QByteArray>
#include <QString>
#include <QVector>

#include <osg/Vec3f>
#include <osg/Vec3i>
#include <osg/Array>
#include <osg/Geometry>

namespace scially {
    class GeometryMesh {
    public:
        QString meshName;
        QVector<osg::Vec3f> vertex;
        QVector<osg::Vec3i> index;
        QVector<osg::Vec3f> normal;
        float height;

        osg::ref_ptr<osg::Geometry> triangleMesh();
        void add(double centerX, double centerY, double height, OGRPolygon* polygon);
        void add(double centerX, double centerY, double height, OGRMultiPolygon* polygon);
        QByteArray toB3DM(bool withHeight = false);
        QVector<int> getBatchID() const;
        QVector<QString> getBatchName() const;
        QVector<double>  getBatchHeights() const;
        QByteArray getGlbBuffer() const;
        virtual ~GeometryMesh();

    private:
        void calcNormal(int baseCnt, int ptNum);
        void init(double centerX, double centerY, double height, OGRPolygon* polygon);
        
        QVector<GeometryMesh*> meshes;
    };
}
