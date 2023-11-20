#include <OSGConvert/OSGFolder.h>
#include "TestTemplate.h"

#include <QtTest/QtTest>
#include <QtDebug>

namespace scially {
	class TestOSGFolder : public QObject {
		Q_OBJECT
	private slots:
		void initTestCase() {
			options.thread = 1;
			options.output = "file:./";
			options.input = "./data/Production_5";
			Q_ASSERT(folder.load(options));
		}

		void testToB3DM() {
			Q_ASSERT(folder.toB3DMPerTile(options));
		}

	private:
		OSGConvertOption options;
		OSGFolder folder;
	};
}

TESTTEMPLAGE_MAIN(scially::TestOSGFolder)
#include "TestOSGFolder.moc"