#pragma once
#include <OGRException.h>

namespace gzpi {
	
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