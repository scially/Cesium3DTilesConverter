#pragma once

#include <osg/Node>
#include <osg/Group>

namespace scially {
	class OSGSimplify {
	public:
		OSGSimplify(osg::Node& node) : mNode(node), mSNode(new osg::Group) {

		}

		bool simplify(double ratio);

		osg::ref_ptr<osg::Node> node() {
			return mSNode;
		}

	private:
		osg::ref_ptr<osg::Group> mSNode;
		osg::Node& mNode;
	};
}
