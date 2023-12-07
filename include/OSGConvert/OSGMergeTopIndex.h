#include <OSGConvert/OSGNode.h>

#include <algorithm>
#include <iterator>

namespace scially {
	// pyramid
	//    t     z = 1
	//  t   t   z = 2
	//t  t t  t z = 3 
	class MergeTileNode : public OSGIndexNode, public QEnableSharedFromThis<MergeTileNode> {
	public:
		using Ptr = QSharedPointer<MergeTileNode>;
		
		// inherit from OSGIndexNode
		virtual QString name() const override {
			return "MergeTileNode";
		}

		virtual QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& transform,
			const TileStorage& storage)
			override;
		
		MergeTileNode::MergeTileNode(int32_t x, int32_t y, int32_t z)
		{
			mXIndex = x;
			mYIndex = y;
			mZIndex = z;
			mFileName = QString("Top_%1").arg(mZIndex);
			mTileName = QString("%1_%2").arg(mXIndex).arg(mYIndex);
		}
		
		virtual ~MergeTileNode() = default;

		bool parentIndex(uint32_t z, int32_t& x, int32_t& y) const;

		QString relativePath(const QString& suffix) const {
			return QString("top/top_%1_%2_%3%4")
				.arg(zIndex())
				.arg(xIndex())
				.arg(yIndex())
				.arg(suffix);
		}

		RootTile toRootTile() const;

		bool operator== (const MergeTileNode& node);
		
	private:

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
			TileStorage& storage);
		
	private:
		// read and simplyf osg node
		static void
			GenerateOSGNodeInPyramidFromBottomToUp(
				const QList<MergeTileNode::Ptr>& nodes,
				uint32_t maxZ,
				uint32_t z);
	};

}