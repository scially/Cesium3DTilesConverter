#pragma once

#include <CesiumGLTF/CesiumB3DM.h>
#include <OSGConvert/OSGIndex.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <CesiumMath/SpatialReference.h>
#include <Commons/TileStorage.h>

#include <QString>
#include <QList>
#include <QVector>

#include <osg/Vec3d>
#include <osg/NodeVisitor>
#include <osg/PagedLOD>

namespace scially {
	class OSGTile {
	public:
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

		bool init();
		
		bool toB3DM(
			const SpatialTransform& transform, 
			TileStorage& storage);
		
		bool saveJson(const SpatialReference& srs, TileStorage& storage) const;

		const TileNode::Ptr b3dms() const { return mB3DMIndexNode; }
		osg::BoundingBoxd boungdingBox() const { return mBoundingBox; }
		QString tileName() const { return mTileName; }
		QString rootTileFilePath() const { return mTileFolder + "/" + mTileName + ".osgb"; }
	
	protected:
		bool loadRoot();
		bool buildIndex();

	private:
		QString mTileFolder;
		QString mTileName;
		QString mFileName;
		int32_t mXIndex = -1;
		int32_t mYIndex = -1;
		int32_t mZIndex = -1;
		
		osg::BoundingBoxd mBoundingBox;

		// convert options
		double mMinGeometricError;
		bool mSkipPerTile;
		uint32_t mSplitPixel;

		TileNode::Ptr mOSGIndexNode = nullptr;
		TileNode::Ptr mB3DMIndexNode = nullptr;
	};
}
