#pragma once

#include "TileStorage.h"

namespace scially {
	class TileStorageDisk : public TileStorage {
	public:
		using Ptr = QSharedPointer<TileStorageDisk>;

		TileStorageDisk(const QString& folder) : mFolder(folder) {}
		
		virtual bool saveJson(const QString& file, const QJsonObject& content) const override;
		virtual bool saveFile(const QString& file, const QByteArray&  content) const override;

		virtual bool exists(const QString& file) const override;
	private:
        QString cleanPath(const QString& path) const;
        bool ensurePath(const QString& path) const;

		QString mFolder;
	};
}
