#include <OSGConvert/OSGFolder.h>

#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <Commons/OSGUtil.h>
#include <OSGConvert/B3DMTile.h>

#include <QDir>
#include <QDomDocument>
#include <QSet>
#include <QtConcurrent>
#include <QtDebug>
#include <QThreadPool>

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
			
			auto tileNode = QSharedPointer<OSGTile>::create(tileFolder.path(), fn, tn);
			
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

	QPointerList<OSGIndexNode> OSGFolder::to3DTiles(uint32_t thread) {
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
				if(b3dm && b3dm->saveJson(*mStorage, mOutSRS.originENU())) {
					mB3dms.append(b3dm);
                    return true;
				}
				return false;
			});
			workFutures.append(f);
		}

		for (auto& worker : workFutures) {
			worker.waitForFinished();
		}
		qInfo() << "all tile process finished";
		
		const BaseTile b = B3DMTile::toBaseTile(mB3dms, mOutSRS.originENU());
		BaseTileReadWriter brw;
		const QJsonObject obj = brw.writeToJson(b);
		if (!mStorage->saveJson("tileset.json", obj)) {
			qCritical() << "write tileset.json failed";
		}
		
		return mB3dms;
	}
}
