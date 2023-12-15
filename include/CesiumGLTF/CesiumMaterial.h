#pragma once

#include "CesiumTexture.h"

#include <osg/Vec4f>

#include <QSharedPointer>
#include <QString>

#include <optional>

namespace scially {
	struct CesiumMaterial {
		using Ptr = QSharedPointer<CesiumMaterial>;

		osg::Vec4f baseColorFactor = osg::Vec4f(1, 1, 1, 1);
		float metallicFactor = 0;
		float roughnessFactor = 0.5;
        std::optional<CesiumTexture> baseColorTexture;
        std::optional<CesiumTexture> normalTexture;
		float  alphaCutoff = 0.5;
		bool   doubleSided = true;
	};
}
