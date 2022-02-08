#include <ShpConvertJob.h>
#include <ShpConvert.h>

namespace scially {
	void ShpConvertJob::run() {
		ShpConvert convert(input, layerName, height);
		convert.convertTiles(output);
	}
}