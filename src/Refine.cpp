#include <Refine.h>
#include <TilesParseException.h>
#include <QDebug>

namespace scially {
    void Refine::read(const QJsonValue& object) {
       if(object.isString()){
            type = object.toString();
       }
    }

    QJsonValue Refine::write() {
        return type;
    }
}
