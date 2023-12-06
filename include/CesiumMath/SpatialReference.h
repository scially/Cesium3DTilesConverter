#pragma once

#include <osg/Vec3d>
#include <osg/Vec3f>
#include <osg/Matrixd>

#include <QSharedPointer>

#include <ogrsf_frmts.h>

namespace scially {
	class SpatialReference {
	public:
		using Ptr = QSharedPointer<SpatialReference>;
		static SpatialReference::Ptr CreateSpatialReference(const QString& inSrs, const osg::Vec3d& origin);
		
		virtual osg::Vec3d toWorld(osg::Vec3d v) const = 0;
		virtual osg::Vec3f toWorldNormal(osg::Vec3d v, osg::Vec3f n) const = 0;

		const osg::Vec3d& origin() const { return mOrigin; }
		osg::Vec3d& origin() { return mOrigin; }

		const osg::Matrixd& originENU() const { return mZeroPointENU; }
		osg::Matrixd& originENU() { return mZeroPointENU; }

	protected:
		osg::Vec3d mOrigin;
		osg::Matrixd mZeroPointENU;
	};

	class SpatialReferenceProj: public SpatialReference {
	public:
		virtual osg::Vec3d toWorld(osg::Vec3d v) const;
		virtual osg::Vec3f toWorldNormal(osg::Vec3d v, const osg::Vec3f n) const;

		bool init(const QString& inSrs, osg::Vec3d origin);
	private:
		bool importFromStr(OGRSpatialReference& srf, const QString& str);

		OGRCoordinateTransformation* mTransfromer = nullptr;
	};

	class SpatialReferenceMatrix : public SpatialReference {
	public:
		virtual osg::Vec3d toWorld(osg::Vec3d v) const;
		virtual osg::Vec3f toWorldNormal(osg::Vec3d v, osg::Vec3f n) const;

		bool init(const QString& inSrs, osg::Vec3d origin);
        void initWithCartesian3(const osg::Vec3d& c);

        const osg::Matrixd& enuMatrix() const { return mENUMatrix; }

    protected:
        osg::Matrixd mENUMatrix;
	};

	
}
