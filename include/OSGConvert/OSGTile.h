#pragma once

#include <OSGConvert/OSGConvertOption.h>
#include <OSGConvert/OSGNode.h>
#include <CesiumGLTF/CesiumB3DM.h>
#include <OSGConvert/OSGIndex.h>
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
		friend class MergeTileNode;
		
		static OSGTile::Ptr ReadRefTileNode(const QString& tileFolder, const OSGConvertOption& options);

		// inherit OSGNode
		// property
		virtual QString name() { 
			return "OSGTile"; 
		}
		
		virtual QString fileName() const {
			return mFileName;
		}

		virtual QString tileName() const {
			return mTileName;
		}
		
		virtual QString filePath() const {
			return mTileFolder;
		}

		// child nodes
		virtual size_t size() const {
			return nodes.size();
		}

		virtual OSGNode* node(size_t i) const {
			return nodes[i].get();
		}

		virtual osg::BoundingBoxd boungdingBox() const override {
			return mBoundingBox;
		}

		virtual int32_t xIndex() const override {
			return mXIndex;
		}

		virtual int32_t yIndex() const override {
			return mYIndex;
		}

		virtual int32_t zIndex() const override {
			return mZIndex;
		}

		// convert
		virtual QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& targetSRS,
			const TileStorage& storage) const override;
		// end inherit

		// class OSGTile
		OSGTile() = default;
		virtual ~OSGTile() = default;

		bool buildIndex(); 
		bool saveJson(const TileStorage& storage) const;

		QString rootTileFilePath() const { 
			return mTileFolder + "/" + mTileName + ".osgb";
		}

	private:
		QString mTileFolder;
		QString mTileName;
		QString mFileName;
		int32_t mXIndex = -1;
		int32_t mYIndex = -1;
		int32_t mZIndex = 1;
		
		osg::BoundingBoxd mBoundingBox;
		double mGeometricError = 0;

		QList<OSGTile::Ptr> nodes;
	
		TileNode::Ptr mB3DMIndexNode = nullptr;
	};
}
