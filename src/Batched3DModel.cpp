#include <Batched3DModel.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>

#include <Utils.h>

namespace scially {
	QByteArray Batched3DModel::write(bool withHeight) const {
		QByteArray buffer;
        QDataStream dataStream(&buffer, QIODevice::WriteOnly);
        dataStream.setByteOrder(QDataStream::LittleEndian);

        // feature json and feature binary
        QString featureJsonStr = QString(R"({"BATCH_LENGTH":%1})").arg(batchLength);

        while (featureJsonStr.size() % 4 != 0) {
            featureJsonStr.append(' ');
        }

        // batch json and batch binary
        QJsonObject batchJson;
        batchJson["batchId"] = arrayToQJsonArray(batchID.begin(), batchID.end());
        batchJson["name"] = arrayToQJsonArray(names.begin(), names.end());

        if (withHeight) {
            batchJson["height"] = arrayToQJsonArray(heights.begin(), heights.end());
        }

        QString batchJsonStr = jsonDump(batchJson);
        while (batchJsonStr.size() % 4 != 0) {
            batchJsonStr.append(' ');
        }


        // feature binary and batch binary not implement
        int totalSize = 28 /*header size*/ + featureJsonStr.size() + batchJsonStr.size() + glbBuffer.size();

        dataStream.writeRawData("b3dm", 4);
        dataStream << 1;
        dataStream << totalSize;
        dataStream << featureJsonStr.size();
        dataStream << 0; // feature binary
        dataStream << batchJsonStr.size();
        dataStream << 0;  // batch binary

        dataStream.writeRawData(featureJsonStr.toStdString().data(), featureJsonStr.size());
        dataStream.writeRawData(batchJsonStr.toStdString().data(), batchJsonStr.size());
        dataStream.writeRawData(glbBuffer.data(), glbBuffer.size());

		return buffer;
	}
}