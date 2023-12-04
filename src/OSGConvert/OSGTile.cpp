#include <OSGConvert/OSGTile.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <CesiumGLTF/CesiumB3DM.h>
#include <Commons/OSGUtil.h>
#include <CesiumReadWrite/BaseTileReadWriter.h>

#include <QDir>
#include <QDomDocument>
#include <QtDebug>
#include <QtConcurrent>
#include <osgDB/ReadFile>
#include <osg/BoundingBox>

namespace scially {
	bool OSGTile::init()
	{
		QDir tileDir(mTileFolder);
		mTileName = tileDir.dirName();
		mFileName = tileDir.dirName();
		//Tile_+018_+019
		QStringList split = mTileName.split("_");

		if (split.length() >= 2) {
			mXIndex = split[1].toInt();
		}
		if (split.length() >= 3) {
			mYIndex = split[2].toInt();
		}
		
		return loadRoot();
	}

	bool OSGTile::loadRoot() {
		auto rootNode = osgDB::readRefNodeFile(rootTileFilePath().toStdString());

		if (rootNode == nullptr) {
			qCritical() << rootTileFilePath() << "load failed";
			return false;
		}

		OSGLodVisitor visitor;
		rootNode->accept(visitor);

		mBoundingBox = visitor.boundingBox;
		if (!mBoundingBox.valid())
			return false;

		return buildIndex();
	}

	bool OSGTile::buildIndex() {
		if (mSkipPerTile) {
			mMinGeometricError = (mBoundingBox._max - mBoundingBox._min).length2() / mSplitPixel;
		}

		mOSGIndexNode = buildOSGTileNodeTree(
			mTileFolder,
			mFileName,
			mMinGeometricError);

		return mOSGIndexNode != nullptr;
	}

	bool OSGTile::toB3DM(const SpatialTransform& transform, TileStorage& storage) {
        mB3DMIndexNode = OSGBToB3DM(
			mTileFolder,
			mOSGIndexNode, 
			transform, 
			storage, 
			mSplitPixel);
		return mB3DMIndexNode != nullptr;
	}

	bool OSGTile::saveJson(const SpatialReference &srs, TileStorage& storage) const {
		RootTile r = mB3DMIndexNode->toRootTile(true);
		BaseTile b;
		b.root = r;
		b.geometricError = r.geometricError;

		BaseTileReadWriter brw;
		
		QString outTilesetName = mB3DMIndexNode->tileName() + '/' + mB3DMIndexNode->fileName() + ".json";
        if (!storage.saveJson(outTilesetName, brw.writeToJson(b))) {
			qCritical() << "save json " << outTilesetName << "failed";
			return false;
		}

		// for debug
		r.transform = osgMatrixToCesiumTransform(srs.originENU());
		b.root = r;
		b.geometricError = osgBoundingSize(mB3DMIndexNode->boundingBox);
		outTilesetName = mB3DMIndexNode->tileName() + "/tileset.json";
        if (!storage.saveJson(outTilesetName, brw.writeToJson(b))) {
			qCritical() << "save json " << outTilesetName << "failed";
			return false;
		}
		return true;
	}
}
