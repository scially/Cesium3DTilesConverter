#pragma once

#include <CesiumGLTF/CesiumMesh.h>
#include <CesiumMath/SpatialTransform.h>

#include <osg/Node>
#include <osg/Vec3d>
#include <osg/NodeVisitor>
#include <QMap>
#include <QString>
#include <QSharedPointer>

#include <optional>

namespace scially {
    QList<CesiumMesh> OSGBtoCesiumMesh(
		const QString& osgFile,
		const osg::Vec3d& tileCenter,
		const SpatialTransform& transform);

	QList<CesiumMesh> OSGBtoCesiumMesh(
		osg::Node& osgNode,
		const osg::Vec3d& tileCenter,
		const SpatialTransform& transform);

	class OSGParseVisitor: public osg::NodeVisitor {
	public:
		OSGParseVisitor(osg::Vec3d tileCenter
			, const SpatialTransform* transformer
			, bool reverseTriangle = false )
			: osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			, mTransformer(transformer)
			, mTileCenter(tileCenter)
			, mReverseTriangle(reverseTriangle){
		}

		virtual void apply(osg::Node& node) override;
		virtual void apply(osg::Geode& geode) override;
		virtual void apply(osg::PagedLOD& lod) override;
		
        QList<CesiumMesh> meshes;

    private:
        std::optional<CesiumMaterial> createMaterial(const osg::StateSet* ss);
        std::optional<CesiumMesh> createMesh(const osg::Drawable* draw);

        QMap<const osg::Image*, CesiumMaterial> mOSGMaterialToCesiumMaterial;
		
		// config
		bool mSkipLOD = false;
		bool mReverseTriangle = false;
		const SpatialTransform* mTransformer;
		bool mNoLight = true;
		osg::Vec3d mTileCenter;
	};
}
