#include <Cesium3DTiles/Batched3DModel.h>
#include <Utils.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>

namespace scially {
	QByteArray Batched3DModel::write(bool withHeight) const {
		QByteArray buffer;
        QDataStream dataStream(&buffer, QIODevice::WriteOnly);
        dataStream.setByteOrder(QDataStream::LittleEndian);
        QString feature, batch;
        
        // feature json and feature binary
        feature = QString(R"({"BATCH_LENGTH":%1})").arg(batchLength);

        while ((feature.size() + 28) % 8 != 0) {
            feature.append(' ');
        }

        // batch json and batch binary
        {
            QJsonObject batchJson;
            batchJson["batchId"] = arrayToQJsonArray(batchID.begin(), batchID.end());
            batchJson["name"] = arrayToQJsonArray(names.begin(), names.end());

            if (withHeight) {
                batchJson["height"] = arrayToQJsonArray(heights.begin(), heights.end());
            }

            batch = QJsonDocument(batchJson).toJson(QJsonDocument::Compact);
            while (batch.size() % 8 != 0) {
                batch.append(' ');
            }
        }
        

        // feature binary and batch binary not implement
        int totalSize = 28 /*header size*/ + 
            feature.size() + batch.size() + glbBuffer.size();

        dataStream.writeRawData("b3dm", 4);
        dataStream << 1;
        dataStream << totalSize;
        dataStream << feature.size();
        dataStream << 0; // feature binary
        dataStream << batch.size();
        dataStream << 0;  // batch binary

        dataStream.writeRawData(feature.toLatin1(), feature.size());
        dataStream.writeRawData(batch.toLatin1(), batch.size());
        dataStream.writeRawData(glbBuffer.data(), glbBuffer.size());
       
		return buffer;
	}
}