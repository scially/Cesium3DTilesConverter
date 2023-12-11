#pragma once

#include <OSGConvert/OSGNode.h>

namespace scially {
	class OSGVirtualFolder : public OSGNode {
	public:
		OSGVirtualFolder(QPointerList<OSGIndexNode>& nodes, int32_t maxZ)
		: mMaxZ(maxZ)
		{
			mTileName = "top";
			buildPyramidIndex();
		}

		// convert to b3dm
		QList<QSharedPointer<OSGIndexNode>> MergeOSGToB3DM(
			const QList<QSharedPointer<OSGIndexNode>>& topNodes,
			const SpatialTransform& transform,
			const TileStorage& storage);

	private:
		void buildPyramidIndex();

		GenerateOSGNodeInPyramid(
			const QList<QSharedPointer<OSGIndexNode>>& topNodes,
			int32_t maxZ);

		// read and simplyf osg node
		 void
			GenerateOSGNodeInPyramidFromBottomToUp(
				const QPointerList<OSGIndexNode>& topNodes,
				uint32_t maxZ,
				uint32_t z);

		 int32_t mMaxZ;
	};
}