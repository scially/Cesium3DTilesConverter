#pragma once

#include <OSGConvert/OSGTile.h>
#include <Cesium3DTiles/BaseTile.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>

namespace scially {
	// oblique photography datasets
	// include metadata.xml and Data folder 
	class OSGFolder: public OSGNode {
	public:
		using Ptr = QSharedPointer<OSGFolder>;
		static SpatialReference::Ptr ReadMetaData(const QString& input);
		
		// inherit from OSGNode
		virtual QString name() {
			return "OSGFolder"; 
		}
		// end inherit

		// begin class Folder
		OSGFolder(const QString& basePath) 
		{
			mTileFolder = basePath;
		}

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
			return tileFolder();
		}

		bool load(const QString& output);
		bool toB3DM(uint32_t thread, bool topMerge = false);

	private:
		BaseTile toBaseTile() const;
		
		bool mergeTile() const;
		bool mergeTop() const;

        SpatialReference::Ptr  mInSRS;
		SpatialReferenceMatrix mOutSRS;
		SpatialTransform::Ptr mSTS;
		TileStorage::Ptr mStorage;

		QMap<OSGIndexNode*, OSGIndexNode*> mLinks; // osgb to b3dm link
	};
}