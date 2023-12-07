#pragma once

#include <Cesium3DTiles/RootTile.h>
#include <CesiumReadWrite/BaseTileReadWriter.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/OSGUtil.h>
#include <Commons/TileStorage.h>

#include <QSharedPointer>
#include <QString>
#include <QList>

#include <osg/Node>

namespace scially {

	constexpr double SPLIT_PIXEL = 512;

	// for TileNode and TileNodeView, use Pointer as child node type?
	// Tile_+001_+001/Tile_+001_+001.osgb
	//   tileName        fileName    suffix
	class OSGNode {
	public:
		// property
		virtual QString name() const { 
			return "OSGNode"; 
		}

		QString fileName() const {
			return mFileName;
		}

		QString& fileName() {
			return mFileName;
		}

		QString tileName() const {
			return mTileName;
		}

		QString& tileName() {
			return mTileName;
		}

		QString tileFolder() const {
			return mTileFolder;
		}

		QString& tileFolder() {
			return mTileFolder;
		}

		virtual QString relativePath(const QString& suffix) const {
			return tileName() + "/" + fileName() + suffix;
		}

		QString absolutePath(const QString& suffix) const {
			return tileFolder() 
				+ "/" 
				+ relativePath(suffix);
		}

		QString absolutePath(const QString& folder, const QString& suffix) const {
			return folder
				+ "/"
				+ relativePath(suffix);
		}

		size_t size() const {
			return mNodes.size();
		}

		template <typename T>
		QSharedPointer<T> node(size_t i) const {
			return mNodes[i].dynamicCast<T>();
		}

		template <typename T>
		QList<QSharedPointer<T>> nodes() const {
			QList<QSharedPointer<T>> ns;
			for (const auto& node : mNodes) {
				ns.append(node.dynamicCast<T>());
			}
			return ns;
		}

		void append(const QSharedPointer<OSGNode>& node) {
			mNodes.append(node);
		}

	protected:
		QList<QSharedPointer<OSGNode>> mNodes;

		QString mTileFolder;
		QString mTileName;
		QString mFileName;
	};

	class OSGIndexNode: public OSGNode, public QEnableSharedFromThis<OSGIndexNode> {
	public:
		// child nodes
		virtual QString name() const override { 
			return "OSGIndexNode"; 
		}
	
		int32_t xIndex() const {
			return mXIndex;
		}

		int32_t yIndex() const {
			return mYIndex;
		}

		int32_t zIndex() const {
			return mZIndex;
		}

		int32_t& xIndex() {
			return mXIndex;
		}

		int32_t& yIndex() {
			return mYIndex;
		}

		int32_t& zIndex() {
			return mZIndex;
		}

		osg::ref_ptr<osg::Node> osgNode() const {
			return mOSGNode;
		}

		osg::BoundingBoxd boundingBox() const {
			return mBoundingBox;
		}

		double geometricError() const {
			return mGeometricError;
		}

		osg::BoundingBoxd& boundingBox() {
			return mBoundingBox;
		}

		double& geometricError() {
			return mGeometricError;
		}

		// convert
		virtual QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& transform, 
			const TileStorage& storage) { 
			return nullptr;
		}

		template <typename T>
		QList<QSharedPointer<T>> firstSplitedChild() {
			QList<QSharedPointer<T>> splitNodes;
			
			if (0 == size()) {
				return splitNodes;
			}

			if (size() == 1) {
				return node(0)->firstSplitedChild<T>();
			}

			for (size_t i = 0; i < size(); i++) {
				splitNodes.append(node<T>(i));
			}

			return splitNodes;
		}

		template <typename T>
		QList<QSharedPointer<T>> collectChildrenMatchGeometricError(double geometricError) {

			QList<QSharedPointer<T>>  matchNodes;

			if (this->geometricError() <= 0 || size() == 0) {
				matchNodes.append(sharedFromThis().dynamicCast<T>());
				return matchNodes;
			}

			double maxAllNodesGeometricError = 0;

			for (size_t i = 0; i < size(); i++) {
				if (node(i)->geometricError() > maxAllNodesGeometricError) {
					maxAllNodesGeometricError = node(i)->geometricError();
				}
			}

			if (std::abs(geometricError - this->geometricError())
				< std::abs(geometricError - maxAllNodesGeometricError)) {
				matchNodes.append(sharedFromThis().dynamicCast<T>());
				return matchNodes;
			}

			for (size_t i = 0; i < size(); i++) {
				auto childMatchNodes = node(i)->collectChildrenMatchGeometricError<T>(geometricError);
				matchNodes.append(childMatchNodes);
			}

			return matchNodes;
		}

		RootTile toRootTile(bool withChilden) const {
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
				root.content.value().uri = fileName() + ".b3dm";
			}
			else {
				root.content.value().uri = tileName() + "/" + fileName() + ".json";
			}

			return root;
		}

		bool saveJson(const TileStorage& storage, const osg::Matrixd& transform) const {
			RootTile r = toRootTile(true);
			BaseTile b;
			b.root = r;
			b.geometricError = r.geometricError;

			BaseTileReadWriter brw;

			QString outTilesetName = tileName() + '/' + fileName() + ".json";
			if (!storage.saveJson(outTilesetName, brw.writeToJson(b))) {
				qCritical() << "save json " << outTilesetName << "failed";
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

	protected:
		osg::ref_ptr<osg::Node> mOSGNode;

		int32_t mXIndex = -1;
		int32_t mYIndex = -1;
		int32_t mZIndex = 1;

		osg::BoundingBoxd mBoundingBox;
		double mGeometricError = 0;
	};
}