#include <BaseTile.h>

namespace scially {
 
    QJsonValue BaseTile::write() {
        QJsonObject object;
        
        object[asset.typeName()] = asset.write();
        object["geometricError"] = geometricError;
        object[root.typeName()] = root.write();
        return object;
    }
    
    void BaseTile::read(const QJsonValue& object) {
        if (!object.isObject())
            return;
    
        asset.read(object["asset"]);
        geometricError = object["geometricError"].toDouble();
        root.read(object["root"]);

    }

}
