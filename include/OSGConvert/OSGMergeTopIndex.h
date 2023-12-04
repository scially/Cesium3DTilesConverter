#include <Cesium3DTiles/RootTile.h>
#include <CesiumMath/SpatialTransform.h>
#include <OSGConvert/OSGTile.h>

#include <QList>
#include <QSharedPointer>

#include <osg/Node>

#include <algorithm>
#include <iterator>

namespace scially {
	// pyramid
	//    t     z = 1
	//  t   t   z = 2
	//t  t t  t z = 3 
	// TODO:
	// rebuild:
	// MergeTileNode -> TileNode -> VirtualNode £¨OSGTile¡¢OSGFolder)
	class MergeTileNode : public QEnableSharedFromThis<MergeTileNode> {
	public:
		using Ptr = QSharedPointer<MergeTileNode>;
		friend class MergeTileNodeBuilder;
		
		MergeTileNode(OSGTile::Ptr osgTile);
		MergeTileNode(int32_t x, int32_t y, int32_t z);
		virtual ~MergeTileNode() {}

		bool parentIndex(uint32_t z, int32_t& x, int32_t& y) const;

		const int32_t& xIndex() const { return mXIndex; }
		const int32_t& yIndex() const { return mYIndex; }
		const int32_t& zIndex() const { return mZIndex; }
		const QString& tileName() const { return mTileName; }
		const QString& fileName() const { return mFileName; }
		const double& geometricError() const { return mGeometricError; }
		const osg::BoundingBoxd& boundingBox() const { return mBoundingBox; }
		const QList<MergeTileNode::Ptr>& nodes() const { return mNodes; }

		int32_t& xIndex() { return mXIndex; }
		int32_t& yIndex() { return mYIndex; }
		int32_t& zIndex() { return mZIndex; }
		double& geometricError() { return mGeometricError; }
		osg::BoundingBoxd& boundingBox() { return mBoundingBox; }
	    QList<MergeTileNode::Ptr>& nodes() { return mNodes; }

		QString relativePath(const QString& suffix) const {
			return QString("top/top_%1_%2_%3%4")
				.arg(zIndex())
				.arg(xIndex())
				.arg(yIndex())
				.arg(suffix);
		}

		// convert to b3dm
		MergeTileNode::Ptr toB3DM(
			const SpatialTransform& transform,
			const TileStorage& storage,
			double splitPixel);

		RootTile toRootTile() const;

		bool operator== (const MergeTileNode& node);
		
	private:
		osg::ref_ptr<osg::Node> mOSGNode;
		QList<MergeTileNode::Ptr> mNodes;

		OSGTile::Ptr mOSGTile;
	
		osg::BoundingBoxd mBoundingBox;
		double mGeometricError = 0;
		int32_t mXIndex;
		int32_t mYIndex;
		int32_t mZIndex;
		QString mTileName;
		QString mFileName;
	};

	class MergeTileNodeBuilder {
	public:
		static QList<MergeTileNode::Ptr>
			BuildPyramidIndex(
				const QList<MergeTileNode::Ptr>& nodes,
				int32_t maxZ);

		// read and simplyf all z level osg node
		static void
			GenerateOSGNodeInPyramid(
				const QList<MergeTileNode::Ptr>& topNodes,
				int32_t maxZ);
		
		// convert to b3dm
		static QList<MergeTileNode::Ptr> MergeOSGToB3DM(
			const QList<MergeTileNode::Ptr>& topNodes,
			const SpatialTransform& transform,
			TileStorage& storage,
			double splitPixel);
		
	private:
		// read and simplyf osg node
		static void
			GenerateOSGNodeInPyramidFromBottomToUp(
				const QList<MergeTileNode::Ptr>& nodes,
				uint32_t maxZ,
				uint32_t z);
	};

}