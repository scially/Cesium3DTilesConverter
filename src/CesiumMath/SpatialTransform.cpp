#include <CesiumMath/SpatialTransform.h>

#include <QtDebug>

namespace scially {
	QSharedPointer<SpatialTransform> SpatialTransform::create(
        const SpatialReference& in,
        const SpatialReference& out) {
		
        auto outSRS = dynamic_cast<const SpatialReferenceMatrix*>(&out);
		if (!outSRS) {
			qCritical("don't support output srs");
			return nullptr;
		}

        auto inSRSMatrix = dynamic_cast<const SpatialReferenceMatrix*>(&in);
        auto inSRSProj = dynamic_cast<const SpatialReferenceProj*>(&in);
		if (!inSRSMatrix && !inSRSProj) {
			qCritical("don't support output srs");
			return nullptr;
		}

		if (inSRSMatrix)
		{
			auto ret = QSharedPointer<SpatialTransformEnuEnu>::create(
				*inSRSMatrix, 
				*outSRS);
			return ret;
		}
		else {
			auto ret = QSharedPointer<SpatialTransformProjEnu>::create(
				*inSRSProj, 
				*outSRS);
			return ret;
		}
	}

	SpatialTransformEnuEnu::SpatialTransformEnuEnu(
		const SpatialReferenceMatrix& in,
		const SpatialReferenceMatrix& out) {

		// algorithm
		// (v + in_srsOrigin) * in_enuMatrix * (out_enuMatrix^) - out_srsOrigin
		osg::Matrixd inOrigin;
		inOrigin.makeTranslate(in.origin());

		osg::Matrixd outEnuReverse = osg::Matrixd::inverse(out.enuMatrix());
		osg::Matrixd outOrigin;
		outOrigin.makeTranslate(-out.origin());

		mMat = inOrigin * in.enuMatrix() * outEnuReverse * outOrigin;
	}

	osg::Vec3d SpatialTransformEnuEnu::transform(const osg::Vec3d& v) const
	{
		return mMat.preMult(v);
	}

	osg::Vec3f SpatialTransformEnuEnu::transfromNormal(
		const osg::Vec3d& v, 
		const osg::Vec3f& n) 
		const
	{
		auto nn = osg::Matrixd::transform3x3(n, mMat);
		nn.normalize();
		return nn;
	}

	SpatialTransformProjEnu::SpatialTransformProjEnu(
		const SpatialReferenceProj& in,
		const SpatialReferenceMatrix& out) :mInSrs(in)
	{
		osg::Matrixd outEnuReverse = osg::Matrixd::inverse(out.enuMatrix());
		osg::Matrixd outOrigin;
		outOrigin.makeTranslate(-out.origin());

		mMat = outEnuReverse * outOrigin;
	}

	osg::Vec3d SpatialTransformProjEnu::transform(const osg::Vec3d& v) const
	{
		auto ret = mInSrs.toWorld(v);
        return mMat.preMult(ret);
	}

	osg::Vec3f SpatialTransformProjEnu::transfromNormal(
		const osg::Vec3d& v, 
		const osg::Vec3f& n)
		const
	{
		auto ret = mInSrs.toWorldNormal(v, n);
		auto nn = osg::Matrixd::transform3x3(ret, mMat);
		nn.normalize();
		return nn;
	}
}
