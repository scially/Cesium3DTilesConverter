#pragma once

#include <OSGConvert/OSGNode.h>
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

	class OSGTile: public OSGIndexNode, public QEnableSharedFromThis<OSGTile> {
	public:
		using Ptr = QSharedPointer<OSGTile>;

		// inherit OSGNode
		// property
		virtual QString name() { 
			return "OSGTile"; 
		}
		// end inherit

		// class OSGTile
		OSGTile(
			const QString& tileFolder,
			const QString& fileName, 
			const QString& tileName);
		
		virtual ~OSGTile() = default;
		
		bool skipOSGLOD() const {
			return mSkipOSGLod;
		}
		
		bool buildIndex();
		
	private:
		bool mSkipOSGLod = false;
	};
}
