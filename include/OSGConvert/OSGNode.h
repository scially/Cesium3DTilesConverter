#pragma once

#include <Cesium3DTiles/RootTile.h>
#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/OSGUtil.h>
#include <Commons/TileStorage.h>
#include <Cesium3DTiles/BaseTile.h>

#include <QList>
#include <QSharedPointer>
#include <QString>

#include <osg/Node>

namespace scially {
    template <typename T>
    using QPointerList = QList<QSharedPointer<T>>;

	#if __cplusplus == 202002L
		using remove_cvref_t = std::remove_cvref_t;
	#else
		template<class T>
		struct remove_cvref
		{
			typedef std::remove_cv_t<std::remove_reference_t<T>> type;
		};
		template< class T >
		using remove_cvref_t = typename remove_cvref<T>::type;
	#endif

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

		template <typename T>
		bool isSameKindAs(const OSGNode* obj) const {
			return dynamic_cast<const remove_cvref_t<T>*>(obj) != nullptr;
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

		QString relativeNodePath(const QString& suffix) const {
			return fileName() + "/" + tileName() + suffix;
		}

		QString absoluteNodePath(const QString& suffix) const {
			return tileFolder() 
				+ "/" 
				+ relativeNodePath(suffix);
		}

		QString absoluteNodePath(const QString& folder, const QString& suffix) const {
			return folder
				+ "/"
				+ relativeNodePath(suffix);
		}

		QString absolutePath() const {
			return tileFolder() + "/" + fileName();
		}

		QString relativePath() const {
			return fileName();
		}

		size_t size() const {
			return mNodes.size();
		}

        template <typename T>
		QSharedPointer<T> node(size_t i) const {
			return mNodes[i].dynamicCast<T>();
		}

        template <typename T>
		QPointerList<T> nodes() const {
			QPointerList<T> ns;
			for (const auto& node : mNodes) {
				ns.append(node.dynamicCast<T>());
			}
			return ns;
		}

		void append(const QSharedPointer<OSGNode>& node) {
			mNodes.append(node);
		}

	protected:
		QPointerList<OSGNode> mNodes;

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

        virtual bool parentIndex(uint32_t z, int32_t& x, int32_t& y) const;

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

		osg::ref_ptr<osg::Node>& osgNode() {
			return mOSGNode;
		}

		// convert
		QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& transform,
			const TileStorage& storage);

		template <typename T = OSGIndexNode>
		QList<QSharedPointer<T>> firstSplitedChild() {
			QList<QSharedPointer<T>> splitNodes;
			
			if (0 == size()) {
				return splitNodes;
			}

			if (size() == 1) {
                return node<OSGIndexNode>(0)->firstSplitedChild<T>();
			}

			for (size_t i = 0; i < size(); i++) {
				splitNodes.append(node<T>(i));
			}

			return splitNodes;
		}

		template <typename T = OSGIndexNode >
		QList<QSharedPointer<T>> collectChildrenMatchGeometricError(double geometricError) {

			QList<QSharedPointer<T>>  matchNodes;

			if (this->geometricError() <= 0 || size() == 0) {
				matchNodes.append(sharedFromThis().dynamicCast<T>());
				return matchNodes;
			}

			double maxAllNodesGeometricError = 0;

			for (size_t i = 0; i < size(); i++) {
                if (node<OSGIndexNode>(i)->geometricError() > maxAllNodesGeometricError) {
                    maxAllNodesGeometricError = node<OSGIndexNode>(i)->geometricError();
				}
			}

			if (std::abs(geometricError - this->geometricError())
				< std::abs(geometricError - maxAllNodesGeometricError)) {
				matchNodes.append(sharedFromThis().dynamicCast<T>());
				return matchNodes;
			}

			for (size_t i = 0; i < size(); i++) {
                auto childMatchNodes = node<OSGIndexNode>(i)->collectChildrenMatchGeometricError<T>(geometricError);
				matchNodes.append(childMatchNodes);
			}

			return matchNodes;
		}

		// base path with tileset.json
		RootTile toRootTile(bool withChilden) const;

		bool saveJson(const TileStorage& storage, const osg::Matrixd& transform) const;

		virtual bool operator== (const OSGIndexNode& node) {
			return mXIndex == node.mXIndex
				&& mYIndex == node.mYIndex
				&& mZIndex == node.mZIndex;
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
