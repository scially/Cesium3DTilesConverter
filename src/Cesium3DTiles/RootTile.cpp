#include <Cesium3DTiles/RootTile.h>

namespace scially {
    QJsonObject RootTile::write() const{
        QJsonObject object;
        object[boundingVolume.TypeName] = boundingVolume.write();
        object["geometricError"] = geometricError;
        object[refine.TypeName] = refine.write();
        object[transform.TypeName] = transform.write();

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

    void RootTile::read(const QJsonObject& object) {
        boundingVolume.read(object[boundingVolume.TypeName].toObject());
        geometricError = object["geometricError"].toDouble();
        transform.read(object[transform.TypeName].toArray());
        content.emplace();
        content->read(object[content->TypeName].toObject());
        refine.read(object[refine.TypeName].toString());

        QJsonArray values = object["children"].toArray();
        for (const auto& value : values) {
            RootTile r;
            r.read(value.toObject());
            children.append(r);
        }
    }
}
