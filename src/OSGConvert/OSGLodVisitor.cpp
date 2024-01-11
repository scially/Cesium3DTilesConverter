#include <OSGConvert/OSGLodVisitor.h>

#include <string>
#include <QFileInfo>
#include <QtDebug>
#include <osg/Geode>
#include <osg/PagedLOD>
#include <osg/Node>
#include <osg/Geometry>

namespace scially {
	bool OSGLodVisitor::isValid(float v) const {
		return v > -100000000 && v < 100000000;
	}

	bool OSGLodVisitor::isValid(const osg::Vec3f& v) const {
		return isValid(v.x()) && isValid(v.y()) && isValid(v.z());
	}

	void OSGLodVisitor::apply(osg::Node& node) {
		traverse(node);
	}

	void OSGLodVisitor::apply(osg::Geode& geode) {
		std::string parentClassName;
		if (geode.getNumParents() > 0)
			parentClassName = geode.getParent(0)->className();

		if (parentClassName != "PagedLOD") {
			hasContentOutLOD = true;
		}

		for (uint32_t i = 0; i < geode.getNumDrawables(); i++) {
			osg::Drawable* draw = geode.getDrawable(i);
			if (draw == nullptr)
				continue;

			osg::Geometry* geom = draw->asGeometry();
			if (geom == nullptr)
				continue;

			osg::MatrixList mats = geom->getWorldMatrices();
			// from node coordinate system to world coordinate system
			osg::Matrixd mat = mats.empty() ? osg::Matrix::identity() : mats[0];
			bool needTransform = !mat.isIdentity();

			osg::Vec3Array* vertex = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
			if (vertex == nullptr)
				continue;

			uint32_t vertexCount = vertex->getNumElements();
			if (vertexCount == 0)
				continue;

			this->vertexCount += vertexCount;

			for (auto v : *vertex) {
				if (!isValid(v)) {
					v = osg::Vec3f(0, 0, 0);
				}

				if (needTransform) {
					v = v * mat;
				}

				boundingBox.expandBy(v);
			}

			for (uint32_t i = 0; i < geom->getNumPrimitiveSets(); i++) {
				osg::PrimitiveSet* primitive = geom->getPrimitiveSet(i);
				switch (primitive->getMode()) {
				case GL_TRIANGLES:
					this->texureCount += primitive->getNumIndices() / 3;
					break;
				case GL_TRIANGLE_STRIP:
					this->texureCount += primitive->getNumIndices() - 2;
					break;
				}
			}
		}
	}

	void OSGLodVisitor::apply(osg::PagedLOD& node) {
		processPagedLod(node);
		apply(static_cast<osg::Node&>(node));
	}

	bool OSGLodVisitor::processPagedLod(osg::PagedLOD& lod) {
		auto ranges = lod.getRangeList();
		if (ranges.empty()) {
			qWarning("%s is empty rangelist", lod.getName().data());
			return false;
		}

		OSGLodNode child;
		child.minPixel = ranges[0].first;
		child.maxPixel = ranges[0].second;
		child.radius = lod.getRadius();

		for (uint32_t i = 0; i < lod.getNumFileNames(); i++) {
			QString fileName = QString::fromStdString(lod.getFileName(i));  // with extension
			if (fileName.isEmpty())
				continue;

			child.tileNames.append(QFileInfo(fileName).baseName());
			children.append(child);
		}
		return true;
	}
}