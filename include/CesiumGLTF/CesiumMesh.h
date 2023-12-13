#pragma once

#include "CesiumMaterial.h"
#include "CesiumTexture.h"

#include <QString>
#include <QSharedPointer>
#include <QByteArray>
#include <QVector>
#include <osg/Matrixd>
#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec2f>
#include <osg/Vec4f>
#include <osg/BoundingBox>

namespace scially {
	class CesiumMesh {
	public:
		using Ptr = QSharedPointer<CesiumMesh>;
		static QByteArray toGltfBinaryWithNoPack(
			const QList<CesiumMesh>& meshes, 
			const osg::Vec3d& center);
		static QByteArray toB3DM(
			const QByteArray& glb, 
			const osg::Vec3d& center);

		struct VertexData {
			QString name;
			QVector<float> values;
		};
	
		// find VertexData by name, if not find, make it and append to datas
        VertexData& findVertexDataByName(const QString& name);

		void addMesh(const CesiumMesh& mesh);
		bool duplicateVertex(uint32_t idx);

		osg::Vec3d toWorldVertex(const osg::Vec3f& v) const;
		osg::Vec3f toWorldNormal(const osg::Vec3f& v) const;

		osg::BoundingBoxd boundingBox() const;

        QVector<VertexData> vertexDatas;
		QVector<osg::Vec3f> vertex;
		QVector<osg::Vec3f> normals;
		QVector<osg::Vec2f> texcoords;
		QVector<float> batchIDs;
		QVector<uint32_t> indices;
		QString vertexDataAttribute = "CUSTOM";
        CesiumMaterial material;

		osg::Matrixd tileCoordinateSystemToWorld;

	private:
		
		void nanToZero();
	};
}
