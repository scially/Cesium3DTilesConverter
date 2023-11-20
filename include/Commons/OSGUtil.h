#pragma once

#include <Cesium3DTiles/BoundingVolume.h>

#include <osg/BoundingBox>
#include <osg/Matrixd>

#include <QVector>

namespace scially {
	BoundingVolumeBox osgBoundingToCesiumBoundBox(const osg::BoundingBoxd& bound);
	QVector<double> osgMatrixToCesiumTransform(const osg::Matrixd& matrix);
	double osgBoundingSize(const osg::BoundingBoxd& box);

}