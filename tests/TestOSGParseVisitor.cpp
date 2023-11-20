#include <OSGConvert/OSGParseVisitor.h>
#include "TestTemplate.h"

#include <osgDB/ReadFile>
#include <QtTest/QtTest>

namespace scially {
	class TestOSGParseVisitor : public QObject {
		Q_OBJECT
	private slots:
		void initTestCase() {
			node = osgDB::readRefNodeFile("data/Production_5/Data/Tile_+000_+012/Tile_+000_+012.osgb");
			Q_ASSERT(node != nullptr);
		}

		void testCase1() {
			OSGParseVisitor visitor(osg::Vec3(0, 0, 0), nullptr);
			node->accept(visitor);
			Q_ASSERT(visitor.meshes.size() > 0);
		}

	private:
		osg::ref_ptr<osg::Node> node;
	};
}

TESTTEMPLAGE_MAIN(scially::TestOSGParseVisitor)
#include "TestOSGParseVisitor.moc"