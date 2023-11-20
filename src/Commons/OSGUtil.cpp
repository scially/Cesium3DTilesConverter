#include <Commons/OSGUtil.h>

namespace scially
{
	BoundingVolumeBox osgBoundingToCesiumBoundBox(const osg::BoundingBoxd &bound)
	{
		BoundingVolumeBox box;

		box.centerX = bound.center().x();
		box.centerY = bound.center().y();
		box.centerZ = bound.center().z();

		const double wx = bound.xMax() - bound.xMin();
		const double wy = bound.yMax() - bound.yMin();
		const double wz = bound.zMax() - bound.zMin();

		box.directionX0 = 0;
		box.directionX1 = 0;
		box.halfXLength = wx * 0.5;

		box.directionY0 = 0;
		box.directionY1 = 0;
		box.halfYLength = wy * 0.5;

		box.directionZ0 = 0;
		box.directionZ1 = 0;
		box.halfZLength = wz * 0.5;

		return box;
	}

	QVector<double> osgMatrixToCesiumTransform(const osg::Matrixd &matrix)
	{
		QVector<double> transform;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				transform.append(matrix(i, j));
			}
		}

		return transform;
	}

	double osgBoundingSize(const osg::BoundingBoxd& box){
		const double dx = box.xMax() - box.xMin();
		const double dy = box.yMax() - box.yMin();
		const double dz = box.zMax() - box.zMin();

		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}
}
