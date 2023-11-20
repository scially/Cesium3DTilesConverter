#include <CesiumMath/Cartesian3.h>

#include <cmath>


namespace scially {

	static double sign(double value) {
		if (value > 0) {
			return 1;
		}
		if (value < 0) {
			return -1;
		}
		return 0;
	};



	Cartesian3 Cartesian3::fromDegree(double lon, double lat, double height) {
		return fromRadians(lon * Constant::PI / 180, lat * Constant::PI / 180, height);
	}

	Cartesian3 Cartesian3::fromRadians(double longitude, double latitude, double height) {

		Cartesian3 radiiSquared = WGS84RadiiSquared;

		double cosLatitude = std::cos(latitude);

		Cartesian3 scratchN;
		Cartesian3 scratchK;

		scratchN.x = cosLatitude * std::cos(longitude);
		scratchN.y= cosLatitude * std::sin(longitude);
		scratchN.z = std::sin(latitude);
		scratchN = normalize(scratchN);

		scratchK = multiplyComponents(radiiSquared, scratchN);
		double gamma = sqrt(dot(scratchN, scratchK));
		scratchK = divideByScalar(scratchK, gamma);
		scratchN = multiplyByScalar(scratchN, height);

		return add(scratchK, scratchN);
	}

	Cartesian3 Cartesian3::add(Cartesian3 left, Cartesian3 right) {
		Cartesian3 result;

		result.x = left.x + right.x;
		result.y = left.y + right.y;
		result.z = left.z + right.z;
		return result;
	};


	double Cartesian3::magnitudeSquared(Cartesian3 cartesian) {
		return cartesian.x * cartesian.x 
			+ cartesian.y * cartesian.y 
			+ cartesian.z * cartesian.z;
	};

	/**
	* Computes the Cartesian's magnitude (length).
	*
	* @param {Cartesian3} cartesian The Cartesian instance whose magnitude is to be computed.
	* @returns {Number} The magnitude.
	*/
	double  Cartesian3::magnitude(Cartesian3 cartesian) {
		return sqrt(magnitudeSquared(cartesian));
	};

	Cartesian3 Cartesian3::normalize(Cartesian3 cartesian) {
		double  magnitude = Cartesian3::magnitude(cartesian);

		Cartesian3 result;

		result.x = cartesian.x / magnitude;
		result.y = cartesian.y / magnitude;
		result.z = cartesian.z / magnitude;

		return result;
	};

	Cartesian3 Cartesian3::multiplyComponents(Cartesian3 left, Cartesian3 right) {
		Cartesian3 result;
		result.x = left.x * right.x;
		result.y = left.y * right.y;
		result.z = left.z * right.z;
		return result;
	};

	double  Cartesian3::dot(Cartesian3 left, Cartesian3  right) {
		return left.x * right.x 
			+ left.y * right.y 
			+ left.z * right.z;
	};

	Cartesian3 Cartesian3::divideByScalar(Cartesian3 cartesian, double scalar) {

		Cartesian3 result;
		result.x = cartesian.x / scalar;
		result.y = cartesian.y / scalar;
		result.z = cartesian.z / scalar;
		return result;
	};

	Cartesian3 Cartesian3::multiplyByScalar(Cartesian3 cartesian, double  scalar) {
		Cartesian3 result;
		result.x = cartesian.x * scalar;
		result.y = cartesian.y * scalar;
		result.z = cartesian.z * scalar;
		return result;
	};


	Cartesian3 Cartesian3::scaleToGeodeticSurface(Cartesian3 cartesian, Cartesian3 oneOverRadii, Cartesian3 oneOverRadiiSquared, double centerToleranceSquared) {
		auto positionX = cartesian.x;
		auto positionY = cartesian.y;
		auto positionZ = cartesian.z;

		auto oneOverRadiiX = oneOverRadii.x;
		auto oneOverRadiiY = oneOverRadii.y;
		auto oneOverRadiiZ = oneOverRadii.z;

		auto x2 = positionX * positionX * oneOverRadiiX * oneOverRadiiX;
		auto y2 = positionY * positionY * oneOverRadiiY * oneOverRadiiY;
		auto z2 = positionZ * positionZ * oneOverRadiiZ * oneOverRadiiZ;

		// Compute the squared ellipsoid norm.
		auto squaredNorm = x2 + y2 + z2;
		auto ratio = sqrt(1.0 / squaredNorm);

		// As an initial approximation, assume that the radial intersection is the projection point.
		auto intersection = multiplyByScalar(cartesian, ratio);

		// If the position is near the center, the iteration will not converge.
		if (squaredNorm < centerToleranceSquared) {
			return  intersection;
		}

		auto oneOverRadiiSquaredX = oneOverRadiiSquared.x;
		auto oneOverRadiiSquaredY = oneOverRadiiSquared.y;
		auto oneOverRadiiSquaredZ = oneOverRadiiSquared.z;

		// Use the gradient at the intersection point in place of the true unit normal.
		// The difference in magnitude will be absorbed in the multiplier.
		Cartesian3 gradient;
		gradient.x = intersection.x * oneOverRadiiSquaredX * 2.0;
		gradient.y = intersection.y * oneOverRadiiSquaredY * 2.0;
		gradient.z = intersection.z * oneOverRadiiSquaredZ * 2.0;

		// Compute the initial guess at the normal vector multiplier, lambda.
		auto lambda = (1.0 - ratio) * magnitude(cartesian) / (0.5 * magnitude(gradient));
		auto correction = 0.0;

		double func;
		double denominator;
		double xMultiplier;
		double yMultiplier;
		double zMultiplier;
		double xMultiplier2;
		double yMultiplier2;
		double zMultiplier2;
		double xMultiplier3;
		double yMultiplier3;
		double zMultiplier3;

		do {
			lambda -= correction;

			xMultiplier = 1.0 / (1.0 + lambda * oneOverRadiiSquaredX);
			yMultiplier = 1.0 / (1.0 + lambda * oneOverRadiiSquaredY);
			zMultiplier = 1.0 / (1.0 + lambda * oneOverRadiiSquaredZ);

			xMultiplier2 = xMultiplier * xMultiplier;
			yMultiplier2 = yMultiplier * yMultiplier;
			zMultiplier2 = zMultiplier * zMultiplier;

			xMultiplier3 = xMultiplier2 * xMultiplier;
			yMultiplier3 = yMultiplier2 * yMultiplier;
			zMultiplier3 = zMultiplier2 * zMultiplier;

			func = x2 * xMultiplier2 + y2 * yMultiplier2 + z2 * zMultiplier2 - 1.0;

			// "denominator" here refers to the use of this expression in the velocity and acceleration
			// computations in the sections to follow.
			denominator = x2 * xMultiplier3 * oneOverRadiiSquaredX + y2 * yMultiplier3 * oneOverRadiiSquaredY + z2 * zMultiplier3 * oneOverRadiiSquaredZ;

			auto derivative = -2.0 * denominator;

			correction = func / derivative;
		} while (abs(func) > WGS84Ellipsoid::EPSILON12);

		Cartesian3 result;
		result.x = positionX * xMultiplier;
		result.y = positionY * yMultiplier;
		result.z = positionZ * zMultiplier;
		return result;
	}
	void Cartesian3::toRadians(double& longitude, double& latitude, double& height) {
		Cartesian3 cartesian = *this;
		constexpr auto& oneOverRadii = WGS84OneOverRadii;
		constexpr auto& oneOverRadiiSquared = WGS84OneOverRadiiSquared;
		constexpr auto& centerToleranceSquared = WGS84CenterToleranceSquared;
		auto p = scaleToGeodeticSurface(cartesian, oneOverRadii, oneOverRadiiSquared, centerToleranceSquared);

		auto n = multiplyComponents(p, oneOverRadiiSquared);
		n = normalize(n);

		auto h = subtract(cartesian, p);

		longitude = atan2(n.y, n.x);
		latitude = asin(n.z);
		height = sign(dot(h, cartesian)) * magnitude(h);
		return;
	}
	void Cartesian3::toDegrees(double& longitude, double& latitude, double& height) {
		toRadians(longitude, latitude, height);
		longitude = longitude * 180 / Constant::PI;
		latitude = latitude * 180 / Constant::PI;
	}

	Cartesian3 Cartesian3::subtract(Cartesian3 left, Cartesian3 right) {
		Cartesian3 result;

		result.x = left.x - right.x;
		result.y = left.y - right.y;
		result.z = left.z - right.z;
		return result;
	};

	Cartesian3 Cartesian3::cross(Cartesian3 left, Cartesian3 right) {
		Cartesian3 result;

		auto leftX = left.x;
		auto leftY = left.y;
		auto leftZ = left.z;
		auto rightX = right.x;
		auto rightY = right.y;
		auto rightZ = right.z;

		auto x = leftY * rightZ - leftZ * rightY;
		auto y = leftZ * rightX - leftX * rightZ;
		auto z = leftX * rightY - leftY * rightX;

		result.x = x;
		result.y = y;
		result.z = z;
		return result;
	};

	Cartesian3 Cartesian3::computeHorizonCullingPointFromVertices(Cartesian3 directionToPoint, double* vertices, int elementSize, int stride, Cartesian3 center) {
		auto scaledSpaceDirectionToPoint = computeScaledSpaceDirectionToPoint(directionToPoint);
		auto resultMagnitude = 0.0;

		Cartesian3 positionScratch;

		for (int i = 0; i < elementSize; i += stride) {
			positionScratch.x = vertices[i] +     center.x;
			positionScratch.y = vertices[i + 1] + center.y;
			positionScratch.z = vertices[i + 2] + center.z;

			double candidateMagnitude = computeMagnitude(positionScratch, scaledSpaceDirectionToPoint);
			resultMagnitude = fmax(resultMagnitude, candidateMagnitude);
		}

		return magnitudeToPoint(scaledSpaceDirectionToPoint, resultMagnitude);
	};

	Cartesian3 Cartesian3::magnitudeToPoint(Cartesian3 scaledSpaceDirectionToPoint, double resultMagnitude) {
		// The horizon culling point is undefined if there were no positions from which to compute it,
		// the directionToPoint is pointing opposite all of the positions,  or if we computed NaN or infinity.
		if (resultMagnitude <= 0.0 || ::isnan(resultMagnitude)) {
			return Cartesian3();
		}

		return  multiplyByScalar(scaledSpaceDirectionToPoint, resultMagnitude);
	}

	Cartesian3  Cartesian3::computeScaledSpaceDirectionToPoint(Cartesian3 directionToPoint) {
		Cartesian3 directionToPointScratch = transformPositionToScaledSpace(directionToPoint);
		return   normalize(directionToPointScratch);
	}

	Cartesian3 Cartesian3::transformPositionToScaledSpace(Cartesian3 directionToPoint) {
		return multiplyComponents(directionToPoint, WGS84OneOverRadii);
	}

	Cartesian3 Cartesian3::transformScaledSpaceToPosition(Cartesian3 directionToPoint) {
		return multiplyComponents(directionToPoint, WGS84Radii);
	}

	double Cartesian3::computeMagnitude(Cartesian3 position, Cartesian3 scaledSpaceDirectionToPoint) {
		auto scaledSpacePosition = transformPositionToScaledSpace(position);
		auto magnitudeSquared = Cartesian3::magnitudeSquared(scaledSpacePosition);
		auto magnitude = sqrt(magnitudeSquared);
		auto direction = divideByScalar(scaledSpacePosition, magnitude);

		// For the purpose of this computation, points below the ellipsoid are consider to be on it instead.
		magnitudeSquared = fmax(1.0, magnitudeSquared);
		magnitude = fmax(1.0, magnitude);

		auto cosAlpha = dot(direction, scaledSpaceDirectionToPoint);
		auto sinAlpha = Cartesian3::magnitude(cross(direction, scaledSpaceDirectionToPoint));
		auto cosBeta = 1.0 / magnitude;
		auto sinBeta = sqrt(magnitudeSquared - 1.0) * cosBeta;

		return 1.0 / (cosAlpha * cosBeta - sinAlpha * sinBeta);
	}

	Cartesian3  Cartesian3::geodeticSurfaceNormal(Cartesian3 cartesian)
	{
		auto result = Cartesian3::multiplyComponents(cartesian, WGS84OneOverRadiiSquared);
		return Cartesian3::normalize(result);
	}

    osg::Matrixd Cartesian3::eastNorthUpMatrixWorld(const osg::Vec3d& center) {
        Cartesian3 pos(center.x(), center.y(), center.z());

        auto up = Cartesian3::geodeticSurfaceNormal(pos);
        Cartesian3 east;
        east.x = -pos.y;
        east.y = pos.x;
        east.z = 0;
        east = Cartesian3::normalize(east);
        auto north = Cartesian3::cross(up, east);
        auto mat = osg::Matrixd::identity();

        mat(0, 0) = east.x;
        mat(0, 1) = east.y;
        mat(0, 2) = east.z;
        mat(0, 3) = 0.0;
        mat(1, 0) = north.x;
        mat(1, 1) = north.y;
        mat(1, 2) = north.z;
        mat(1, 3) = 0.0;
        mat(2, 0) = up.x;
        mat(2, 1) = up.y;
        mat(2, 2) = up.z;
        mat(2, 3) = 0.0;
        mat(3, 0) = pos.x;
        mat(3, 1) = pos.y;
        mat(3, 2) = pos.z;
        mat(3, 3) = 1.0;

        return mat;
    }

    osg::Matrixd Cartesian3::eastNorthUpMatrix(const osg::Vec3d& center) {
        auto pos = Cartesian3::fromDegree(center.x(), center.y(), center.z());
        auto v = osg::Vec3d(pos.x, pos.y, pos.z);
        return eastNorthUpMatrixWorld(v);
    }

	const Cartesian3 Cartesian3::WGS84Radii = Cartesian3(
		WGS84Ellipsoid::RX,
		WGS84Ellipsoid::RY,
		WGS84Ellipsoid::RZ);
	const Cartesian3 Cartesian3::WGS84RadiiSquared = Cartesian3(
		WGS84Ellipsoid::RX * WGS84Ellipsoid::RX,
		WGS84Ellipsoid::RY * WGS84Ellipsoid::RY,
		WGS84Ellipsoid::RZ * WGS84Ellipsoid::RZ);
	const Cartesian3 Cartesian3::WGS84OneOverRadii = Cartesian3(
		1 / WGS84Ellipsoid::RX,
		1 / WGS84Ellipsoid::RY,
		1 / WGS84Ellipsoid::RZ);
	const Cartesian3 Cartesian3::WGS84OneOverRadiiSquared = Cartesian3(
		1 / (WGS84Ellipsoid::RX * WGS84Ellipsoid::RX),
		1 / (WGS84Ellipsoid::RY * WGS84Ellipsoid::RY),
		1 / (WGS84Ellipsoid::RZ * WGS84Ellipsoid::RZ));
}
