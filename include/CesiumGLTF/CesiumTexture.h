#pragma once

#include <QByteArray>
#include <QSharedPointer>
#include <QString>

#include <osg/Image>
#include <tiny_gltf.h>

namespace scially {

    // always 3 components
    class CesiumTexture {
    public:
		using Ptr = QSharedPointer<CesiumTexture>;

		int width = 0;
		int height = 0;
		QString name;
		QByteArray data;

		bool importFromOSGImage(const osg::Image* image);
		bool toGltfImage(tinygltf::Image& image) const;
		void makeFakeTexture(char fill);
	};
}
