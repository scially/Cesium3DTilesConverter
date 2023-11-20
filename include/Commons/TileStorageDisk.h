#pragma once

#include "TileStorage.h"

namespace scially {
	class TileStorageDisk : public TileStorage {
	public:
		using Ptr = QSharedPointer<TileStorageDisk>;

		TileStorageDisk(const QString& folder) : mFolder(folder) {}
		
		virtual bool saveJson(const QString& file, const QJsonObject& content) override;
		virtual bool saveFile(const QString& file, const QByteArray&  content) override;

		virtual bool exists(const QString& file) override;
	private:
        QString cleanPath(const QString& path);
        bool ensurePath(const QString& path);

		QString mFolder;
	};
}
