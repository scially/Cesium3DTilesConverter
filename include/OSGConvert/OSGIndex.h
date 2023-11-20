#pragma once

#include <Cesium3DTiles/RootTile.h>

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QSharedPointer>
#include <osg/BoundingBox>

namespace scially {
	class  TileStorage;
	class  SpatialTransform;

    // for TileNode and TileNodeView, use Pointer as child node type?
	// Tile_+001_+001/Tile_+001_+001.osgb
    //   mTileName      mFileName    suffix
	class TileNode: public QEnableSharedFromThis<TileNode> {
	public:
		using Ptr = QSharedPointer<TileNode>;
		
		bool skipOSGLod = false;
		uint32_t vertexCount = 0;
		uint32_t textureCount = 0;
		double geometricError = 0;
		osg::BoundingBoxd boundingBox;
		QList<TileNode::Ptr> nodes;

		TileNode() {}
		
		TileNode(const QString& tileName, const QString& fileName)
			: mTileName(tileName)
			, mFileName(fileName) {

		}

		QString relativePath(const QString& suffix) const noexcept { 
			return mTileName + "/" + fileName(suffix);
		}

		QString tileName() const noexcept { return mTileName; }
		QString fileName(const QString& suffix = "") const noexcept { return mFileName + suffix; }

		QList<TileNode::Ptr> firstSplitedChild();
		QList<TileNode::Ptr> collectChildrenMatchGeometricError(double geometricError);

		RootTile toRootTile(bool withChilden) const;

	private:
		QString mTileName;
		QString mFileName;
	};

	TileNode::Ptr buildOSGTileNodeTree(
		const QString& tileFolder,
		const QString& fileName,
		double minGeometricError);

	TileNode::Ptr OSGBToB3DM(
		const QString tileFolder,
		TileNode::Ptr osgb,
		const SpatialTransform& transform,
		TileStorage& storage,
		double splitPixel);
}
