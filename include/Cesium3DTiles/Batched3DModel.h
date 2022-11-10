#pragma once

#include <QString>
#include <QVector>
#include <QByteArray>

namespace scially {
	struct Batched3DModel {
		QByteArray write(bool withHeight = false) const;

		// feature table and feature binary
		int batchLength = 0;
		QByteArray featureBinary; 
		// batch table and batch binary
		QVector<int> batchID;
		QVector<QString> names;
		QVector<double> heights;
		// glb buffer
		QByteArray glbBuffer;
	};
}