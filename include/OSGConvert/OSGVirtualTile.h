#include <OSGConvert/OSGNode.h>
#include <OSGConvert/B3DMTile.h>

#include <osg/Group>

namespace scially {

	class OSGVirtualTile : 
		public OSGIndexNode, 
		public QEnableSharedFromThis<OSGVirtualTile>
	{
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
			mFileName = "Top";
			mTileName = QString("Top_%1_%2_%3").arg(mZIndex).arg(mXIndex).arg(mYIndex);
		}

		virtual ~OSGVirtualTile() = default;

		const osg::Node* osgNode() const {
			return mOSGNode;
		}
		osg::ref_ptr<osg::Node>& osgNode() {
			return mOSGNode;
		}
	private:
		osg::ref_ptr<osg::Node> mOSGNode;
	};
}
