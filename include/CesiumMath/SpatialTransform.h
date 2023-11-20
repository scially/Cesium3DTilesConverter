#pragma once

#include "SpatialReference.h"

#include <QSharedPointer>
#include <osg/Matrixd>

namespace scially {
	class SpatialTransform
	{
	public:
		using Ptr = QSharedPointer<SpatialTransform>;

		static SpatialTransform::Ptr create(
            const SpatialReference& in,
            const SpatialReference& out);

		virtual	osg::Vec3d transform(const osg::Vec3d& v) const = 0 ;
		virtual osg::Vec3f transfromNormal(const osg::Vec3d& v, const osg::Vec3f& n) const = 0;
	};

	class SpatialTransformEnuEnu :public SpatialTransform
	{
	public:
		SpatialTransformEnuEnu(
			const SpatialReferenceMatrix& in,
			const SpatialReferenceMatrix& out);

		virtual osg::Vec3d transform(const osg::Vec3d& v) const override;
		virtual osg::Vec3f transfromNormal(const osg::Vec3d& v, const osg::Vec3f& n) const override;

	private:
		osg::Matrixd mMat;
	};


	class SpatialTransformProjEnu :public SpatialTransform
	{
	public:
		SpatialTransformProjEnu(
			const SpatialReferenceProj& in,
			const SpatialReferenceMatrix& out);

		virtual osg::Vec3d transform(const osg::Vec3d& v) const override;
		virtual osg::Vec3f transfromNormal(const osg::Vec3d& v, const osg::Vec3f& n) const override;
	
	private:
		SpatialReferenceProj mInSrs;
		osg::Matrixd mMat;
	};
}
