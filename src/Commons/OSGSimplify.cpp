#include <Commons/OSGSimplify.h>

#include <osgUtil/Simplifier>
#include <osg/NodeVisitor>
#include <osg/PagedLOD>
#include <osg/Geode>
#include <osg/Image>
#include <osg/Texture2D>

namespace scially {
	class RemovePagedLod : public osg::NodeVisitor {
	public:
		RemovePagedLod(osg::ref_ptr<osg::Group> node) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {
			this->node = node;
		}

		virtual void apply(osg::Node& node) override {
			traverse(node);
		}

		virtual void apply(osg::PagedLOD& node) override {
			apply(static_cast<osg::Node&>(node));
		}

		virtual void apply(osg::Geode& geode) override {
			node->addChild(&geode);
		}

		osg::ref_ptr<osg::Group> node;
	};


	class OSGImageVisitor : public osg::NodeVisitor {
	public:
		OSGImageVisitor(double ratio) 
			: osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			, ratio(ratio){
			
		}

		virtual void apply(osg::Node& node) override {
			traverse(node);
		}


		virtual void apply(osg::Geode& geode) override {
			for (size_t i = 0; i < geode.getNumDrawables(); i++) {
				osg::StateSet* ss = geode.getDrawable(i)->getStateSet();
				if (ss == nullptr)
					continue;

				auto tt = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));

				if (tt == nullptr)
					continue;

				osg::Image* img = tt->getImage(0);

				if (img == nullptr)
					continue;

				img->scaleImage(img->s() * ratio, img->t() * ratio, img->r() * ratio);
			}

		}


		double ratio;
	};

	bool OSGSimplify::simplify(double ratio) {
		RemovePagedLod rpl(mSNode);
		mNode.accept(rpl);
		mSNode = rpl.node;

		if (mSNode == nullptr || mSNode->getNumChildren() == 0)
			return false;

		osgUtil::Simplifier simp(ratio);
		mSNode->accept(simp);

		OSGImageVisitor oiv(ratio);
		mSNode->accept(oiv);
	}
}
