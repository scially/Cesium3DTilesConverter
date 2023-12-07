#include <CesiumGLTF/CesiumB3DM.h>
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
#include <QtDebug>

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

		mOSGNode = osgDB::readRefNodeFile(absolutePath(".osgb").toStdString());

		// maybe throw exception if mOSGNode is null
		// TODO
	}
	
	bool OSGTile::buildIndex() {
		if (mOSGNode == nullptr)
			return false;

		OSGLodVisitor lodVisitor;
		mOSGNode->accept(lodVisitor);

		if (!lodVisitor.isValid()) {
			qCritical() << "osg bounding box is not valid:" << tileName();
			return nullptr;
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

	QSharedPointer<OSGIndexNode> OSGTile::toB3DM(
		const SpatialTransform& transform,
		const TileStorage& storage) {
		
		double geometricError = osgBoundingSize(mBoundingBox) / SPLIT_PIXEL;
		
		auto osgIndexNodes =
			collectChildrenMatchGeometricError<OSGTile>(geometricError);

		if (osgIndexNodes.isEmpty())
			return nullptr;

		osg::Vec3d tileCenter = transform.transform(mBoundingBox.center());

		CesiumB3DM::Ptr b3dm(new CesiumB3DM);
		b3dm->center = tileCenter;
		double maxGeometricError = 0;
		foreach(auto node, osgIndexNodes) {
			auto meshes = OSGBtoCesiumMesh(
				*mOSGNode,
				tileCenter,
				transform);

			b3dm->meshes.append(meshes);
			maxGeometricError = std::max(maxGeometricError, node->geometricError());
		}

		if (b3dm->meshes.isEmpty()) {
			return nullptr;
		}

		auto b3dmTopNode = QSharedPointer<B3DMTile>::create(mTileFolder, mFileName, mTileName);
		b3dmTopNode->geometricError() = maxGeometricError;

		QString outFileName = b3dmTopNode->relativePath(".b3dm");

		QByteArray b3dmBuffer;
		if (!b3dm->toGltfBinaryWithNoPack(b3dmBuffer)) {
			qCritical() << "fialed convert meshes to b3dm buffer";
			return nullptr;
		}

		if (!storage.saveFile(outFileName, b3dmBuffer)) {
			return nullptr;
		}

		
		for(const auto& mesh: b3dm->meshes) {
			b3dmTopNode->boundingBox().expandBy(mesh.boundingBox());
		}

		// Attention!
		b3dmTopNode->boundingBox().xMin() += tileCenter.x();
		b3dmTopNode->boundingBox().yMin() += tileCenter.y();
		b3dmTopNode->boundingBox().zMin() += tileCenter.z();
		b3dmTopNode->boundingBox().xMax() += tileCenter.x();
		b3dmTopNode->boundingBox().yMax() += tileCenter.y();
		b3dmTopNode->boundingBox().zMax() += tileCenter.z();

		auto children = firstSplitedChild<OSGTile>();
		if (children.isEmpty()) {
			children = nodes<OSGTile>();
		}

		for(const auto& cn: children) {
			auto n = cn->toB3DM(transform, storage);
			if (n) {
				b3dmTopNode->append(n);
			}
		}
		return b3dmTopNode;
	}
}
