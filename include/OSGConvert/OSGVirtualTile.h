#include <OSGConvert/OSGNode.h>
#include <OSGConvert/B3DMTile.h>

#include <osg/Group>

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

	QPointerList<OSGIndexNode>
		BuildPyramidIndex(const QPointerList<OSGIndexNode>& nodes, int32_t maxZ);

    class OSGPyramidBuilder {
	public:

		// read and simplyf all z level osg node
		static void
			GenerateOSGNodeInPyramid(
                const QList<QSharedPointer<OSGIndexNode>>& topNodes,
				int32_t maxZ);
		
		// convert to b3dm
        static QList<QSharedPointer<OSGIndexNode>> MergeOSGToB3DM(
            const QList<QSharedPointer<OSGIndexNode>>& topNodes,
			const SpatialTransform& transform,
            const TileStorage& storage);
		
	private:
		// read and simplyf osg node
		static void
			GenerateOSGNodeInPyramidFromBottomToUp(
            const QPointerList<OSGIndexNode>& topNodes,
            uint32_t maxZ,
            uint32_t z);
	};

}
