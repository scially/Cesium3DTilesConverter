#include "OSGParseVisitor.h"

#include <osg/Node>
#include <osg/Geode>
#include <osg/PagedLOD>
#include <osg/StateSet>
#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>

#include <QtDebug>

namespace scially {
	void OSGParseVisitor::apply(osg::Node& node)  {
		traverse(node);
	}

	void OSGParseVisitor::apply(osg::Geode& geode) {
		for (size_t i = 0; i < geode.getNumDrawables(); i++) {
			auto mesh = createMesh(geode.getDrawable(i));

            if (mesh.has_value()) {
                if (!mesh->texcoords.isEmpty()) {
                    meshes.append(*mesh);
				}
			}
		}
	}

	void OSGParseVisitor::apply(osg::PagedLOD& lod) {
		if (!mSkipLOD) {
			apply(static_cast<osg::Node&>(lod));
		}
	}

    std::optional<CesiumMaterial> OSGParseVisitor::createMaterial(const osg::StateSet* ss) {
		if (ss == nullptr)
            return std::nullopt;

		const osg::Texture2D* texture = dynamic_cast<const osg::Texture2D*>(
			ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE)
			);

		if (texture == nullptr)
            return std::nullopt;

		auto img = texture->getImage();
		if (img == nullptr) {
			qWarning("image in texture of model is empty");
            return std::nullopt;
		}

		if (!mOSGMaterialToCesiumMaterial.contains(img)) {
            CesiumTexture texture;
            if (texture.importFromOSGImage(img)) {
                CesiumMaterial material;
                material.baseColorTexture = texture;

				mOSGMaterialToCesiumMaterial[img] = material;
				return material;
			}
		}
		
		return mOSGMaterialToCesiumMaterial[img];
		
	}

    std::optional<CesiumMesh> OSGParseVisitor::createMesh(const osg::Drawable* draw) {
		if (draw == nullptr)
            return std::nullopt;

		const osg::Geometry* geom = draw->asGeometry();
		if (geom == nullptr)
            return std::nullopt;

		const osg::MatrixList mats = geom->getWorldMatrices();
		const osg::Matrix mat = mats.empty() ? osg::Matrix::identity() : mats[0];
		const osg::Array* vertex = geom->getVertexArray();
		if (vertex == nullptr)
            return std::nullopt;

        CesiumMesh rmesh;
		// vertex copy
		{
			uint32_t vertexCount = vertex->getNumElements();
            rmesh.vertex.resize(vertexCount);
            std::memcpy(rmesh.vertex.data()
				, vertex->getDataPointer()
				, vertex->getTotalDataSize());

			bool needTransform = !mat.isIdentity();
            for (auto& v : rmesh.vertex) {
				if (needTransform) {
					v = v * mat;
				}

				if (mTransformer) {
					v = mTransformer->transform(v);
					v -= mTileCenter;
				}
			}
		}

		// normal copy
		{
			const osg::Array* normals = geom->getNormalArray();
			if (!normals && !mNoLight) {
				osg::Geometry* geomWithSmoothNoramals = const_cast<osg::Geometry*>(geom);
				osgUtil::SmoothingVisitor::smooth(*geomWithSmoothNoramals);
				normals = geom->getNormalArray();
			}

			if (normals) {
                rmesh.normals.resize(normals->getNumElements());
                std::memcpy(rmesh.normals.data()
					, normals->getDataPointer()
					, normals->getTotalDataSize());
			}
		}

		// texture coord copy
		{
			const osg::Array* uvs = geom->getTexCoordArray(0);
			if (uvs) {
                rmesh.texcoords.resize(uvs->getNumElements());
				switch(uvs->getType()) {
				case osg::Array::Vec2ArrayType:
                    std::memcpy(rmesh.texcoords.data()
						, uvs->getDataPointer()
						, uvs->getTotalDataSize());
					break;
				default:
					auto p = static_cast<const uchar*>(uvs->getDataPointer());
					for (size_t i = 0; i < uvs->getNumElements(); i++) {
						const float* uv = reinterpret_cast<const float*>(p + uvs->getElementSize() * i);
                        rmesh.texcoords[i].x() = *uv;
                        rmesh.texcoords[i].y() = *(uv + 1);
					}
					break;
				}
			}
		}

		// vertex index copy 
		{
			for (size_t i = 0; i < geom->getNumPrimitiveSets(); i++) {
				const osg::PrimitiveSet* primitive = geom->getPrimitiveSet(i);
				if (primitive == nullptr)
					continue;

				switch (primitive->getMode()) {
				case GL_TRIANGLES:
					for (size_t i = 0; i < primitive->getNumIndices() / 3; i++) {
                        const uint32_t i0 = primitive->index(i * 3);
                        const uint32_t i1 = primitive->index(i * 3 + 1);
                        const uint32_t i2 = primitive->index(i * 3 + 2);

                        rmesh.indices.append(i2);
                        rmesh.indices.append(i1);
                        rmesh.indices.append(i0);
					}
					break;
				case GL_TRIANGLE_STRIP:
					for (uint32_t i = 0; i < primitive->getNumIndices() - 2; i++) {
                        const uint32_t i0 = primitive->index(i);
                        const uint32_t i1 = primitive->index(i + 1);
                        const uint32_t i2 = primitive->index(i + 2);

						if (i % 2 == 1) {
                            rmesh.indices.append(i1);
                            rmesh.indices.append(i0);
                            rmesh.indices.append(i2);
						}
						else {
                            rmesh.indices.append(i0);
                            rmesh.indices.append(i1);
                            rmesh.indices.append(i2);
						}
					}
					break;
				default:
					qCritical("primitive mode isn't triangle");
					continue;
				}
			}
		}

		// material create 
		{
            auto material = createMaterial(draw->getStateSet());

            if (!material.has_value())
                material.emplace();

            if (!material->baseColorTexture.has_value()) {
                material->baseColorTexture.emplace();
                material->baseColorTexture->width = 4;
                material->baseColorTexture->height = 4;
                material->baseColorTexture->makeFakeTexture(255);
			}

            rmesh.material = *material;
		}

		return rmesh;
	}

	QList<CesiumMesh> OSGBtoCesiumMesh(
		osg::Node& osgNode,
		const osg::Vec3d& tileCenter,
		const SpatialTransform& transform) {

		OSGParseVisitor loader(tileCenter, &transform);
		osgNode.accept(loader);
		return loader.meshes;
	}
}
