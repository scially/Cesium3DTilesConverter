#include <Commons/OSGSimplify.h>
#include <Commons/OSGUtil.h>
#include <Commons/TileStorageDisk.h>
#include <OSGConvert/OSGParseVisitor.h>
#include <OSGConvert/OSGVirtualTile.h>

#include <QMap>
#include <QQueue>
#include <QSet>
#include <QStack>
#include <QtDebug>

#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile>

#include <tuple>

namespace scially {

QPointerList<OSGIndexNode>
BuildPyramidIndex(const QPointerList<B3DMTile> &nodes, int32_t maxZ) {
    QQueue<QSharedPointer<OSGIndexNode>> nodeQueue;
    QPointerList<OSGIndexNode> topNodes;

    QMap<std::tuple<int32_t, int32_t, int32_t>, OSGVirtualTile::Ptr> buildCache;

    for (const auto &node : nodes) {
        node->zIndex() = maxZ;
        nodeQueue.enqueue(node);
    }

    for (int32_t i = maxZ; i > 0; --i) {
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
                    topNodes.append(parent);
                }
            }

            parent->append(n);
        }
  }

  return topNodes;
}

void OSGPyramidBuilder::GenerateOSGNodeInPyramid(
    const QList<QSharedPointer<OSGIndexNode>> &topNodes, int32_t maxZ) {

  if (maxZ == 1) {
    return;
  }

  GenerateOSGNodeInPyramidFromBottomToUp(topNodes, maxZ, 1);
}

// pyramid and update bound
//    t     z = 1 1 / pyramid = ratio
//  t   t   z = 2 1 / pyramid * 2 = ratio
// t  t t  t z = 3 1 / pyramid * 3 = ratio ( ratio = 1 )
void OSGPyramidBuilder::GenerateOSGNodeInPyramidFromBottomToUp(
    const QList<QSharedPointer<OSGIndexNode>> &topNodes, uint32_t maxZ,
    uint32_t z) {
  if (z == 0)
    return;

  if (z != maxZ) {
    for (const auto &topNode : topNodes) {
      GenerateOSGNodeInPyramidFromBottomToUp(topNode->nodes<OSGIndexNode>(),
                                             maxZ, z + 1);

      auto group = osg::dynamic_pointer_cast<osg::Group>(topNode->osgNode());

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
      simp.simplify(1.0 / maxZ);

      topNode->geometricError() = maxGeometricError * 2;
      topNode->boundingBox() = box;
    }
  }
}

QList<QSharedPointer<OSGIndexNode>>
OSGPyramidBuilder::MergeOSGToB3DM(const QList<OSGVirtualTile::Ptr> &topNodes,
                                  const SpatialTransform &transform,
                                  TileStorage &storage, double splitPixel) {

  QList<OSGVirtualTile::Ptr> b3dms;
  for (const auto &node : topNodes) {
    auto r = node->toB3DM(transform, storage);
    if (r) {
      b3dms.append(r);
    }
  }
  return b3dms;
}
} // namespace scially
