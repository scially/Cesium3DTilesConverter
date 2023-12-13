#pragma once

#include <OSGConvert/OSGNode.h>
#include <Cesium3DTiles/BaseTile.h>
#include <CesiumReadWrite/BaseTileReadWriter.h>

#include <osg/BoundingBox>
#include <osg/Matrixd>

namespace scially {
	class B3DMTile : public OSGIndexNode, public QEnableSharedFromThis<B3DMTile> {
	public:
		using Ptr = QSharedPointer<B3DMTile>;

		static BaseTile toBaseTile(
			const QPointerList<OSGIndexNode> &b3dms,
			const osg::Matrixd& transform)
		{
			osg::BoundingBoxd mergeBoundingBox;
			RootTile root;

			for (const auto& b3dm: b3dms) {
				mergeBoundingBox.expandBy(b3dm->boundingBox());

				RootTile r = b3dm->toRootTile(false);
				root.children.append(r);
			}

			root.boundingVolume.box = osgBoundingToCesiumBoundBox(mergeBoundingBox);
			root.geometricError = osgBoundingSize(mergeBoundingBox);
			root.refine = "REPLACE";
			root.transform = osgMatrixToCesiumTransform(transform);

			BaseTile b;
			b.root = root;
			b.geometricError = root.geometricError;
			return b;
		}

		// inherit OSGNode
		// property
		virtual QString name() const override {
			return "B3DMTile";
		}
		// end inherit

		// class OSGTile
		B3DMTile() = default;
		virtual ~B3DMTile() = default;
		
		bool isToTileset() const {
			return mIsToTileset;
		}
		bool& isToTileset() {
			return mIsToTileset;
		}

	private:
		bool mIsToTileset = false;
	};
}
