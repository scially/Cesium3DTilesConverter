#include <Commons/OSGSimplify.h>
#include <Commons/OSGUtil.h>
#include <Commons/TileStorageDisk.h>
#include <OSGConvert/OSGMergeTopIndex.h>
#include <OSGConvert/OSGParseVisitor.h>

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
	
	MergeTileNode::MergeTileNode(OSGTile::Ptr osgTile): mOSGTile(osgTile) {
		Q_ASSERT(osgTile != nullptr);

		mXIndex = osgTile->xIndex();
		mYIndex = osgTile->yIndex();
		mZIndex = osgTile->zIndex();

		mFileName = QString("Top_%1").arg(mZIndex);
		mTileName = QString("Top_%1_%2").arg(mXIndex).arg(mYIndex);

		if (mOSGTile) {
			mBoundingBox = mOSGTile->mB3DMIndexNode->boundingBox;
			mGeometricError = mOSGTile->mB3DMIndexNode->geometricError;
		}
	}

	MergeTileNode::MergeTileNode(int32_t x, int32_t y, int32_t z)
		:mXIndex(x), mYIndex(y), mZIndex(z){
		mFileName = QString("Top_%1").arg(mZIndex);
		mTileName = QString("%1_%2").arg(mXIndex).arg(mYIndex);
	}

	bool MergeTileNode::operator== (const MergeTileNode& node) {
		return mXIndex == node.mXIndex
			&& mYIndex == node.mYIndex
			&& mZIndex == node.mZIndex;
	}

	
	bool MergeTileNode::parentIndex(uint32_t z, int32_t& x, int32_t& y) const {
		if (z >= mZIndex) {
			qWarning() << "try to new level zoom";
			return false;
		}
			
		x = mXIndex >> (mZIndex - z);
		y = mYIndex >> (mZIndex - z);

		return true;
	}

	MergeTileNode::Ptr MergeTileNode::toB3DM(
		const SpatialTransform& transform,
		const TileStorage& storage,
		double splitPixel) {

		if (mOSGTile != nullptr) {
			MergeTileNode::Ptr leafNode{
				new MergeTileNode(mOSGTile)
			};

			return leafNode;
		}

		osg::Vec3d tileCenter = transform.transform(mBoundingBox.center());

		CesiumB3DM::Ptr b3dm{ new CesiumB3DM };
		b3dm->center = tileCenter;

		double maxGeometricError = 0;

		auto meshes = OSGBtoCesiumMesh(
			*mOSGNode,
			tileCenter,
			transform);

		b3dm->meshes.append(meshes);
		maxGeometricError = std::max(maxGeometricError, geometricError());

		if (b3dm->meshes.isEmpty()) {
			return nullptr;
		}

		QString outFileName = relativePath(".b3dm");

		QByteArray b3dmBuffer;
		if (!b3dm->toGltfBinaryWithNoPack(b3dmBuffer)) {
			qCritical() << "fialed convert meshes to b3dm buffer";
			return nullptr;
		}

		if (!storage.saveFile(outFileName, b3dmBuffer)) {
			return nullptr;
		}

		MergeTileNode::Ptr b3dmTree{
			new MergeTileNode(mXIndex, mYIndex, mZIndex)
		};

		b3dmTree->mGeometricError = maxGeometricError;
		for (const auto& mesh : b3dm->meshes) {
			b3dmTree->mBoundingBox.expandBy(mesh.boundingBox());
		}

		// Attention!
		b3dmTree->mBoundingBox.xMin() += tileCenter.x();
		b3dmTree->mBoundingBox.yMin() += tileCenter.y();
		b3dmTree->mBoundingBox.zMin() += tileCenter.z();
		b3dmTree->mBoundingBox.xMax() += tileCenter.x();
		b3dmTree->mBoundingBox.yMax() += tileCenter.y();
		b3dmTree->mBoundingBox.zMax() += tileCenter.z();

		for (const auto& childNode : mNodes) {
			MergeTileNode::Ptr node = childNode->toB3DM(
				transform,
				storage,
				splitPixel
			);

			if (node) {
				b3dmTree->mNodes.append(node);
			}
		}

		return b3dmTree;
	}

	RootTile MergeTileNode::toRootTile() const {
		RootTile root;
		root.geometricError = mGeometricError * 16;
		root.refine = "REPLACE";
		root.boundingVolume.box = osgBoundingToCesiumBoundBox(mBoundingBox);

		// child node to 3dtiles content
		for(const auto& node: mNodes) {
			RootTile r = node->toRootTile();
			root.children.append(r);
		}
		
		root.content.emplace();
		// per tile tieset.json
		if (mOSGTile) {
			root.content.value().uri = mOSGTile->mOSGIndexNode->relativePath(".json");
		}
		else {
			root.content.value().uri = relativePath(".b3dm");
		}

		return root;
	}

	QList<MergeTileNode::Ptr> MergeTileNodeBuilder::BuildPyramidIndex(const QList<MergeTileNode::Ptr>& nodes, int32_t maxZ) {
		QQueue<MergeTileNode::Ptr> nodeQueue;
		QList<MergeTileNode::Ptr> topNodes;

		QMap<std::tuple<int32_t, int32_t, int32_t>, MergeTileNode::Ptr> buildCache;

		for (auto node : nodes) {
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
				
				auto parent = buildCache.value(std::make_tuple( x, y, i - 1 ), nullptr);
				
				if (parent == nullptr) {
					parent = MergeTileNode::Ptr::create(x, y, i - 1);
					nodeQueue.enqueue(parent);
					buildCache[std::make_tuple(x, y, i - 1)] = parent;

					if (i == 2) {
						topNodes.append(parent);
					}
				}

				parent->mNodes.append(n);
			}
		}

		return topNodes;
	}

	void MergeTileNodeBuilder::GenerateOSGNodeInPyramid(const QList<MergeTileNode::Ptr>& topNodes, int32_t maxZ) {
		
		if (maxZ == 1) {
			return;
		}

		GenerateOSGNodeInPyramidFromBottomToUp(topNodes, maxZ, 1);
	}

	// pyramid and update bound
	//    t     z = 1 1 / pyramid = ratio
	//  t   t   z = 2 1 / pyramid * 2 = ratio
	//t  t t  t z = 3 1 / pyramid * 3 = ratio ( ratio = 1 )
	void MergeTileNodeBuilder::GenerateOSGNodeInPyramidFromBottomToUp(
		const QList<MergeTileNode::Ptr>& nodes,
		uint32_t maxZ, 
		uint32_t z) {
		if (z == 0)
			return;

		if (z == maxZ) {
			for(const auto& node: nodes) {
				QString osgPath = node->mOSGTile->rootTileFilePath();
				auto osgNode = osgDB::readRefNodeFile(osgPath.toStdString());

				if (osgNode) {
					node->mOSGNode = osgNode;
				}
				else {
					qWarning() << "read" << node->mOSGTile->rootTileFilePath() << "failed";
				}
			}
		}else {
			for(const auto& node: nodes) {
				GenerateOSGNodeInPyramidFromBottomToUp(node->mNodes, maxZ, z + 1);

				osg::ref_ptr<osg::Group> group = new osg::Group;

				double maxGeometricError = 0;
				osg::BoundingBoxd box;
				for(const auto& n: node->mNodes) {
					if (n->mOSGNode) {
						group->addChild(n->mOSGNode);
					}
					maxGeometricError = std::max(maxGeometricError, n->mGeometricError);
					box.expandBy(n->mBoundingBox);
				}

				// simplify
				OSGSimplify simp(*group);
				simp.simplify(1.0 / maxZ);

				node->mOSGNode = group;
				node->mGeometricError = maxGeometricError * 2;
				node->mBoundingBox = box;
			}
		}
	}

	QList<MergeTileNode::Ptr> MergeTileNodeBuilder::MergeOSGToB3DM(
		const QList<MergeTileNode::Ptr>& topNodes,
		const SpatialTransform& transform,
		TileStorage& storage,
		double splitPixel) {

		QList<MergeTileNode::Ptr> b3dms;
		for (const auto& node : topNodes) {
			auto r = node->toB3DM(transform, storage, splitPixel);
			if (r) {
				b3dms.append(r);
			}
		}
		return b3dms;
	}
}

//else if(z == maxZ - 1) {
//	foreach(auto node, nodes) {
//		osg::ref_ptr<osg::Group> group = new osg::Group;

//		foreach(auto n, node->nodes) {
//			auto dyn = n.dynamicCast<MergeTileNode>();
//			Q_ASSERT(dyn);		

//			if (dyn->mOSGNode) {
//				group->addChild(dyn->mOSGNode);
//			}
//		}

//		// simplify
//		OSGSimplify simp(*group);
//		simp.simplify(1.0 / maxZ);

//		node->mOSGNode = group;
//	}
//}