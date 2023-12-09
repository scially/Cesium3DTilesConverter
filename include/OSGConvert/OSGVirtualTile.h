#include <OSGConvert/OSGNode.h>

#include <algorithm>
#include <iterator>

namespace scially {
	// pyramid
	//    t     z = 1
	//  t   t   z = 2
	//t  t t  t z = 3 
	class OSGVirtualTile : public OSGIndexNode, public QEnableSharedFromThis<OSGVirtualTile> {
	public:
		using Ptr = QSharedPointer<OSGVirtualTile>;
		
		// inherit from OSGIndexNode
		virtual QString name() const override {
			return "OSGVirtualTile";
		}
	
		// class OSGVirtualTile
		OSGVirtualTile(int32_t x, int32_t y, int32_t z)
		{
			mXIndex = x;
			mYIndex = y;
			mZIndex = z;
			mFileName = QString("Top_%1").arg(mZIndex);
			mTileName = QString("%1_%2").arg(mXIndex).arg(mYIndex);

			mOSGNode = new osg::Group();
		}
		
		virtual ~OSGVirtualTile() = default;

		virtual QString relativePath(const QString& suffix) const override {
			return QString("top/top_%1_%2_%3%4")
				.arg(zIndex())
				.arg(xIndex())
				.arg(yIndex())
				.arg(suffix);
		}
	};

    class OSGPyramidBuilder {
	public:
        static QList<QSharedPointer<OSGIndexNode>>
			BuildPyramidIndex(
                const QList<QSharedPointer<OSGIndexNode>>& nodes,
				int32_t maxZ);

		// read and simplyf all z level osg node
		static void
			GenerateOSGNodeInPyramid(
				const QList<OSGVirtualTile::Ptr>& topNodes,
				int32_t maxZ);
		
		// convert to b3dm
		static QList<OSGVirtualTile::Ptr> MergeOSGToB3DM(
			const QList<OSGVirtualTile::Ptr>& topNodes,
			const SpatialTransform& transform,
			TileStorage& storage);
		
	private:
		// read and simplyf osg node
		static void
			GenerateOSGNodeInPyramidFromBottomToUp(
				const QList<OSGVirtualTile::Ptr>& nodes,
				uint32_t maxZ,
				uint32_t z);
	};

}
