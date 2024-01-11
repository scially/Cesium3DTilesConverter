#include <Commons/TileStorageDisk.h>

#include <QDir>
#include <QFile>
#include <QtDebug>
#include <QJsonDocument>

namespace scially {
	bool TileStorageDisk::saveJson(const QString& file, const QJsonObject& content) const {
		QJsonDocument doc(content);
		#if NDEBUG
		QByteArray json = doc.toJson(QJsonDocument::Compact);
		#else
		QByteArray json = doc.toJson(QJsonDocument::Indented);
		#endif
		return saveFile(file, json);
	}

	bool TileStorageDisk::saveFile(const QString& file, const QByteArray& content) const {
        QString outPath = cleanPath(file);
        ensurePath(outPath);

		QFile writer(outPath);
		if (!writer.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qCritical("can't write file %s", qUtf8Printable(file));
			return false;
		}

		int r = writer.write(content);
		return r > 0;
	}

	bool TileStorageDisk::exists(const QString& file) const {
        return QDir(cleanPath(file)).exists();
	}

    QString TileStorageDisk::cleanPath(const QString& path) const {
        QString outPath;
        if (QDir::isAbsolutePath(path)) {
            outPath = path;
        }
        else {
            outPath = QDir(mFolder).absoluteFilePath(path);
        }

        return outPath;
    }

    bool TileStorageDisk::ensurePath(const QString& path) const{
		auto parentPath = QDir::cleanPath(QDir(path).filePath(QStringLiteral("..")));
		return QDir().mkpath(parentPath);
    }
}
