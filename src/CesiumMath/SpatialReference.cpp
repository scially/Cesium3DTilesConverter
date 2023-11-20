#include <CesiumMath/SpatialReference.h>
#include <CesiumMath/Cartesian3.h>

#include <QtDebug>
#include <QMutex>
#include <QMutexLocker>

namespace scially {
	
	QSharedPointer<SpatialReference> SpatialReference::create(const QString& inSrs, const osg::Vec3d& origin){
		if (inSrs.isEmpty())
			return nullptr;

		if (inSrs.indexOf("ENU:") == 0) {
			auto srs = QSharedPointer<SpatialReferenceMatrix>::create();
			if(srs->init(inSrs, origin))
				return srs;
		}
		else {
			auto srs = QSharedPointer<SpatialReferenceProj>::create();
			if (srs->init(inSrs, origin))
				return srs;
		}
		return nullptr;
	}

	osg::Vec3d SpatialReferenceProj::toWorld(osg::Vec3d v) const
	{
		static QMutex mutex;

		if (!mTransfromer)
			return osg::Vec3d(0, 0, 0);

		auto  projPoint = mOrigin + v;

		QMutexLocker locker(&mutex);
		
		// WGS84
		if (mTransfromer->Transform(1, &projPoint.x(), &projPoint.y(), 0) == 0)
			return osg::Vec3d(0, 0, 0);

		// Geodetic
		auto gp = Cartesian3::fromDegree(projPoint.x(), projPoint.y(), projPoint.z());
		return osg::Vec3d(gp.x, gp.y, gp.z);
	}

	osg::Vec3f SpatialReferenceProj::toWorldNormal(osg::Vec3d v, osg::Vec3f n) const
	{
        // v0 ----> n0
        // v1 ----> n1
        // v1 - v0 ---- > n1-n0
		auto p0 = toWorld(v);
		auto v1 = v + (n * 10);
		auto p1 = toWorld(v1);

		auto ret = p1 - p0;

		ret.normalize();
		return ret;
	}

	bool SpatialReferenceProj::init(const QString& inSrs, osg::Vec3d origin)
	{
		mOrigin = origin;
		OGRSpatialReference srf;
		if (!importFromStr(srf, inSrs)) {
            qCritical() << "parse crs" << inSrs << "failed";
			return false;
		}

		srf.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRSpatialReference t4326;
		t4326.importFromEPSG(4326);
		t4326.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		mTransfromer = OGRCreateCoordinateTransformation(&srf, &t4326);
		if (!mTransfromer) {
			qCritical() << inSrs << "can't transform to WGS84";
			return false;
		}
		return true;
	}

	bool SpatialReferenceProj::importFromStr(OGRSpatialReference& srf, const QString& srs)
	{
		std::string stdSRS = srs.toStdString();
		char* cSRS = const_cast<char*>(stdSRS.c_str());
		if (srs.indexOf("EPSG") == 0)
		{
			bool ret = OSRSetFromUserInput(&srf, cSRS);
			if (!ret) {
				qCritical() << "OSRSetFromUserInput failed£º" << srs;
				return false;
			}

			return true;
		}
		else
		{
			//try wkt
			auto ret = OSRImportFromWkt(&srf, &cSRS);
			if (!ret) {
				// try esri prj
				ret = OSRMorphFromESRI(&srf);
				if (!ret) 
					return true;
			}
	
			//try proj4
			ret = OSRImportFromProj4(&srf, srs.toStdString().c_str());
			if (!ret) 
				return true;

			//try esri prj
			ret = OSRImportFromESRI(&srf, &cSRS);
			if (!ret) 
				return true;

			return false;
		}
	}


	bool SpatialReferenceMatrix::init(const QString& inSrs, osg::Vec3d origin)
	{
		QStringList srs = inSrs.mid(4).split(",");
		if (srs.length() != 2) {
			qCritical() << "parse enu:" << inSrs << "failed";
			return false;
		}

		double lat = srs[0].toDouble();
		double lon = srs[1].toDouble();

		osg::Vec3d llh(lon, lat, 0);

        mENUMatrix = Cartesian3::eastNorthUpMatrix(llh);
		
        // enu point
		auto v = osg::Vec3d(0, 0, 0);
		auto g = toWorld(v);
        mZeroPointENU = Cartesian3::eastNorthUpMatrixWorld(g);

		return true;
	}

	osg::Vec3d SpatialReferenceMatrix::toWorld(osg::Vec3d v) const
	{
		auto p = mOrigin + v;
        p = p * mENUMatrix;

		return p;
	}

	osg::Vec3f SpatialReferenceMatrix::toWorldNormal(osg::Vec3d v, osg::Vec3f n) const
	{
        // enuMatrix * n
        return osg::Matrixd::transform3x3(mENUMatrix, n);
	}

    void SpatialReferenceMatrix::initWithCartesian3(const osg::Vec3d& v)
	{
        mENUMatrix = Cartesian3::eastNorthUpMatrixWorld(v);
        mZeroPointENU = mENUMatrix;
	}
}

