#include <BaseTile.h>

namespace gzpi {
 
    QJsonValue BaseTile::write() {
        QJsonObject object;
        object[asset->typeName()] = asset->write();
        object["geometricError"] = geometricError;
        object[root->typeName()] = root->write();
        return object;
    }
    
    void BaseTile::read(const QJsonValue& object) {
        if (!object.isObject())
            throw TilesParseException("object is not Json Object");
    
        if (required(object["asset"], QJsonValue::Object)) {
            asset = AssetProperties::AssetPropertiesPtr::create();
            asset->read(object["asset"]);
        }
    
        QJsonValue geometricErrorValue = object["geometricError"];
        if (!required(geometricErrorValue, QJsonValue::Double))
            throw TilesParseException("geometricError is required");
        geometricError = geometricErrorValue.toDouble();
    
        if (required(object["root"], QJsonValue::Object)) {
            root = RootTile::RootTilePtr::create();
            root->read(object["root"]);
        }
    }

}
