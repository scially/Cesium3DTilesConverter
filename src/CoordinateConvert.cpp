#include <CoordinateConvert.h>

namespace gzpi {
	void CoordinateConvert::setSourceSrs(const QString& srs, SrsType t) throw(OGRException) {
		setSrs(sourceSrs, srs, t);
	}

	void CoordinateConvert::setTargetSrs(const QString& srs, SrsType t) throw(OGRException) {
		setSrs(targetSrs, srs, t);
	}

	void CoordinateConvert::transform() throw(OGRException) {
		auto transform = createCoordinateTransformation();
		double x = sourceX, y = sourceY;
		bool succeed = transform->Transform(1, &x, &y);
		if (!succeed)
			throw OGRException(OGRERR_FAILURE);
		targetX = x;
		targetY = y;
	}

	void CoordinateConvert::setSrs(OGRSpatialReference& srs, const QString& describe, SrsType t) throw(OGRException) {
		OGRErr err = OGRERR_UNSUPPORTED_SRS;
		switch (t)
		{
		case CoordinateConvert::WKT:
			err = srs.importFromWkt(describe.toStdString().c_str());
			break;
		case CoordinateConvert::EPSG:
			err = srs.importFromEPSG(describe.toInt());
			break;
		case CoordinateConvert::Proj4:
			err = srs.importFromProj4(describe.toStdString().c_str());
			break;
		default:
			break;
		}

		if (err != OGRERR_NONE)
			throw OGRException(err);
	}

	CoordinateConvert::OGRCoordinateTransformationPtr CoordinateConvert::createCoordinateTransformation() {
		OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&sourceSrs, &targetSrs);
		if (transform == nullptr)
			throw OGRException(OGRERR_UNSUPPORTED_SRS);

		return OGRCoordinateTransformationPtr(transform);
	}
}