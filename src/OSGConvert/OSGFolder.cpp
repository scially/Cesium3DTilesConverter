#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <Commons/OSGUtil.h>
#include <OSGConvert/B3DMTile.h>
#include <OSGConvert/OSGFolder.h>
#include <OSGConvert/OSGVirtualTile.h>

#include <QDir>
#include <QSet>
#include <QDomDocument>
#include <QtConcurrent>
#include <QtDebug>
#include <QThreadPool>

#include <algorithm>

namespace scially {
	bool OSGFolder::load(const QString& output) {
        QString metadataPath = mTileFolder + "/metadata.xml";
		mInSRS = ReadMetaData(metadataPath);
		if (nullptr == mInSRS)
			return false;

		mStorage = TileStorage::create(output);
		if (mStorage == nullptr) {
			return false;
		}
		
		QDir dataDir(tileFolder() + "/Data");
		auto tileFolders = dataDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

		osg::BoundingBoxd osgBound;

		for (const auto& tileFolder : tileFolders) {
			QString tn = tileFolder.fileName();  // tileName;
			QString fn = tileFolder.fileName();  // fileName;
			
			auto tileNode = QSharedPointer<OSGTile>::create(tileFolder, fn, tn);
			
			if (!tileNode->buildIndex()) {
				qWarning() << "load" << tileFolder.fileName() << "failed";
				continue;
			}

			mNodes.append(tileNode);
			osgBound.expandBy(tileNode->boundingBox());
		}

		if (size() == 0)
			return false;


		const osg::Vec3d osgCenter = mInSRS->toWorld(osgBound.center());
		mOutSRS.initWithCartesian3(osgCenter);

		mSTS = SpatialTransform::create(*mInSRS, mOutSRS);
		if (mSTS == nullptr)
			return false;

		return true;
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

	bool OSGFolder::toB3DM(uint32_t thread, bool topMerge) {
		QThreadPool threadPool;
		if (thread != 0) {
			threadPool.setMaxThreadCount(thread);
		}
	
		QList<QFuture<bool>> workFutures;
		for (size_t i = 0; i < size(); i++) {
			auto tile = node<OSGTile>(i);
			QFuture<bool> f = QtConcurrent::run(&threadPool, [this, tile]() {
				qInfo() << tile->tileName() << "tile start convert to b3dm";
			
				auto b3dm = tile->toB3DM(*mSTS, *mStorage);
				if(b3dm) {
                    mLinks[tile.get()] = b3dm;
                    return b3dm->saveJson(*mStorage, mOutSRS.originENU());
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
		qInfo() << "all tile process finished";
		
		bool r = mergeTile();
		if (r && topMerge) {
			return mergeTop();
		}

		return r;	
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

		for (size_t i = 0; i < size(); i++) {
			auto b3dmIndex = mLinks[node<OSGIndexNode>(i).get()];
			if (!b3dmIndex)
				continue;

			mergeBoundingBox.expandBy(b3dmIndex->boundingBox());
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

	bool OSGFolder::mergeTop() const {
		int32_t minx = std::numeric_limits<int32_t>::max();
		int32_t miny = std::numeric_limits<int32_t>::max();
	
		int32_t maxx = std::numeric_limits<int32_t>::min();
		int32_t maxy = std::numeric_limits<int32_t>::min();

        QList<QSharedPointer<OSGIndexNode>> nodes;

		for (size_t i = 0; i < size(); i++) {
			auto b3dmIndex = mLinks[node<OSGIndexNode>(i).get()];
			if (!b3dmIndex)
				continue;

            nodes.push_back(b3dmIndex);
			
            minx = std::min(minx, b3dmIndex->xIndex());
            miny = std::min(miny, b3dmIndex->yIndex());
				   
            maxx = std::max(maxx, b3dmIndex->xIndex());
            maxy = std::max(maxy, b3dmIndex->yIndex());
		}

		int32_t maxIndex = std::max({ maxx - minx, maxy - miny}); 

		if (maxIndex <= 0) {
			return false;
		}
		
		int32_t maxZ = static_cast<int32_t>(std::log2(maxIndex) + 1) + 1; // z start from 1
		
		// buiding pyramid index
        auto topNodes = OSGPyramidBuilder::BuildPyramidIndex(nodes, maxZ);
		
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
