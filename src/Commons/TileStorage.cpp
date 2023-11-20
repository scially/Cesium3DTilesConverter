#include <Commons/TileStorage.h>
#include <Commons/TileStorageDisk.h>

namespace scially {
	TileStorage::Ptr TileStorage::create(const QUrl& url) {
		QString schema = url.scheme();
		if (schema == "file") {
			TileStorageDisk::Ptr storage(new TileStorageDisk(url.path()));
			if (storage->init()) {
				return storage;
			}
		}
		else if (schema == "postgresql") {

		}
		else if (schema == "sqlite") {

		}
		else if (schema == "mongodb") {

		}
		return nullptr;
	}
}
