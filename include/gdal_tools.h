#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QException>
#include <ogr_spatialref.h>

using OGRCoordinateTransformationPtr = QSharedPointer<OGRCoordinateTransformation>;

class OGRException : QException {
public:
	OGRException(OGRErr err) : error(err) {}
	virtual const char* what() const override;
private:
	OGRErr error;
};

class CoordinateConvert: QObject{
	Q_OBJECT
public:
	double sourceX;
	double sourceY;
	double targetX;
	double targetY;

	enum SrsType {
		WKT,
		EPSG,
		Proj4
	};
	Q_ENUM(SrsType);

	CoordinateConvert(double x, double y) : sourceX(x), sourceY(y) {}
	CoordinateConvert(): CoordinateConvert(0, 0) {}

	void setSourceSrs(const QString& srs, SrsType t) throw(OGRException);
	void setTargetSrs(const QString& srs, SrsType t) throw(OGRException);
	void transform() throw(OGRException);
private:
	void setSrs(OGRSpatialReference& srs, const QString& describe, SrsType t) throw(OGRException);
	OGRCoordinateTransformationPtr createCoordinateTransformation() throw(OGRException);
private:
	OGRSpatialReference sourceSrs;
	OGRSpatialReference targetSrs;
};