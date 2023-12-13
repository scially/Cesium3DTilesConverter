#include <OSGConvert/OSGNode.h>

#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <OSGConvert/B3DMTile.h>
#include <OSGConvert/OSGParseVisitor.h>

namespace scially {
	
	bool OSGIndexNode::parentIndex(uint32_t z, int32_t& x, int32_t& y) const {
		if (z >= mZIndex) {
			qWarning() << "try to new level zoom";
			return false;
		}

		x = mXIndex >> (mZIndex - z);
		y = mYIndex >> (mZIndex - z);

		return true;
	}

	RootTile OSGIndexNode::toRootTile(bool withChilden) const {
		RootTile root;
		root.geometricError = geometricError() * 16;
		root.refine = "REPLACE";
		root.boundingVolume.box = osgBoundingToCesiumBoundBox(boundingBox());

		// child node to 3dtiles content
		if (withChilden) {
			for (size_t i = 0; i < size(); i++) {
				RootTile r = node<OSGIndexNode>(i)->toRootTile(withChilden);
				root.children.append(r);
			}
		}

		root.content.emplace();
		if (withChilden) {
			root.content.value().uri = tileName() + ".b3dm";
		}
		else {
			root.content.value().uri = fileName() + "/" + tileName() + ".json";
		}

		return root;
	}

	bool OSGIndexNode::saveJson(const TileStorage& storage, const osg::Matrixd& transform) const {
		RootTile r = toRootTile(true);
		BaseTile b;
		b.root = r;
		b.geometricError = r.geometricError;

		BaseTileReadWriter brw;

		QString outTilesetName = relativeNodePath(".json");
		if (!storage.saveJson(outTilesetName, brw.writeToJson(b))) {
			qCritical() << "save json" << outTilesetName << "failed";
			return false;
		}

		// for debug
		r.transform = osgMatrixToCesiumTransform(transform);
		b.root = r;
		b.geometricError = osgBoundingSize(boundingBox());
		outTilesetName = tileName() + "/tileset.json";
		if (!storage.saveJson(outTilesetName, brw.writeToJson(b))) {
			qCritical() << "save json " << outTilesetName << "failed";
			return false;
		}
		return true;
	}

	QSharedPointer<OSGIndexNode> OSGIndexNode::toB3DM(
		const SpatialTransform& transform,
		const TileStorage& storage) {

		if (isSameKindAs<B3DMTile>(this))
			return sharedFromThis();

		double geometricError = osgBoundingSize(mBoundingBox) / SPLIT_PIXEL;

		// first foreach from self maybe or some child
		auto osgIndexNodes =
			collectChildrenMatchGeometricError<OSGIndexNode>(geometricError);

		if (osgIndexNodes.isEmpty())
			return nullptr;

		osg::Vec3d tileCenter = transform.transform(mBoundingBox.center());
		QList<CesiumMesh> tileMeshes;

		double maxGeometricError = 0;

		for (const auto& node : osgIndexNodes) {
			if (node->mOSGNode == nullptr)
				continue;

			auto meshes = OSGBtoCesiumMesh(
				*node->mOSGNode,
				tileCenter,
				transform);

			tileMeshes.append(meshes);
			maxGeometricError = std::max(maxGeometricError, node->geometricError());
		}

		if (tileMeshes.isEmpty()) {
			return nullptr;
		}

		auto b3dmTopNode = QSharedPointer<B3DMTile>::create();
		b3dmTopNode->tileName() = tileName();
		b3dmTopNode->fileName() = fileName();
		b3dmTopNode->xIndex() = xIndex();
		b3dmTopNode->yIndex() = yIndex();
		b3dmTopNode->zIndex() = zIndex();
		b3dmTopNode->geometricError() = maxGeometricError;
		b3dmTopNode->osgNode() = osgNode();
		QString outFileName = b3dmTopNode->relativeNodePath(".b3dm");

		QByteArray b3dmBuffer = CesiumMesh::toGltfBinaryWithNoPack(tileMeshes, tileCenter);
		if (b3dmBuffer.isEmpty()) {
			qCritical() << "fialed convert meshes to b3dm buffer";
			return nullptr;
		}

		if (!storage.saveFile(outFileName, b3dmBuffer)) {
			return nullptr;
		}


		for (const auto& mesh : tileMeshes) {
			b3dmTopNode->boundingBox().expandBy(mesh.boundingBox());
		}

		// Attention
		b3dmTopNode->boundingBox().xMin() += tileCenter.x();
		b3dmTopNode->boundingBox().yMin() += tileCenter.y();
		b3dmTopNode->boundingBox().zMin() += tileCenter.z();
		b3dmTopNode->boundingBox().xMax() += tileCenter.x();
		b3dmTopNode->boundingBox().yMax() += tileCenter.y();
		b3dmTopNode->boundingBox().zMax() += tileCenter.z();

		auto children = firstSplitedChild<OSGIndexNode>();
		if (children.isEmpty()) {
			children = nodes<OSGIndexNode>();
		}

		for (const auto& cn : children) {
			auto n = cn->toB3DM(transform, storage);
			if (n) {
				b3dmTopNode->append(n);
			}
		}
		return b3dmTopNode;
	}
}
