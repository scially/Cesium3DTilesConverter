#include <QtTest/QTest>
#include <cmath>
#include <3dtiles_core.h>

class Test3DTILES : public QObject {
    Q_OBJECT
private slots:
    void testBoundingVolume() {
        QJsonDocument doc = QJsonDocument::fromJson(R"( {
	        "region": [
                -1.2419052957251926,
                0.7395016240301894,
                -1.2415404171917719,
                0.7396563300150859,
                0,
                20.4
                ]
            })");
        auto boundingVolume = BoundingVolume::create(doc.object());
        QVERIFY(boundingVolume->write() == doc.object());
    }

    void testAssetProperties() {
        QJsonDocument doc = QJsonDocument::fromJson(R"({
            "version": "1.0",
            "tilesetVersion": "e575c6f1-a45b-420a-b172-6449fa6e0a59"
            })");
        auto asset = AssetProperties();
        asset.read(doc.object());
        QVERIFY(asset.assets.contains("version"));
        QVERIFY(asset.assets.contains("tilesetVersion"));
        QVERIFY(asset.assets["version"] == "1.0");
        QVERIFY(asset.assets["tilesetVersion"] == "e575c6f1-a45b-420a-b172-6449fa6e0a59");
        QVERIFY(asset.write() == doc.object());
    }

    void testTileMatrix() {
        QJsonDocument doc = QJsonDocument::fromJson(R"({ "transform": [
            4.843178171884396,   1.2424271388626869, 0,                  0,
            -0.7993325488216595,  3.1159251367235608, 3.8278032889280675, 0,
            0.9511533376784163, -3.7077466670407433, 3.2168186118075526, 0,
            1215001.7612985559, -4736269.697480114,  4081650.708604793,  1
            ]})");
        auto tileMatrix = TileMatrix();
        tileMatrix.read(doc.object()["transform"]);
        QVERIFY(tileMatrix.write() == doc["transform"]);
    }

    void testContentTile() {
        QJsonDocument doc = QJsonDocument::fromJson(R"({
	        "content": {
                "uri": "0/0/0.b3dm",
                "boundingVolume": {
                     "region": [
                         -0.0004001690908972599,
                         0.8988700116775743,
                         0.00010096729722787196,
                         0.8989625664878067,
                         0,
                         241.6
                         ]
              }}})");

        auto content = ContentTile();
        content.read(doc["content"]);
        QVERIFY(content.write() == doc["content"]);
    }

    void testRootTile() {
        QJsonDocument doc = QJsonDocument::fromJson(R"({
	        "root": {
            "boundingVolume": {
            "region": [
                -0.0005682966577418737,
                0.8987233516605286,
                0.00011646582098558159,
                0.8990603398325034,
                0,
                241.6
                ]
            },
            "geometricError": 268.37878244706053,
            "refine": "REPLACE",
            "content": {
                "uri": "0/0/0.b3dm",
                "boundingVolume": {
                "region": [
                    -0.0004001690908972599,
                    0.8988700116775743,
                    0.00010096729722787196,
                    0.8989625664878067,
                    0,
                    241.6
                    ]
                }
              }
            }})");

        auto root = RootTile();
        root.read(doc["root"]);
        QVERIFY(root.write() == doc["root"]);
    }

    void testBaseTile() {
        QJsonDocument doc = QJsonDocument::fromJson(R"({
            "asset": {
              "version" : "1.0"
            },
            "geometricError": 494.50961650991815,
            "root": {
              "boundingVolume": {
                "region": [
                  -0.0005682966577418737,
                  0.8987233516605286,
                  0.00011646582098558159,
                  0.8990603398325034,
                  0,
                  241.6
                ]
              },
              "content": {
                "boundingVolume": {
                  "region": [
                    -0.0004001690908972599,
                    0.8988700116775743,
                    0.00010096729722787196,
                    0.8989625664878067,
                    0,
                    241.6
                  ]
                },
                "uri": "0/0/0.b3dm"
              },
              "geometricError": 268.37878244706053,
              "refine": "ADD",
              "children": [
                {
                  "boundingVolume": {
                    "region": [
                      -0.0004853062518095434,
                      0.898741188925484,
                      -0.0002736676267127107,
                      0.8989037314387226,
                      0,
                      158.4
                    ]
                  },
                  "content": {
                    "boundingVolume": {
                      "region": [
                        -0.0004058588642587614,
                        0.898746512179703,
                        -0.0002736676267127107,
                        0.8989037314387226,
                        0,
                        158.4
                      ]
                    },
                    "uri": "1/0/0.b3dm"
                  },
                  "geometricError": 159.43385994848,
                  "children": [
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0004853062518095434,
                          0.898741188925484,
                          -0.0003930656008416433,
                          0.898818995703538,
                          0,
                          66.7
                        ]
                      },
                      "content": {
                        "uri": "2/0/0.b3dm"
                      },
                      "geometricError": 10.831613588830955
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0003984063083527456,
                          0.8987434753068045,
                          -0.00028070130359824817,
                          0.8988027117816164,
                          0,
                          48.2
                        ]
                      },
                      "content": {
                        "uri": "2/1/0.b3dm"
                      },
                      "geometricError": 11.833855250694043
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00039631191325035245,
                          0.8988008442793176,
                          -0.000280491864088009,
                          0.8989002407802179,
                          0,
                          78.1
                        ]
                      },
                      "content": {
                        "uri": "2/1/1.b3dm"
                      },
                      "geometricError": 24.187299340965403
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00047979101137324135,
                          0.8988092742196048,
                          -0.0003937113726648811,
                          0.898901462510695,
                          0,
                          122.2
                        ]
                      },
                      "content": {
                        "uri": "2/0/1.b3dm"
                      },
                      "geometricError": 48.508446081365975
                    }
                  ]
                },
                {
                  "boundingVolume": {
                    "region": [
                      -0.0002874033679259065,
                      0.8987233516605286,
                      0.00009827949017980081,
                      0.8988939226883266,
                      0,
                      75.2
                    ]
                  },
                  "content": {
                    "boundingVolume": {
                      "region": [
                        -0.00028230700651008276,
                        0.8987309438427749,
                        -0.00011402236003278958,
                        0.8988939226883266,
                        0,
                        75.2
                      ]
                    },
                    "uri": "1/1/0.b3dm"
                  },
                  "geometricError": 132.82048511777703,
                  "children": [
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0002874033679259065,
                          0.8987293381398633,
                          -0.00018024015185345448,
                          0.8987995352823785,
                          0,
                          38.1
                        ]
                      },
                      "content": {
                        "uri": "2/2/0.b3dm"
                      },
                      "geometricError": 4.206139430532202
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00018407987620784196,
                          0.8987233516605286,
                          -0.0000894132175796695,
                          0.8987836876927705,
                          0,
                          72.9
                        ]
                      },
                      "content": {
                        "uri": "2/3/0.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00018493508754131914,
                          0.8987896218122265,
                          0.00009827949017980081,
                          0.8988920377327339,
                          0,
                          46.9
                        ]
                      },
                      "content": {
                        "uri": "2/3/1.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00027722809838677943,
                          0.8987969696483782,
                          -0.00017832028967626075,
                          0.8988916014004213,
                          0,
                          55.4
                        ]
                      },
                      "content": {
                        "uri": "2/2/1.b3dm"
                      },
                      "geometricError": 0
                    }
                  ]
                },
                {
                  "boundingVolume": {
                    "region": [
                      -0.0002821848334624433,
                      0.8988867144785156,
                      0.00011646582098558159,
                      0.8990603398325034,
                      0,
                      158
                    ]
                  },
                  "content": {
                    "boundingVolume": {
                      "region": [
                        -0.0002782229360604159,
                        0.8989292306990948,
                        0.000006946410422937427,
                        0.899046220118855,
                        0,
                        158
                      ]
                    },
                    "uri": "1/1/1.b3dm"
                  },
                  "geometricError": 156.46285780389445,
                  "children": [
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00027865926837341453,
                          0.8988880758353316,
                          -0.00014501940754820897,
                          0.8989746092596459,
                          0,
                          77.3
                        ]
                      },
                      "content": {
                        "uri": "2/2/2.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00015598007525073333,
                          0.8988867144785156,
                          0.00011646582098558159,
                          0.8989826028676196,
                          0,
                          106.2
                        ]
                      },
                      "content": {
                        "uri": "2/3/2.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00015252432333178438,
                          0.8989769130942584,
                          0.00003328342883553189,
                          0.8990603398325034,
                          0,
                          67.9
                        ]
                      },
                      "content": {
                        "uri": "2/3/3.b3dm"
                      },
                      "geometricError": 8.010233984367021
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0002821848334624433,
                          0.8989765465751156,
                          -0.0001477072145962801,
                          0.899040914317929,
                          0,
                          76
                        ]
                      },
                      "content": {
                        "uri": "2/2/3.b3dm"
                      },
                      "geometricError": 40.38435697163592
                    }
                  ]
                },
                {
                  "boundingVolume": {
                    "region": [
                      -0.0005682966577418737,
                      0.8989007643789939,
                      -0.0002669481090925327,
                      0.8990582279841088,
                      0,
                      204
                    ]
                  },
                  "content": {
                    "boundingVolume": {
                      "region": [
                        -0.0005526410543514849,
                        0.8989100669839071,
                        -0.0002669481090925327,
                        0.8990037911647392,
                        0,
                        204
                      ]
                    },
                    "uri": "1/0/1.b3dm"
                  },
                  "geometricError": 149.600454457028,
                  "children": [
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0005474399731805417,
                          0.8989017068567899,
                          -0.00040917498983755046,
                          0.8990014698768336,
                          0,
                          81.2
                        ]
                      },
                      "content": {
                        "uri": "2/0/2.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.00041203732981082115,
                          0.8989007643789939,
                          -0.00027176521782803744,
                          0.8989922894449685,
                          0,
                          108.7
                        ]
                      },
                      "content": {
                        "uri": "2/1/2.b3dm"
                      },
                      "geometricError": 0
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0004253716452960582,
                          0.8989891478523147,
                          -0.0002760587277879431,
                          0.8990362368355337,
                          0,
                          30.1
                        ]
                      },
                      "content": {
                        "uri": "2/1/3.b3dm"
                      },
                      "geometricError": 18.837170280352364
                    },
                    {
                      "boundingVolume": {
                        "region": [
                          -0.0005682966577418737,
                          0.8989984853638134,
                          -0.000407220221075317,
                          0.8990582279841088,
                          0,
                          53.3
                        ]
                      },
                      "content": {
                        "uri": "2/0/3.b3dm"
                      },
                      "geometricError": 67.4774528507299
                    }
                  ]
                }
              ]
        }})");
        auto tile = BaseTile();
        tile.read(doc.object());
        QString str = QJsonDocument(tile.write().toObject()).toJson();
        QVERIFY(tile.write() == doc.object());
    }

};

QTEST_MAIN(Test3DTILES)

#include "test_3dtiles.moc"