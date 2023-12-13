#include <OSGConvert/OSGVirtualFolder.h>
#include <OSGConvert/OSGVirtualTile.h>
#include <Commons/OSGSimplify.h>

#include <QQueue>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>

namespace scially
{
	OSGVirtualFolder::OSGVirtualFolder(
		QPointerList<OSGIndexNode>& nodes,
		const SpatialReference& outSrs,
		const SpatialTransform& transform,
		const TileStorage& storage)
		: mOutSrs(outSrs)
		, mSTS(transform)
		, mStorage(storage)
	{
		mFileName = "Top";

		int32_t minx = std::numeric_limits<int32_t>::max();
		int32_t miny = std::numeric_limits<int32_t>::max();

		int32_t maxx = std::numeric_limits<int32_t>::min();
		int32_t maxy = std::numeric_limits<int32_t>::min();

		for (const auto& node : nodes) {
			nodes.push_back(node);

			minx = std::min(minx, node->xIndex());
			miny = std::min(miny, node->yIndex());

			maxx = std::max(maxx, node->xIndex());
			maxy = std::max(maxy, node->yIndex());
		}

		int32_t maxIndex = std::max({ maxx - minx, maxy - miny });

		if (maxIndex <= 0) {
			return;
		}

		mMaxZ = static_cast<int32_t>(std::log2(maxIndex) + 1) + 1; // z start from 1

		buildPyramidIndex(nodes);
	}

	void OSGVirtualFolder::buildPyramidIndex(const QPointerList<OSGIndexNode>& nodes)
	{
		QQueue<QSharedPointer<OSGIndexNode>> nodeQueue;

		QMap<std::tuple<int32_t, int32_t, int32_t>, OSGVirtualTile::Ptr> buildCache;

		for (const auto& node : nodes) {
			node->zIndex() = mMaxZ;
			nodeQueue.enqueue(node);
		}

		for (int32_t i = mMaxZ; i > 0; --i) {
			size_t s = nodeQueue.size();
			for (size_t j = 0; j < s; ++j) {
				auto n = nodeQueue.dequeue();

				int32_t x, y;
				if (!n->parentIndex(i - 1, x, y)) {
					continue;
				}

				auto parent = buildCache.value(std::make_tuple(x, y, i - 1), nullptr);

				if (parent == nullptr) {
					parent = OSGVirtualTile::Ptr::create(x, y, i - 1);
					nodeQueue.enqueue(parent);
					buildCache[std::make_tuple(x, y, i - 1)] = parent;

					if (i == 2) {
						mNodes.append(parent);
					}
				}

				parent->append(n);
			}
		}
	}

	QPointerList<OSGIndexNode> OSGVirtualFolder::to3DTiles(uint32_t thread) {
		if (mMaxZ == 1) {
			return {};
		}

		generateOSGNodeInPyramid(nodes<OSGIndexNode>());

		QThreadPool threadPool;
		if (thread != 0) {
			threadPool.setMaxThreadCount(thread);
		}

		QList<QFuture<bool>> workFutures;
		QPointerList<OSGIndexNode> b3dms;

		for (size_t i = 0; i < size(); i++) {
			auto dyn = node<OSGIndexNode>(i);
			QFuture<bool> f = QtConcurrent::run(&threadPool, [this, &dyn, &b3dms]() {
				auto r = dyn->toB3DM(mSTS, mStorage);
				if (r && r->saveJson(mStorage, mOutSrs.originENU())) {
					b3dms.append(r);
					return true;
				}
				return false;
			});
			workFutures.append(f);
		}

		for (auto& worker : workFutures) {
			worker.waitForFinished();
		}

		BaseTile b = B3DMTile::toBaseTile(b3dms, mOutSrs.originENU());
		BaseTileReadWriter brw;

		if (!mStorage.saveJson("tileset.json", brw.writeToJson(b))) {
			qCritical() << "write tileset.json failed";
		}

		return b3dms;
	}

	void OSGVirtualFolder::generateOSGNodeInPyramid(
		const QPointerList<OSGIndexNode>& nodes,
		uint32_t z)
	{
		if (z == 0 || z == mMaxZ)
			return;

		for (const auto& topNode : nodes) {
			generateOSGNodeInPyramid(topNode->nodes<OSGIndexNode>(), z + 1);

			osg::ref_ptr<osg::Group> group = new osg::Group();

			double maxGeometricError = 0;
			osg::BoundingBoxd box;
			for (size_t i = 0; i < topNode->size(); i++) {
				auto dyn = topNode->node<OSGIndexNode>(i);
				if (dyn && dyn->osgNode()) {
					group->addChild(dyn->osgNode());
				}
				maxGeometricError = std::max(maxGeometricError, dyn->geometricError());
				box.expandBy(dyn->boundingBox());
			}

			// simplify
			OSGSimplify simp(*group);
			simp.simplify(1.0 / mMaxZ);

			topNode->osgNode() = group;
			topNode->geometricError() = maxGeometricError * 2;
			topNode->boundingBox() = box;
		}

	}
}