#include <Cesium3DTiles/BaseTile.h>

namespace scially {
    QJsonObject BaseTile::write() const {
        QJsonObject object;   
        object[asset.TypeName] = asset.write();
        object["geometricError"] = geometricError;
        object[root.TypeName] = root.write();
        return object;
    }
    
    void BaseTile::read(const QJsonObject& object) {
        asset.read(object[asset.TypeName].toObject());
        geometricError = object["geometricError"].toDouble();
        root.read(object["root"].toObject());
    }

}
