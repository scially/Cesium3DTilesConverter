#pragma once

#include <gdal_tools.h>


const char* OGRException::what() const {
	switch (error) {
	case OGRERR_NOT_ENOUGH_DATA: 
		return "Not enough data";
		break;
	case OGRERR_NOT_ENOUGH_MEMORY:
		return "Not enough memory";
		break;
	case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
		return "Unsupported geometry type";
		break;
	case OGRERR_UNSUPPORTED_OPERATION:
		return "Unsupported operation";
		break;
	case OGRERR_CORRUPT_DATA:
		return "Corrupt data";
		break;
	case OGRERR_FAILURE:
		return "Failure";
		break;
	case OGRERR_UNSUPPORTED_SRS:
		return "Unsupported srs";
		break;
	case OGRERR_INVALID_HANDLE:
		return "Invalid handle";
		break;
	case OGRERR_NON_EXISTING_FEATURE:
		return "Non existing feataure";
		break;
	default:
		return "Unkonwn error";
	}
}


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

OGRCoordinateTransformationPtr CoordinateConvert::createCoordinateTransformation() throw(OGRException) {
	OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&sourceSrs, &targetSrs);
	if (transform == nullptr)
		throw OGRException(OGRERR_UNSUPPORTED_SRS);

	return OGRCoordinateTransformationPtr(transform);
}