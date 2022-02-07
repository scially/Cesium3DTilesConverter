#include <QtTest/QTest>
#include <OSGBConvertJob.h>

using namespace gzpi;

class TestOSGBConvertJob : public QObject {
    Q_OBJECT
private slots:
    void testOSGBConvertJob() {
        OSGBConvertJob convertJob("./Production_3", "./");
        convertJob.run();
    }
};

QTEST_MAIN(TestOSGBConvertJob)

#include "TestOSGBConvertJob.moc"
