#pragma once

#include <QString>
#include <QVector>
#include <QByteArray>

namespace scially {
	class Batched3DModel {
	public:
		// feature table and feature binary
		int batchLength = 0;
		QByteArray featureBinary; 
		// batch table and batch binary
		QVector<int> batchID;
		QVector<QString> names;
		QVector<double> heights;
		// glb buffer
		QByteArray glbBuffer;

		QByteArray write(bool withHeight = false) const;
	};
}