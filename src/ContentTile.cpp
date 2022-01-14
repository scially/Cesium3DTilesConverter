#include <ContentTile.h>
#include <TilesParseException.h>

namespace gzpi {

    QJsonValue ContentTile::write() {
        QJsonObject object;
        if (boundingVolume != nullptr)
            object["boundingVolume"] = boundingVolume->write();
        object["uri"] = uri;
        return object;
    }

    void ContentTile::read(const QJsonValue& object) {

        if (object.isNull())
            return;

        if (required(object["boundingVolume"], QJsonValue::Object))
            boundingVolume = BoundingVolume::create(object["boundingVolume"]);

        if (!required(object["uri"], QJsonValue::String))
            throw TilesParseException("uri is required");
        uri = object["uri"].toString();
    }
}