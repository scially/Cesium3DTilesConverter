#include <QtTest/QTest>
#include <OSGBLevel.h>


using namespace gzpi;

class TestOSGBLevel : public QObject {
    Q_OBJECT
private slots:
    void testOSGBConvert() {
        OSGBLevel osgLevel("./data/Tile_+007_+078.osgb");
        osgLevel.getAllOSGBLevels(20);
        QVERIFY(osgLevel.subNodes.size() == 1);
        osgLevel.writeB3DM("./");
    }
};

QTEST_MAIN(TestOSGBLevel)

#include "TestOSGBLevel.moc"