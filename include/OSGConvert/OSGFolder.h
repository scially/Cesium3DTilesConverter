#pragma once

#include <OSGConvert/OSGNode.h>
#include <OSGConvert/OSGConvertOption.h>
#include <OSGConvert/OSGTile.h>
#include <Cesium3DTiles/BaseTile.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>

#include <QString>

namespace scially {
	// oblique photography datasets
	// include metadata.xml and Data folder 
	class OSGFolder: public OSGNode {
	public:
		using Ptr = QSharedPointer<OSGFolder>;
		static OSGFolder::Ptr ReadRefFolderNode(const OSGConvertOption& options);
		static SpatialReference::Ptr ReadMetaData(const QString& input);
		// inherit from OSGNode
		virtual QString name() {
			return "OSGFolder"; 
		}
		
		virtual QString fileName() const {
			return "";
		}

		virtual QString tileName() const {
			return "";
		}

		virtual QString filePath() const {
			return mDataPath;
		}

		// child nodes (OSGTile)
		virtual size_t size() const {
			return mTiles.size();
		}

		// child nodes (OSGTile)
		virtual OSGNode* node(size_t i) const {
			return mTiles[i].get();
		}

		// end inherit

		bool append(const QSharedPointer<OSGNode>& node) {
			auto dynNode = node.dynamicCast<OSGTile>();
			if (dynNode) {
				mTiles.append(dynNode);
				return true;
			}
			return false;
		}

		// begin class Folder
		SpatialReference* inSRS() const {
			return mInSRS.get();
		}

		SpatialReferenceMatrix outSRS() const {
			return mOutSRS;
		}

		SpatialTransform* transform() const {
			return mSTS.get();
		}

		TileStorage* storage() const {
			return mStorage.get();
		}

		QString dataPath() const {
			return mDataPath;
		}

		void setInSRS(const SpatialReference::Ptr& srs) {
			mInSRS = srs;
		}

		void setOutSRS(const SpatialReferenceMatrix& srs) {
			mOutSRS = srs;
		}

		void setTransform(const SpatialTransform::Ptr& sts) {
			mSTS = sts;
		}

		void setStorage(const TileStorage::Ptr& storage) {
			mStorage = storage;
		}

		void setDataPath(const QString& path) {
			mDataPath = path;
		}

		bool load(const OSGConvertOption& options);
		bool toB3DMPerTile(const OSGConvertOption& options);
		bool mergeTile() const;
		bool mergeTop(const OSGConvertOption& options) const;

	private:
		BaseTile toBaseTile() const;

		QList<OSGTile::Ptr> mTiles;

        SpatialReference::Ptr  mInSRS;
		SpatialReferenceMatrix mOutSRS;
		SpatialTransform::Ptr mSTS;
		TileStorage::Ptr mStorage;

		QString mDataPath;
	};
}