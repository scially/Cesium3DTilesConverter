#pragma once

#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>
#include <OSGConvert/OSGNode.h>

namespace scially {
	class OSGVirtualFolder : public OSGNode {
	public:
		OSGVirtualFolder(QPointerList<OSGIndexNode>& nodes, 
			const SpatialReference& outSrs,
			const SpatialTransform& transform, 
			const TileStorage& storage);

		// convert to b3dm
		QPointerList<OSGIndexNode> to3DTiles(uint32_t thread);

	private:
		// pyramid
		//    t     z = 1 (top from level 1)
		//  t   t   z = 2
		//t  t t  t z = 3 
		void buildPyramidIndex(const QPointerList<OSGIndexNode>& nodes);

		// pyramid and update bound
		//    t     z  = 1  1 / pyramid     = ratio
		//  t   t   z  = 2  1 / pyramid * 2 = ratio
		// t  t t  t z = 3  1 / pyramid * 3 = ratio ( ratio = 1 )
		void generateOSGNodeInPyramid(
			const QPointerList<OSGIndexNode> &nodes,
			uint32_t z = 1);

		int32_t mMaxZ;
		const SpatialReference& mOutSrs;
		const SpatialTransform& mSTS;
		const TileStorage& mStorage;
	};
}