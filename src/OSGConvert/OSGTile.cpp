#include <OSGConvert/OSGTile.h>
#include <OSGConvert/OSGFolder.h>
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
	OSGTile::Ptr OSGTile::ReadRefTileNode(const QString& tileFolder, const OSGConvertOption& options) {
		OSGTile::Ptr tile{ new OSGTile };
		tile->mTileFolder = tileFolder;

		QDir tileDir(tile->mTileFolder);
		tile->mTileName = tileDir.dirName();
		tile->mFileName = tileDir.dirName();
		//Tile_+018_+019
		QStringList split = tile->mTileName.split("_");

		if (split.length() >= 2) {
			tile->mXIndex = split[1].toInt();
		}
		if (split.length() >= 3) {
			tile->mYIndex = split[2].toInt();
		}

		tile->mOSGNode = osgDB::readRefNodeFile(tile->rootTileFilePath().toStdString());

		if (tile->mOSGNode == nullptr) {
			qCritical() << tile->rootTileFilePath() << "load failed";
			return nullptr;
		}

		return tile;
	}

	bool OSGTile::buildIndex() {
		OSGLodVisitor visitor;
		mOSGNode->accept(visitor);

		mBoundingBox = visitor.boundingBox;
		if (!mBoundingBox.valid())
			return false;

		mOSGIndexNode = buildOSGTileNodeTree(
			mTileFolder,
			mFileName,
			mMinGeometricError);

		return mOSGIndexNode != nullptr;
	}

	QSharedPointer<OSGIndexNode> OSGTile::toB3DM(
		const SpatialTransform& transfrom,
		const TileStorage& storage) {
        mB3DMIndexNode = OSGBToB3DM(
			mTileFolder,
			mOSGIndexNode, 
			transfrom,
			storage,
			mSplitPixel);
		return mB3DMIndexNode;
	}

	bool OSGTile::saveJson(const TileStorage& storage) const {
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
		r.transform = osgMatrixToCesiumTransform(mOutSRS.originENU());
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
