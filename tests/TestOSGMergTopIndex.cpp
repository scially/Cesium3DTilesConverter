#include <OSGConvert/OSGMergeTopIndex.h>
#include <OSGConvert/OSGFolder.h>

#include "TestTemplate.h"

namespace scially {
	class TestOSGMergTopIndex : public QObject {
		Q_OBJECT
	private slots:
		void initTestCase() {
			options.thread = 1;
			options.output = "file:./";
			options.input = "./data/Production_5";
			QVERIFY(folder.load(options));

			for (int32_t i = 0; i < 5; i++) {
				for (int32_t j = 0; j < 5; j++) {
					MergeTileNode::Ptr node{
						new MergeTileNode(i, j, 2)
					};
					fakeNodes.append(node);
				}
			}
		}

		void testBuildPyramidIndex() {
			auto topNodes = MergeTileNodeBuilder::BuildPyramidIndex(fakeNodes, 2);
			for(const auto & node: topNodes) {
				qDebug() << node->xIndex() << node->yIndex() << node->zIndex();
			}
			QVERIFY(topNodes.size() == (5 / 2 + 1) * (5 / 2 + 1 ));
		}
				
		void testToB3DM() {
			QBENCHMARK{
				QVERIFY(folder.toB3DMPerTile(options));
			}
		}

		void testMergeTop() {
			QBENCHMARK{
				QVERIFY(folder.mergeTop(options));
			}
		}

	private:
		QList<MergeTileNode::Ptr> fakeNodes;
		OSGConvertOption options;
		OSGFolder folder;
	};
}

TESTTEMPLAGE_MAIN(scially::TestOSGMergTopIndex)
#include "TestOSGMergTopIndex.moc"