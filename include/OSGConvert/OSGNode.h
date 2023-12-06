#pragma once

#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/SpatialTransform.h>
#include <Commons/TileStorage.h>

#include <QString>
#include <QSharedPointer>

#include <osg/Node>

namespace scially {

	constexpr double SPLIT_PIXEL = 512;

	// for TileNode and TileNodeView, use Pointer as child node type?
	// Tile_+001_+001/Tile_+001_+001.osgb
	//   tileName      fileName     suffix
	class OSGNode {
	public:
		// property
		virtual QString name() { return "OSGNode"; }
		virtual QString fileName() const = 0;
		virtual QString tileName() const = 0;
		virtual QString filePath() const = 0;

		// child nodes
		virtual size_t size() const = 0;
		virtual OSGNode* node(size_t i) const = 0;
	};

	class OSGIndexNode: public OSGNode {
	public:
		
		virtual QString name() { return "OSGIndexNode"; }
		
		virtual int32_t xIndex() const = 0;
		virtual int32_t yIndex() const = 0;
		virtual int32_t zIndex() const = 0;

		virtual osg::BoundingBoxd boungdingBox() const = 0;
		virtual double geometricError() const {
			return 0;
		}

		// convert
		virtual QSharedPointer<OSGIndexNode> toB3DM(
			const SpatialTransform& transform, 
			const TileStorage& storage) const { 
			return nullptr;
		}

		QList<QSharedPointer<OSGIndexNode>> firstSplitedChild() {
			QList<QSharedPointer<OSGIndexNode>> splitNodes;
			
			if (0 == size()) {
				return splitNodes;
			}

			if (size() == 1) {
				return node(0)->firstSplitedChild();
			}

			for (size_t i = 0; i < size(); i++) {
				splitNodes.append(node(i));
			}

			return splitNodes;
		}

		QList<QSharedPointer<OSGIndexNode>> collectChildrenMatchGeometricError(double geometricError) {

			QList<QSharedPointer<OSGIndexNode>>  matchNodes;

			if (this->geometricError() <= 0 || size() == 0) {
				matchNodes.append(sharedFromThis());
				return matchNodes;
			}

			double maxAllNodesGeometricError = 0;

			foreach(const auto & node, nodes) {
				if (node->geometricError > maxAllNodesGeometricError) {
					maxAllNodesGeometricError = node->geometricError;
				}
			}

			if (std::abs(geometricError - this->geometricError)
				< std::abs(geometricError - maxAllNodesGeometricError)) {
				matchNodes.append(sharedFromThis());
				return matchNodes;
			}

			foreach(auto node, nodes) {
				auto childMatchNodes = node->collectChildrenMatchGeometricError(geometricError);
				matchNodes.append(childMatchNodes);
			}

			return matchNodes;
		}

	protected:
		osg::ref_ptr<osg::Node> mOSGNode;
	};


}