#include <CesiumGLTF/CesiumTexture.h>

#include <stb_image.h>
#include <stb_image_write.h>
#include <osg/Texture>

#include <QtDebug>

namespace scially {
	bool CesiumTexture::importFromOSGImage(const osg::Image* image) {
        data.clear();
		width = image->s();
		height = image->t();

		const GLenum format = image->getPixelFormat();
		const char* rgb = (const char*)(image->data());
		uint32_t rowStep = image->getRowStepInBytes();
		uint32_t rowSize = image->getRowSizeInBytes();
		switch (format)
		{
		case GL_RGBA:
            data.resize(width * height * 3);
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					data[i * width * 3 + j * 3] = rgb[i * width * 4 + j * 4];
					data[i * width * 3 + j * 3 + 1] = rgb[i * width * 4 + j * 4 + 1];
					data[i * width * 3 + j * 3 + 2] = rgb[i * width * 4 + j * 4 + 2];
				}
			}
			return true;
		case GL_RGB:
			for (int i = 0; i < height; i++)
			{
				data.append(rgb + rowStep * i, rowSize);
			}
			return true;

		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			qCritical("don't support osg dds texture format");
			return false;
		default:
			return false;
		}
	}

	bool CesiumTexture::toGltfImage(tinygltf::Image &image) const{
		int r = stbi_write_jpg_to_func(
            [](void* context, void* data, int size) {
				tinygltf::Image& img = *(tinygltf::Image*)context;
				img.image.insert(img.image.end(), (char*)data, (char*)data + size);
			},
            &image, width, height, 3, data.data(), 90);
		
		image.mimeType = "image/jpeg";
		return r > 0;
	}

	void CesiumTexture::makeFakeTexture(char fill) {
        data.resize(width * height * 3);
		data.fill(fill);
	}
}
