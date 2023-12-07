#pragma once

#include <OSGConvert/OSGConvertOption.h>
#include <OSGConvert/OSGNode.h>
#include <CesiumGLTF/CesiumB3DM.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>

#include <QString>
#include <QList>
#include <QVector>
#include <QSharedPointer>

#include <osg/Vec3d>
#include <osg/NodeVisitor>
#include <osg/PagedLOD>

namespace scially {

	class B3DMTile: public OSGIndexNode, public QEnableSharedFromThis<B3DMTile> {
	public:
		using Ptr = QSharedPointer<B3DMTile>;
		friend class MergeTileNode;
		
		// inherit OSGNode
		// property
		virtual QString name() override { 
			return "B3DMTile"; 
		}

		// convert
		virtual QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& transform,
			const TileStorage& storage) override {
			return nullptr;
		}
		// end inherit

		// class OSGTile
		B3DMTile() = default;
		virtual ~B3DMTile() = default;

	private:
	
	};
}
