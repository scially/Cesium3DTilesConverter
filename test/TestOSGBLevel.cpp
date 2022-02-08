#include <QtTest/QTest>
#include <OSGBLevel.h>
#include <BaseTile.h>

using namespace scially;

class TestOSGBLevel : public QObject {
    Q_OBJECT
private slots:
    void testOSGBConvert() {
        OSGBLevel level("./Production_3/Data/Tile_+000_+000/Tile_+000_+000.osgb");
        BaseTile tile;
        level.convertTiles(tile, "./");
    }
};

QTEST_MAIN(TestOSGBLevel)

#include "TestOSGBLevel.moc"
