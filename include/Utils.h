#pragma once

#include <osg/Math>

namespace scially {
	/************************************* MATH Utils *****************************************************/
	constexpr double METERIC = 0.01;

	inline double meterToLat(double m/*, double lat*/) {
		return osg::RadiansToDegrees(m * 0.000000157891);
	}

	inline float meterToLat(float m/*, float lat*/) {
		return osg::RadiansToDegrees(m * 0.000000157891);
	}

	inline double meterToLon(double m, double lat) {
		return osg::RadiansToDegrees(m * 0.000000156785 / std::cos(osg::DegreesToRadians(lat)));
	}

	inline float meterToLon(float m, float lat) {
		return osg::RadiansToDegrees(m * 0.000000156785 / std::cos(osg::DegreesToRadians(lat)));
	}

	inline double latToMeter(double diff/*, double lat*/) {
		return osg::DegreesToRadians(diff) / 0.000000157891;
	}

	inline float latToMeter(float diff/*, float lat*/) {
		return osg::DegreesToRadians(diff) / 0.000000157891;
	}

	inline double lonToMeter(double diff, double lat) {
		return osg::DegreesToRadians(diff) / 0.000000156785 * std::cos(osg::DegreesToRadians(lat));
	}

	inline float lonToMeter(float diff, float lat) {
		return osg::DegreesToRadians(diff) / 0.000000156785 * std::cos(osg::DegreesToRadians(lat));
	}
}