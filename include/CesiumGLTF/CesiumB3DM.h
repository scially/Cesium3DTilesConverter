#pragma once

#include "CesiumMesh.h"

#include <osg/Vec3d>

#include <QByteArray>
#include <QSharedPointer>

namespace scially {
	class CesiumB3DM {
	public:
		using Ptr = QSharedPointer<CesiumB3DM>;
		// convert all mesh to glb and no package texture
		bool toGltfBinaryWithNoPack(QByteArray& buffer) const;
		
		osg::Vec3d center;
        QList<CesiumMesh> meshes;
	private:
		void toB3DM(const QByteArray& glb, QByteArray& buffer) const;
	};
	
}
