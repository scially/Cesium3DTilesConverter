#include <RootTile.h>

namespace gzpi {

    RootTile::RootTile() {
        children.clear();
    }
    QJsonValue RootTile::write() {
        QMetaEnum metaState = QMetaEnum::fromType<Refine::RefineType>();
        QJsonObject object;
        object["boundingVolume"] = boundingVolume->write();
        object["geometricError"] = geometricError;

        if (refine != nullptr)
            object["refine"] = refine->write();

        if (content != nullptr) {
            object["content"] = content->write();
        }

        if (children.size() != 0) {
            QJsonArray childrenValue;
            for (auto& child : children) {
                childrenValue.append(child->write());
            }
            object["children"] = childrenValue;
        }

        return object;
    }

    void RootTile::read(const QJsonValue& object) {
        if (object.isNull() || object.isUndefined())
            return;

        if (!required(object["boundingVolume"], QJsonValue::Object))
            throw ThreedTilesParseException("boundingVolume is required");
        boundingVolume = BoundingVolume::create(object["boundingVolume"]);

        if (!required(object["geometricError"], QJsonValue::Double))
            throw ThreedTilesParseException("geometricError is required");
        geometricError = object["geometricError"].toDouble();

        if (required(object["transform"], QJsonValue::Array)) {
            transform = TileMatrix::TileMatrixPtr::create();
            transform->read(object["transform"]);
        }


        if (required(object["content"], QJsonValue::Object)) {
            content = ContentTile::ContentTilePtr::create();
            content->read(object["content"]);
        }


        if (required(object["refine"], QJsonValue::String)) {
            refine = Refine::RefinePtr::create();
            refine->read(object["refine"]);
        }


        if (required(object["children"], QJsonValue::Array)) {
            QJsonArray values = object["children"].toArray();
            for (const auto& value : values) {
                RootTilePtr r = QSharedPointer<RootTile>::create();
                r->read(value);
                children.append(r);
            }
        }
    }
}