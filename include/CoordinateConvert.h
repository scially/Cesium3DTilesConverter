#pragma once

#include <GDALWrapper.h>
#include <OGRException.h>
#include <osg/Math>
namespace scially {

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
            Proj4,
		};
		Q_ENUM(SrsType);

		CoordinateConvert(double x, double y) : sourceX(x), sourceY(y), targetX(0), targetY(0) {
            static internal::GDALDriverWrapper init;
        }
		CoordinateConvert() : CoordinateConvert(0, 0) {  }

		void setSourceSrs(const QString& srs, SrsType t) ;
		void setTargetSrs(const QString& srs, SrsType t);
		void transform() ;

	private:
		void setSrs(OGRSpatialReference& srs, const QString& describe, SrsType t) ;
		OGRCoordinateTransformationPtr createCoordinateTransformation();
        void init() const {
            const char* projResource = "proj_data";
            proj_context_set_search_paths(nullptr, 1, &projResource);
        }
	private:
		OGRSpatialReference sourceSrs;
		OGRSpatialReference targetSrs;
	};
}
