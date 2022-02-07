#pragma once
#include <OGRException.h>
#include <osg/Math>

namespace gzpi {

	class Math {
	public:
		// 单位均为度
		/*纬度变化一度，球面南北方向距离变化：πR/180 ........111.7km
		  经度变化一度，球面东西方向距离变化：πR/180*cosB ....111.7*cosB*/
		static double meterToLat(double m/*, double lat*/) {
			return osg::RadiansToDegrees(m * 0.000000157891);
		}
        static float meterToLat(float m/*, float lat*/) {
            return osg::RadiansToDegrees(m * 0.000000157891);
        }

		static double meterToLon(double m, double lat) {
			return osg::RadiansToDegrees(m * 0.000000156785 / std::cos(osg::DegreesToRadians(lat)));
		}
        static float meterToLon(float m, float lat) {
            return osg::RadiansToDegrees(m * 0.000000156785 / std::cos(osg::DegreesToRadians(lat)));
        }

		static double latToMeter(double diff/*, double lat*/) {
			return osg::DegreesToRadians(diff) / 0.000000157891;
		}
        static float latToMeter(float diff/*, float lat*/) {
            return osg::DegreesToRadians(diff) / 0.000000157891;
        }

		static double lonToMeter(double diff, double lat) {
			return osg::DegreesToRadians(diff) / 0.000000156785 * std::cos(osg::DegreesToRadians(lat));
		}
        static float lonToMeter(float diff, float lat) {
            return osg::DegreesToRadians(diff) / 0.000000156785 * std::cos(osg::DegreesToRadians(lat));
        }

        static constexpr double METERIC = 0.01;
	};

	class CoordinateConvert : QObject {
		Q_OBJECT
	public:
		using OGRCoordinateTransformationPtr = QSharedPointer<OGRCoordinateTransformation>;

		double sourceX;
		double sourceY;
		double targetX = 0;
		double targetY = 0;

		enum SrsType {
			WKT,
			EPSG,
			Proj4
		};
		Q_ENUM(SrsType);

		CoordinateConvert(double x, double y) : sourceX(x), sourceY(y), targetX(0), targetY(0) {}
		CoordinateConvert() : CoordinateConvert(0, 0) {}

		void setSourceSrs(const QString& srs, SrsType t) noexcept(false);
		void setTargetSrs(const QString& srs, SrsType t) noexcept(false);
		void transform() noexcept(false);

		

	private:
		void setSrs(OGRSpatialReference& srs, const QString& describe, SrsType t) ;
		OGRCoordinateTransformationPtr createCoordinateTransformation();
	private:
		OGRSpatialReference sourceSrs;
		OGRSpatialReference targetSrs;
	};
}
