#include <QtTest/QTest>
#include <OSGBConvert.h>


using namespace gzpi;

class TestOSGBConvert : public QObject {
    Q_OBJECT
private slots:
    void testOSGBConvert() {
        OSGBConvert convert("./data/Tile_+007_+078.osgb");
        bool isConvert = convert.toB3DM();
        QVERIFY(isConvert == true);
    }
};

QTEST_MAIN(TestOSGBConvert)

#include "TestOSGBConvert.moc"