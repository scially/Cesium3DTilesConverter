#pragma once

#include <OSGConvert/OSGConvertOption.h>
#include <OSGConvert/OSGTile.h>
#include <Cesium3DTiles/BaseTile.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>

#include <QString>

namespace scially {
	class OSGFolder {
	public:
		bool load(const OSGConvertOption& options);
		bool toB3DMPerTile(const OSGConvertOption& options);
		bool mergeTile() const;
		bool mergeTop(const OSGConvertOption& options) const;

		QList<OSGTile::Ptr> tiles() const;
	private:
		bool loadMetaData(const QString& input);
		
		BaseTile toBaseTile() const;

		QList<OSGTile::Ptr> mTiles;
        SpatialReference::Ptr mInSRS;
        SpatialReferenceMatrix mOutSRS;
		SpatialTransform::Ptr mSTS;
		TileStorage::Ptr mStorage;
		osg::Vec3f mOrigin = { 0,0,0 };
	};
}
