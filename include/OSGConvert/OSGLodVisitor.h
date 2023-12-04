#pragma once

#include <QList>
#include <QString>
#include <QVector>

#include <osg/BoundingBox>
#include <osg/NodeVisitor>

namespace scially {
	class OSGLodVisitor : public osg::NodeVisitor {
	public:
		struct OSGLodNode {
			// pagelod range
			int32_t minPixel = 0;
			int32_t maxPixel = 0;
			// pagelod radius
			double radius = 0;
			// chiled osgb file name
			QVector<QString> tileNames;
		};

		OSGLodVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {

		}

		bool isValid(float v) const;
		bool isValid(const osg::Vec3f& v) const;
		bool isValid() const { return boundingBox.valid(); }
		bool processPagedLod(osg::PagedLOD& lod);

		virtual void apply(osg::Node& node);
		virtual void apply(osg::Geode& geode);
		virtual void apply(osg::PagedLOD& node);
	
	public:
		QList<OSGLodNode> children;
		uint32_t vertexCount = 0;
		uint32_t texureCount = 0;
		bool hasContentOutLOD = false;
		osg::BoundingBoxd boundingBox;
	};
}