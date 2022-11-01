#include <RootTile.h>

namespace scially {

    RootTile::RootTile() {
        children.clear();
    }

    QJsonValue RootTile::write() {
        QJsonObject object;
        object["boundingVolume"] = boundingVolume.write();
        object["geometricError"] = geometricError;
        object["refine"] = refine.write();
        object[transform.typeName()] = transform.write();

        if (content.has_value()) {
            object["content"] = content->write();
        }

        QJsonArray childrenValue;
        for (auto& child : children) {
            childrenValue.append(child.write());
        }
        object["children"] = childrenValue;
        return object;
    }

    void RootTile::read(const QJsonValue& object) {
        if (object.isNull())
            return;

        boundingVolume.read(object["boundingVolume"]);
        geometricError = object["geometricError"].toDouble();
        transform.read(object["transform"]);

        content.emplace();
        content->read(object["content"]);

        refine.read(object["refine"]);

        QJsonArray values = object["children"].toArray();
        for (const auto& value : values) {
            RootTile r;
            r.read(value);
            children.append(r);
        }

    }
}
