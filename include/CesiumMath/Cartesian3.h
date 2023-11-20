#pragma once

#include "Constant.h"

#include <osg/Vec3d>
#include <osg/Matrixd>

namespace scially {
	struct Cartesian3 {

		double x = 0;
		double y = 0;
		double z = 0;

		static constexpr double WGS84CenterToleranceSquared = WGS84Ellipsoid::EPSILON1;
		
		Cartesian3() = default;
		Cartesian3(double x, double y, double z) : x(x), y(y), z(z) {}

		void toRadians(double& longitude, double& latitude, double& height);

		void toDegrees(double& longitude, double& latitude, double& height);

		static Cartesian3 fromDegree(double lon, double lat, double height = 0);

		static Cartesian3 fromRadians(double longitude, double latitude, double height = 0);

		static Cartesian3 add(Cartesian3 left, Cartesian3 right);

		static double  magnitudeSquared(Cartesian3 cartesian);

		static double  magnitude(Cartesian3 cartesian);

		static Cartesian3 normalize(Cartesian3 cartesian);

		static Cartesian3 multiplyComponents(Cartesian3 left, Cartesian3  right);

		static double  dot(Cartesian3 left, Cartesian3  right);

		static Cartesian3 divideByScalar(Cartesian3 cartesian, double scalar);

		static Cartesian3 multiplyByScalar(Cartesian3 cartesian, double  scalar);

		static Cartesian3 scaleToGeodeticSurface(Cartesian3 cartesian, Cartesian3 oneOverRadii, Cartesian3 oneOverRadiiSquared, double centerToleranceSquared);

		static Cartesian3 subtract(Cartesian3 left, Cartesian3 right);

		static Cartesian3 cross(Cartesian3 left, Cartesian3 right);

		static Cartesian3 computeHorizonCullingPointFromVertices(Cartesian3 directionToPoint, double* vertices, int elementSize, int stride, Cartesian3 center);

		static Cartesian3 magnitudeToPoint(Cartesian3 scaledSpaceDirectionToPoint, double resultMagnitude);

		static Cartesian3  computeScaledSpaceDirectionToPoint(Cartesian3 directionToPoint);

		static Cartesian3  transformPositionToScaledSpace(Cartesian3 directionToPoint);

		static Cartesian3  transformScaledSpaceToPosition(Cartesian3 directionToPoint);

		static double computeMagnitude(Cartesian3 position, Cartesian3 scaledSpaceDirectionToPoint);

		static Cartesian3  geodeticSurfaceNormal(Cartesian3 cartesian);

        static osg::Matrixd eastNorthUpMatrixWorld(const osg::Vec3d& center);

        static osg::Matrixd eastNorthUpMatrix(const osg::Vec3d& center);

		static const Cartesian3 WGS84Radii;
		static const Cartesian3 WGS84RadiiSquared;
		static const Cartesian3 WGS84OneOverRadii;
		static const Cartesian3 WGS84OneOverRadiiSquared;
	};

	
}
