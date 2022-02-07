#include <QtTest/QTest>
#include <OSGBConvert.h>


using namespace gzpi;

class TestOSGBConvert : public QObject {
    Q_OBJECT
private slots:
    void testOSGBConvert() {
        OSGBConvert convert("./data/Tile_+007_+078.osgb");
        QByteArray b3dmBuffer = convert.toB3DM();
        QVERIFY(b3dmBuffer.size() > 0);

    }
};

QTEST_MAIN(TestOSGBConvert)

#include "TestOSGBConvert.moc"