#include <ShpConvertJob.h>
#include <ShpConvert.h>

#include <QtDebug>

namespace scially {
	void ShpConvertJob::run() {
        try{
            ShpConvert convert(input, layerName, height);
            convert.convertTiles(output);
        }catch(...){
            qCritical() << "Unkown error";
        }

	}
}
