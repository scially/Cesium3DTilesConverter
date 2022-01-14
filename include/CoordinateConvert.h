#pragma once
#include <OGRException.h>

namespace gzpi {
	
	class CoordinateConvert : QObject {
		Q_OBJECT
	public:
		using OGRCoordinateTransformationPtr = QSharedPointer<OGRCoordinateTransformation>;

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
		CoordinateConvert() : CoordinateConvert(0, 0) {}

		void setSourceSrs(const QString& srs, SrsType t) throw(OGRException);
		void setTargetSrs(const QString& srs, SrsType t) throw(OGRException);
		void transform() throw(OGRException);
	private:
		void setSrs(OGRSpatialReference& srs, const QString& describe, SrsType t) ;
		OGRCoordinateTransformationPtr createCoordinateTransformation();
	private:
		OGRSpatialReference sourceSrs;
		OGRSpatialReference targetSrs;
	};
}