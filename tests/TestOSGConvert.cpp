#include <OSGConvert/OSGFolder.h>
#include <OSGConvert/OSGVirtualFolder.h>

#include "TestTemplate.h"

namespace scially {
	class TestOSGConvert : public QObject {
		Q_OBJECT
	
	private slots:
		void initTestCase() {
			folder = QSharedPointer<OSGFolder>::create("data/Production_5");
			QVERIFY(folder->load("file:./"));
		}
			
		void testToB3DM() {
			QBENCHMARK{				
				auto nodes = folder->to3DTiles(1);
				QVERIFY(nodes.size() > 0);
				vFolder = QSharedPointer<OSGVirtualFolder>::create(
					nodes,
					folder->outSRS(),
					*folder->transform(),
					*folder->storage());
			};
		}

		void testMergeTop() {
			QBENCHMARK{                                                      
				auto nodes = vFolder->to3DTiles(1);
				QVERIFY(nodes.size() > 0);
			}
		}

	private:
		QSharedPointer<OSGFolder> folder;
		QSharedPointer<OSGVirtualFolder> vFolder;
	};
}

TESTTEMPLAGE_MAIN(scially::TestOSGConvert)
#include "TestOSGConvert.moc"