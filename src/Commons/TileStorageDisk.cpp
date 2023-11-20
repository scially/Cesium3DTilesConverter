#include <Commons/TileStorageDisk.h>

#include <QDir>
#include <QFile>
#include <QtDebug>
#include <QJsonDocument>

namespace scially {
	bool TileStorageDisk::saveJson(const QString& file, const QJsonObject& content) {
		QJsonDocument doc(content);
		#if NDEBUG
		QByteArray json = doc.toJson(QJsonDocument::Compact);
		#else
		QByteArray json = doc.toJson(QJsonDocument::Indented);
		#endif
		return saveFile(file, json);
	}

	bool TileStorageDisk::saveFile(const QString& file, const QByteArray& content) {
        QString outPath = cleanPath(file);
        ensurePath(outPath);

		QFile writer(outPath);
		if (!writer.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qCritical() << "can't write file:" << file;
			return false;
		}

		int r = writer.write(content);
		return r > 0;
	}

	bool TileStorageDisk::exists(const QString& file) {
        return QDir(cleanPath(file)).exists();
	}

    QString TileStorageDisk::cleanPath(const QString& path){
        QString outPath;
        if (QDir::isAbsolutePath(path)) {
            outPath = path;
        }
        else {
            outPath = QDir(mFolder).absoluteFilePath(path);
        }

        return outPath;
    }

    bool TileStorageDisk::ensurePath(const QString& path){
		auto parentPath = QDir::cleanPath(QDir(path).filePath(QStringLiteral("..")));
		return QDir().mkpath(parentPath);
    }
}
