#include <OSGConvert/OSGIndex.h>
#include <OSGConvert/OSGLodVisitor.h>
#include <OSGConvert/OSGParseVisitor.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <CesiumReadWrite/RootTileReadWriter.h>
#include <CesiumReadWrite/BoundingVolumeReadWriter.h>
#include <Commons/TileStorage.h>
#include <Commons/OSGUtil.h>

#include <osgDB/ReadFile>
#include <QDir>
#include <QtDebug>
#include <algorithm>

namespace scially {
	
	QList<TileNode::Ptr> TileNode::firstSplitedChild() {
		QList<TileNode::Ptr> splitNodes;
		if (nodes.isEmpty()) {
			return splitNodes;
		}

		if (nodes.length() == 1) {
			return nodes[0]->firstSplitedChild();
		}

        foreach (const auto& nodes, nodes) {
			splitNodes.append(nodes);
		}

		return splitNodes;
	}
	
	QList<TileNode::Ptr>
		TileNode::collectChildrenMatchGeometricError(double geometricError) {
		
		QList<TileNode::Ptr> matchNodes;

		if (this->geometricError <= 0 || nodes.isEmpty()) {
			matchNodes.append(sharedFromThis());
			return matchNodes;
		}

		double maxAllNodesGeometricError = 0;
		
        foreach (const auto& node, nodes) {
			if (node->geometricError > maxAllNodesGeometricError) {
				maxAllNodesGeometricError = node->geometricError;
			}
		}

		if (std::abs(geometricError - this->geometricError) 
			< std::abs(geometricError - maxAllNodesGeometricError)) {
			matchNodes.append(sharedFromThis());
			return matchNodes;
		}

        foreach (auto node, nodes) {
			auto childMatchNodes = node->collectChildrenMatchGeometricError(geometricError);
			matchNodes.append(childMatchNodes);
		}

		return matchNodes;
	}

	TileNode::Ptr buildOSGTileNodeTree(const QString& tileFolder, const QString& fileName, double minGeometricError) {
		QString rootFile = tileFolder + "/" + fileName + ".osgb";
		auto rootNode = osgDB::readRefNodeFile(rootFile.toStdString());
		if (!rootNode) {
			qCritical() << "osg load file failed: " << rootFile;
			return nullptr;
		}

		OSGLodVisitor lodVisitor;
		rootNode->accept(lodVisitor);

		if (!lodVisitor.isValid()) {
			qCritical() << "osg bounding box is not valid:" << rootFile;
			return nullptr;
		}

		QString tileName = QDir(tileFolder).dirName();
		TileNode::Ptr tileNode(new TileNode(QDir(tileFolder).dirName(), fileName));
		tileNode->boundingBox = lodVisitor.boundingBox;
		tileNode->vertexCount = lodVisitor.vertexCount;

		double sumPixel = 0;
        foreach (const auto& node, lodVisitor.children) {
			sumPixel += node.maxPixel;
		}
		if (sumPixel == 0) {
			tileNode->geometricError = 0;
		}
		else {
			tileNode->geometricError = osgBoundingSize(tileNode->boundingBox) / sumPixel;
		}

		if (minGeometricError > 0 && tileNode->geometricError < minGeometricError) {
			lodVisitor.children.clear();
		}

        foreach (const auto& node, lodVisitor.children) {
			for (const auto& tile : node.tileNames) {
				TileNode::Ptr leafNode = buildOSGTileNodeTree(
					tileFolder,
					tile,
					minGeometricError);

				if (leafNode) {
					tileNode->nodes.append(leafNode);
				}
			}
		}

		if (tileNode->nodes.isEmpty()) {
			tileNode->geometricError = 0;
		}
		else {
			if (lodVisitor.hasContentOutLOD) {
				TileNode::Ptr node (
					new TileNode(tileName, fileName)
				);
				node->boundingBox = lodVisitor.boundingBox;
				node->vertexCount = lodVisitor.vertexCount;
				node->textureCount = lodVisitor.texureCount;
				node->geometricError = 0;
				node->skipOSGLod = true;
				tileNode->nodes.append(node);
			}
		}

		return tileNode;
	}

	RootTile TileNode::toRootTile(bool withChilden) const {
		RootTile root;
		root.geometricError = geometricError * 16;
		root.refine = "REPLACE";
		root.boundingVolume.box = osgBoundingToCesiumBoundBox(boundingBox);

		// child node to 3dtiles content
		if (withChilden) {
			foreach(auto node, nodes) {
				RootTile r = node->toRootTile(withChilden);
				root.children.append(r);
			}
		}

		root.content.emplace();
		if (withChilden) {
			root.content.value().uri = mFileName + ".b3dm";
		}
		else {
			root.content.value().uri = mTileName + "/" +mFileName + ".json";
		}

		return root;
	}

	TileNode::Ptr OSGBToB3DM(
		const QString tileFolder,
		TileNode::Ptr osgb,
		const SpatialTransform& transform,
		TileStorage& storage,
		double splitPixel)
	{
		double geometricError = osgBoundingSize(osgb->boundingBox) / splitPixel;
		QList<TileNode::Ptr> osgIndexNodes =
			osgb->collectChildrenMatchGeometricError(geometricError);

		if (osgIndexNodes.isEmpty())
			return nullptr;

		osg::Vec3d tileCenter = transform.transform(osgb->boundingBox.center());

		CesiumB3DM::Ptr b3dm(new CesiumB3DM);
		b3dm->center = tileCenter;
		double maxGeometricError = 0;
        foreach (auto node, osgIndexNodes) {
			auto meshes = OSGBtoCesiumMesh(
				tileFolder + '/' + node->fileName(".osgb"),
				tileCenter, 
				transform);
			
			b3dm->meshes.append(meshes);
			maxGeometricError = std::max(maxGeometricError, node->geometricError);
		}

		if (b3dm->meshes.isEmpty()) {
			return nullptr;
		}

		QString outFileName = osgb->relativePath(".b3dm");

		QByteArray b3dmBuffer;
		if (!b3dm->toGltfBinaryWithNoPack(b3dmBuffer)) {
			qCritical() << "fialed convert meshes to b3dm buffer";
			return nullptr;
		}
		
		if (!storage.saveFile(outFileName, b3dmBuffer)) {
			return nullptr;
		}

		TileNode::Ptr b3dmTree(new TileNode(osgb->tileName(), osgb->fileName()));
		b3dmTree->geometricError = maxGeometricError;
		
		foreach (auto mesh, b3dm->meshes) {
            b3dmTree->boundingBox.expandBy(mesh.boundingBox());
		}

        // Attention!
		b3dmTree->boundingBox.xMin() += tileCenter.x();
		b3dmTree->boundingBox.yMin() += tileCenter.y();
		b3dmTree->boundingBox.zMin() += tileCenter.z();
		b3dmTree->boundingBox.xMax() += tileCenter.x();
		b3dmTree->boundingBox.yMax() += tileCenter.y();
		b3dmTree->boundingBox.zMax() += tileCenter.z();

		auto children = osgb->firstSplitedChild();
		if (children.isEmpty()) {
			children = osgb->nodes;
		}

        foreach (TileNode::Ptr childNode, children) {
			TileNode::Ptr node = OSGBToB3DM(
				tileFolder,
				childNode,
				transform,
				storage,
				splitPixel
			);
			if (node) {
				b3dmTree->nodes.append(node);
			}
		}
		return b3dmTree;
	}
}
