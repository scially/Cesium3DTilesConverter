#pragma once

#include <cmath>

namespace scially{
	struct Constant {
		static constexpr double E = 2.71828182845904523536;  // e
		static constexpr double LOG2E = 1.44269504088896340736;  // log2(e)
		static constexpr double LOG10E = 0.434294481903251827651;  // log10(e)
		static constexpr double LN2 = 0.693147180559945309417;// ln(2)
		static constexpr double LN10 = 2.30258509299404568402; // ln(10)
		static constexpr double PI = 3.14159265358979323846; // pi
		static constexpr double PI_2 = 1.57079632679489661923;  // pi/2
		static constexpr double PI_4 = 0.785398163397448309616;  // pi/4
		static constexpr double SQRTPI_2 = 1.12837916709551257390;   // 2/sqrt(pi)
		static constexpr double SQRT2 = 1.41421356237309504880; // sqrt(2)
		static constexpr double SQRT1_2 = 0.707106781186547524401;  // 1/sqrt(2)
	};

	struct WGS84Ellipsoid {
		static constexpr double RX = 6378137.0;
		static constexpr double RY = 6378137.0;
		static constexpr double RZ = 6356752.3142451793;

		static constexpr double EPSILON1 = 0.1;
		static constexpr double PSILON2 = 0.01;
		static constexpr double EPSILON12 = 0.000000000001;
	};

}
