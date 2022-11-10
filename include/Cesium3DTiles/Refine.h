#pragma once

#include <QString>

namespace scially {
    /// <summary>
    /// Refinement determines the process by which a lower resolution parent tile renders
    /// when its higher resolution children are selected to be rendered.
    /// Permitted refinement types are replacement ("REPLACE") and additive ("ADD")
    /// </summary>
    struct Refine {
        static constexpr const char* TypeName = "refine";
        
        void read(const QString& object) { type = object; }
        QString write() const  { return type;  }

        QString type = "REPLACE";
    };

}
