#pragma once

#include <QString>

namespace scially {
	struct OSGConvertOption {
		QString  input;
		QString  output;
		uint32_t thread = 0;
	};
}