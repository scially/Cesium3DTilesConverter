#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <Commons/OSGUtil.h>
#include <OSGConvert/OSGFolder.h>
#include <OSGConvert/OSGMergeTopIndex.h>

#include <osg/BoundingBox>
#include <osgDB/ReadFile>

#include <QDir>
#include <QSet>
#include <QDomDocument>
#include <QtConcurrent>
#include <QtDebug>
#include <QThreadPool>

#include <algorithm>
#include <iterator>

namespace scially {
	OSGFolder::Ptr OSGFolder::ReadRefFolderNode(const OSGConvertOption& options) {
		OSGFolder::Ptr folderNode{ new OSGFolder };
		folderNode->mDataPath = options.input;

		QString metadataPath = folderNode->mDataPath + "/metadata.xml";
		folderNode->mInSRS = ReadMetaData(metadataPath);
		if (nullptr == folderNode->mInSRS)
			return nullptr;

		folderNode->mStorage = TileStorage::create(options.output);
		if (folderNode->mStorage == nullptr) {
			return nullptr;
		}
		
		QDir dataDir(options.input + "/Data");
		auto tileFolders = dataDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

		osg::BoundingBoxd osgBound;

		for (const auto& tileFolder : tileFolders) {
			OSGTile::Ptr tileNode = OSGTile::ReadRefTileNode(tileFolder.absolutePath(), options);
			
			if (tileNode && !tileNode->buildIndex()) {
				qWarning() << "load" << tileFolder.fileName() << "failed";
				continue;
			}

			folderNode->append(tileNode);
			osgBound.expandBy(tileNode->boungdingBox());
		}

		if (folderNode->mTiles.isEmpty())
			return nullptr;


		const osg::Vec3d osgCenter = folderNode->mInSRS->toWorld(osgBound.center());
		folderNode->mOutSRS.initWithCartesian3(osgCenter);

		folderNode->mSTS = SpatialTransform::create(*folderNode->mInSRS, folderNode->mOutSRS);
		if (folderNode->mSTS == nullptr)
			return nullptr;

		return folderNode;
	}

	SpatialReference::Ptr OSGFolder::ReadMetaData(const QString& input) {
		QFile metaDataFile(input);
		if (!metaDataFile.exists()) {
			qCritical() << "can't find metadata.xml";
			return nullptr;
		}

		QDomDocument metaDataDom;
		if (!metaDataDom.setContent(&metaDataFile)) {
			qCritical() << "can't parse metadata.xml file";
			return nullptr;
		}

		QDomElement rootElement = metaDataDom.documentElement();
		if (rootElement.tagName() != "ModelMetadata") {
			qCritical() << "not find ModelMetaData node in metadata.xml";
			return nullptr;
		}

		QDomNodeList srsNodes = rootElement.elementsByTagName("SRS");
		QDomNodeList originNodes = rootElement.elementsByTagName("SRSOrigin");

		if (srsNodes.isEmpty()) {
			qCritical() << "not find SRS node in metadata.xml";
			return nullptr;
		}

		QString srs = srsNodes.at(0).toElement().text();
		osg::Vec3d origin = { 0, 0, 0 };

		if (!originNodes.isEmpty()) {
			auto origins = originNodes.at(0).toElement().text().split(',');

			origin.x() = origins[0].toDouble();
			origin.y() = origins[1].toDouble();
			if (origins.size() > 2) {
				origin.z() = origins[2].toDouble();
			}
		}

		return SpatialReference::CreateSpatialReference(srs, origin);
	}

	bool OSGFolder::toB3DMPerTile(const OSGConvertOption& options) {
		QThreadPool threadPool;
		if (options.thread != 0) {
			threadPool.setMaxThreadCount(options.thread);
		}
	
		QList<QFuture<bool>> workFutures;
		QList<TileNode::Ptr> tree;
		foreach (auto tile, mTiles) {
			QFuture<bool> f = QtConcurrent::run(&threadPool, [this, tile, &tree]() {
				qInfo() << tile->tileName() << "tile start convert to b3dm";
			
				if (tile->toB3DM(*mSTS, *mStorage)) {
                    return tile->saveJson(*mStorage);
				}
				else {
					return false;
				}
			});
			workFutures.append(f);
		}

		for (auto& worker : workFutures) {
			worker.waitForFinished();
		}

		qInfo() << "all tile process finished, start merge";
		return mergeTile();
	}

	bool OSGFolder::mergeTile() const {
		const BaseTile b = toBaseTile();
		BaseTileReadWriter brw;
		const QJsonObject obj = brw.writeToJson(b);
		return mStorage->saveJson("tileset.json", obj);
	}

	BaseTile OSGFolder::toBaseTile() const {
		osg::BoundingBoxd mergeBoundingBox;
		RootTile root;

		foreach(auto tile, mTiles) {
			auto b3dmIndex = tile->mB3DMIndexNode;
			if (b3dmIndex == nullptr)
				continue;

			mergeBoundingBox.expandBy(b3dmIndex->boundingBox);
			RootTile r = b3dmIndex->toRootTile(false);
			root.children.append(r);
		}

		root.boundingVolume.box = osgBoundingToCesiumBoundBox(mergeBoundingBox);
		root.geometricError = osgBoundingSize(mergeBoundingBox);
		root.refine = "REPLACE";
		root.transform = osgMatrixToCesiumTransform(mOutSRS.originENU());

		BaseTile b;
		b.root = root;
		b.geometricError = root.geometricError;
		return b;
	}

	bool OSGFolder::mergeTop(const OSGConvertOption& options) const {
		int32_t minx = std::numeric_limits<int32_t>::max();
		int32_t miny = std::numeric_limits<int32_t>::max();
	
		int32_t maxx = std::numeric_limits<int32_t>::min();
		int32_t maxy = std::numeric_limits<int32_t>::min();
	
		QList<MergeTileNode::Ptr> nodes;

		for(const auto& tile: mTiles) {
			if (!tile->mB3DMIndexNode)
				continue;

			MergeTileNode::Ptr node{ new MergeTileNode(tile) };
			nodes.push_back(node);
			
			minx = std::min(minx, node->xIndex());
			miny = std::min(miny, node->yIndex());
				   
			maxx = std::max(maxx, node->xIndex());
			maxy = std::max(maxy, node->yIndex());
		}

		int32_t maxIndex = std::max({ maxx - minx, maxy - miny}); 

		if (maxIndex <= 0) {
			return false;
		}
		
		int32_t maxZ = static_cast<int32_t>(std::log2(maxIndex) + 1) + 1; // z start from 1
		
		// buiding pyramid index
		QList<MergeTileNode::Ptr> topNodes = MergeTileNodeBuilder::BuildPyramidIndex(nodes, maxZ);
		
		// constrution
		MergeTileNodeBuilder::GenerateOSGNodeInPyramid(topNodes, maxZ);
		auto mergeB3dmTiles = MergeTileNodeBuilder::MergeOSGToB3DM(topNodes, *mSTS, *mStorage, options.SplitPixel);

		if (mergeB3dmTiles.isEmpty()) {
			return false;
		}

		osg::BoundingBoxd mergeBoundingBox;
		RootTile root;

		for(const auto& tile: mergeB3dmTiles) {
			mergeBoundingBox.expandBy(tile->boundingBox());
			RootTile r = tile->toRootTile();
			root.children.append(r);
		}

		root.boundingVolume.box = osgBoundingToCesiumBoundBox(mergeBoundingBox);
		root.geometricError = osgBoundingSize(mergeBoundingBox);
		root.refine = "REPLACE";
		root.transform = osgMatrixToCesiumTransform(mOutSRS.originENU());

		BaseTile b;
		b.root = root;
		b.geometricError = root.geometricError;
		
		BaseTileReadWriter brw;
		const QJsonObject obj = brw.writeToJson(b);
		return mStorage->saveJson("tileset.json", obj);
	}
}
