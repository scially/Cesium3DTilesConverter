#pragma once

#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QSharedPointer>

namespace scially {

	class TileStorage {
	public:
		using Ptr = QSharedPointer<TileStorage>;

		static TileStorage::Ptr create(const QUrl& path);

		virtual bool init() { return true; }
		virtual bool saveJson(const QString& file, const QJsonObject& content) = 0;
		virtual bool saveFile(const QString& file, const QByteArray& content) = 0;

		virtual bool exists(const QString& file) {
			return false;
		}
	};
}