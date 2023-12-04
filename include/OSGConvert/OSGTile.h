#pragma once

#include <CesiumGLTF/CesiumB3DM.h>
#include <OSGConvert/OSGIndex.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <CesiumMath/SpatialReference.h>
#include <Commons/TileStorage.h>

#include <QString>
#include <QList>
#include <QVector>
#include <QSharedPointer>

#include <osg/Vec3d>
#include <osg/NodeVisitor>
#include <osg/PagedLOD>

namespace scially {

	class OSGTile: public QEnableSharedFromThis<OSGTile> {
	public:
		using Ptr = QSharedPointer<OSGTile>;
		friend class OSGFolder;
		friend class MergeTileNode;

		OSGTile(const QString& tileFolder
			, double minGeometricError
			, uint32_t splitPixel = 512
			, bool skipPerTile = false)
			: mTileFolder(tileFolder)
			, mSplitPixel(splitPixel)
			, mMinGeometricError(minGeometricError)
			, mSkipPerTile(skipPerTile)
		{
		}

		virtual ~OSGTile() {}

		bool init();
		
		bool toB3DM(
			const SpatialTransform& transform, 
			TileStorage& storage);
		
		bool saveJson(const SpatialReference& srs, TileStorage& storage) const;

		osg::BoundingBoxd boungdingBox() const { return mBoundingBox; }
		QString tileName() const { return mTileName; }
		int32_t xIndex() const { return mXIndex; }
		int32_t yIndex() const { return mYIndex; }
		int32_t zIndex() const { return mZIndex; }
		QString rootTileFilePath() const { return mTileFolder + "/" + mTileName + ".osgb"; }
	
	private:
		bool loadRoot();
		bool buildIndex();

	protected:
		QString mTileFolder;
		QString mTileName;
		QString mFileName;
		int32_t mXIndex = -1;
		int32_t mYIndex = -1;
		int32_t mZIndex = 1;
		
		osg::BoundingBoxd mBoundingBox;

		// convert options
		double mMinGeometricError;
		bool mSkipPerTile;
		uint32_t mSplitPixel;

		TileNode::Ptr mOSGIndexNode = nullptr;
		TileNode::Ptr mB3DMIndexNode = nullptr;
	};
}
