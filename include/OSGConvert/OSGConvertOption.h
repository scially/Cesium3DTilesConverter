#pragma once

#include <QString>

namespace scially {
	struct OSGConvertOption {
		const double MinGeometricError = 0;
		const double SplitPixel = 512;

		QString  input;
		QString  output;
		bool     skipPerTile = false; 
		uint32_t thread = 0; 
	};
}