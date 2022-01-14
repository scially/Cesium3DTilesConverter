#include <Refine.h>
#include <TilesParseException.h>

namespace gzpi {
    void Refine::read(const QJsonValue& object) {
        if (!required(object, QJsonValue::String))
            throw TilesParseException("refine is required");
        if (object == "ADD") {
            type_ = ADD;
        }
        else if (object == "REPLACE") {
            type_ = REPLACE;
        }
        else {
            throw TilesParseException("refine is not add neither nor replace");
        }
    }
    QJsonValue Refine::write() {
        return QJsonValue(toString());
    }
    QString Refine::toString() const {
        return typeMeta.valueToKey(type_);
    }
}