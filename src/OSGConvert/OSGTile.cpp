#include <Commons/OSGUtil.h>
#include <OSGConvert/B3DMTile.h>
#include <OSGConvert/OSGFolder.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <OSGConvert/OSGParseVisitor.h>
#include <OSGConvert/OSGTile.h>

#include <QDir>
#include <QDomDocument>
#include <QException>
#include <QtConcurrent>

#include <osg/BoundingBox>
#include <osgDB/ReadFile>

namespace scially {
	OSGTile::OSGTile(const QString& tileFolder, const QString& fileName, const QString& tileName)
	{
		mTileFolder = tileFolder;
		mFileName = fileName;
		mTileName = tileName;
		//Tile_+018_+019
		QStringList split = mTileName.split("_");

		if (split.length() >= 2) {
			mXIndex = split[1].toInt();
		}
		if (split.length() >= 3) {
			mYIndex = split[2].toInt();
		}

	}
	
	bool OSGTile::buildIndex() {
		auto osgNode = osgDB::readRefNodeFile(absoluteNodePath(".osgb").toStdString());

		if (osgNode == nullptr)
			return false;

		OSGLodVisitor lodVisitor;
		osgNode->accept(lodVisitor);

		if (!lodVisitor.isValid()) {
			qCritical("tile %s bounding box is not valid", qUtf8Printable(tileName()));
            return false;
		}

		mBoundingBox = lodVisitor.boundingBox;
		
		double sumPixel = 0;
		foreach(const auto & node, lodVisitor.children) {
			sumPixel += node.maxPixel;
		}
		if (sumPixel == 0) {
			mGeometricError = 0;
		}
		else {
			mGeometricError = osgBoundingSize(mBoundingBox) / sumPixel;
		}

		for(const auto & node: lodVisitor.children) {
			for (const auto& tileName : node.tileNames) {
				auto leafNode = QSharedPointer<OSGTile>::create(
					mTileFolder,
					mFileName,
					tileName
				);

				if (leafNode->buildIndex()) {
					mNodes.append(leafNode);
				}
			}
		}

		if (mNodes.isEmpty()) {
			mGeometricError = 0;
		}

		else {
			if (lodVisitor.hasContentOutLOD) {
				auto leafNode = QSharedPointer<OSGTile>::create(
					mTileFolder,
					mFileName,
					mTileName
				);

				leafNode->mBoundingBox = lodVisitor.boundingBox;
			
				leafNode->mGeometricError = 0;
				leafNode->mSkipOSGLod = true;
				mNodes.append(leafNode);
			}
		}

		return true;
	}
}
