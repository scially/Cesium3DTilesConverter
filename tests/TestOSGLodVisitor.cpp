#include <OSGConvert/OSGLodVisitor.h>
#include "TestTemplate.h"

#include <osgDB/ReadFile>
#include <QtTest/QtTest>
#include <QtDebug>

namespace scially {
	class TestOSGLodVisitor : public QObject {
		Q_OBJECT
	private slots:
		void initTestCase(){
			node = osgDB::readRefNodeFile("data/Production_5/Data/Tile_+000_+012/Tile_+000_+012.osgb");
			QVERIFY(node != nullptr);
		}

		void testCase1() {
			OSGLodVisitor visitor;
			node->accept(visitor);
			QVERIFY(visitor.vertexCount > 0);
			QVERIFY(visitor.children.size() > 0);
		}

	private:
		osg::ref_ptr<osg::Node> node;
	};
}

TESTTEMPLAGE_MAIN(scially::TestOSGLodVisitor)
#include "TestOSGLodVisitor.moc"