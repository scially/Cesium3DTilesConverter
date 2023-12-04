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
	bool OSGFolder::load(const OSGConvertOption& options) {
		QString metadataPath = options.input + "/metadata.xml";
		if (!loadMetaData(metadataPath))
			return false;

		mStorage = TileStorage::create(options.output);
		if (mStorage == nullptr) {
			return false;
		}

		QDir dataDir(options.input + "/Data");
		auto tileFolders = dataDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

		osg::BoundingBoxd osgBound;

		for (const auto& tileFolder : tileFolders) {
			OSGTile::Ptr tile{ 
				new OSGTile(
				tileFolder.absoluteFilePath(),
				options.MinGeometricError,
				options.SplitPixel,
				options.skipPerTile)
			};

			if (!tile->init()) {
				qWarning() << "load" << tileFolder.fileName() << "failed";
				continue;
			}
			mTiles.append(tile);
			osgBound.expandBy(tile->boungdingBox());
		}

		if (mTiles.isEmpty())
			return false;

        const osg::Vec3d osgCenter = mInSRS->toWorld(osgBound.center());

        mOutSRS.initWithCartesian3(osgCenter);

        mSTS = SpatialTransform::create(*mInSRS, mOutSRS);
		return mSTS != nullptr;
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
                    return tile->saveJson(mOutSRS, *mStorage);
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

	bool OSGFolder::loadMetaData(const QString& input) {
		QFile metaDataFile(input);
		if (!metaDataFile.exists()) {
			qCritical() << "can't find metadata.xml file";
			return false;
		}

		QDomDocument metaDataDom;
		if (!metaDataDom.setContent(&metaDataFile)) {
			qCritical() << "can't parse metadata.xml file";
		}

		QDomElement rootElement = metaDataDom.documentElement();
		if (rootElement.tagName() != "ModelMetadata") {
			qCritical() << "not find ModelMetaData node in metadata.xml";
		}

		QDomNodeList srsNodes = rootElement.elementsByTagName("SRS");
		QDomNodeList originNodes = rootElement.elementsByTagName("SRSOrigin");

		if (srsNodes.isEmpty()) {
			qCritical() << "not find SRS node in metadata.xml";
			return false;
		}

		QString srs = srsNodes.at(0).toElement().text();

		if (!originNodes.isEmpty()) {
			QString origin = originNodes.at(0).toElement().text();
			auto origins = origin.split(',');

			mOrigin.x() = origins[0].toDouble();
			mOrigin.y() = origins[1].toDouble();
			if (origins.size() > 2) {
				mOrigin.z() = origins[2].toDouble();
			}
		}

        mInSRS = SpatialReference::create(srs, mOrigin);

        return mInSRS != nullptr;
	}

	QList<OSGTile::Ptr> OSGFolder::tiles() const {
		QList<OSGTile::Ptr> validTile;
		std::copy_if(mTiles.begin(), mTiles.end(), 
			std::back_inserter(validTile), 
			[](const OSGTile::Ptr& p) {
				return p->mB3DMIndexNode != nullptr;
			});
		return validTile;
	}
}
