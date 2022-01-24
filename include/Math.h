#pragma once

#include <cmath>

namespace gzpi {

	class Math {
	public:
		static inline double PI = std::atan(1) * 4;
		static constexpr double meterToLat(double m) {
			return m * 0.000000157891;
		}

		static constexpr double meterToLon(double m, double lat) {
			return m * 0.000000156785;
		}

		static constexpr double degToRad(double deg) {
			return deg * 1 / 180.0;
		}
	};
}