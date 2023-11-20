#include <OSGConvert/OSGFolder.h>
#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <Commons/OSGUtil.h>

#include <QDir>
#include <QDomDocument>
#include <QtDebug>
#include <QThreadPool>
#include <QtConcurrent>
#include <osgDB/ReadFile>
#include <osg/BoundingBox>

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
			OSGTile tile(
				tileFolder.absoluteFilePath(),
				options.MinGeometricError,
				options.SplitPixel,
				options.skipPerTile);

			if (!tile.init()) {
				qWarning() << "load" << tileFolder.fileName() << "failed";
				continue;
			}
			mTiles.append(tile);
			osgBound.expandBy(tile.boungdingBox());
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
		for (OSGTile& tile : mTiles) {
			QFuture<bool> f = QtConcurrent::run(&threadPool, [this, &tile, &tree]() {
				qInfo() << tile.tileName() << "tile start convert to b3dm";
			
				if (tile.toB3DM(*mSTS, *mStorage)) {
                    return tile.saveJson(mOutSRS, *mStorage);
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

		foreach(const OSGTile & tile, mTiles) {
			auto b3dmIndex = tile.b3dms();
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
}
