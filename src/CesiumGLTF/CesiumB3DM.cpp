#include <CesiumGLTF/CesiumB3DM.h>
#include <CesiumGLTF/CesiumTexture.h>

#include <tiny_gltf.h>

#include <QSet>
#include <QLatin1String>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>
#include <QtDebug>

#include <sstream>

namespace scially {
	class TinyGLTFConvert {
	public:
		struct TextureResult {
			int index = -1;
		};

        // material result
		struct  MaterialResult {
			int index = -1;
		};

		tinygltf::Model gltfModel;
		tinygltf::Mesh gltfMesh;
		tinygltf::Buffer gltfBuffer;

		TinyGLTFConvert() {
			gltfModel.asset.generator = "Cesium3DTilesConverter";
			gltfModel.asset.version = "2.0";
		}

		void add(const CesiumMesh& m) {
			mBaseTextureIndex.index = -1;
			mNormalTextureIndex.index = -1;

            if (m.material.baseColorTexture.has_value()) {
                mBaseTextureIndex = saveGltfTexture(*(m.material.baseColorTexture));
			}
            if (m.material.normalTexture.has_value()) {
                mNormalTextureIndex = saveGltfTexture(*(m.material.normalTexture));
			}

            addGltfMaterial(m.material);
			addGltfMesh(m);
		}

		bool save(QByteArray& buffer) {
			foreach(const auto & extension, extensionsRequired) {
				gltfModel.extensionsRequired.push_back(extension.toStdString());
			}
			foreach(const auto & extension, extensionsUsed) {
				gltfModel.extensionsUsed.push_back(extension.toStdString());
			}

			gltfModel.buffers.push_back(gltfBuffer);
			gltfModel.meshes.push_back(gltfMesh);

			// node
			tinygltf::Node node;
			// to yup axis
			node.matrix = { 1,0, 0,0,
							0,0,-1,0,
							0,1, 0,0,
							0,0, 0,1 };
			node.mesh = gltfModel.meshes.size() - 1;
			gltfModel.nodes.push_back(node);

			tinygltf::Scene scene;
			for (int i = 0; i < gltfModel.nodes.size(); i++) {
				scene.nodes.push_back(i);
			}
			gltfModel.scenes.push_back(scene);
			gltfModel.defaultScene = 0;

			tinygltf::TinyGLTF tf;
			std::ostringstream oss;

			if (tf.WriteGltfSceneToStream(&gltfModel, oss, false, true)) {
				const std::string content = oss.str();
				buffer.append(content.data(), content.size());
				return true;
			}
			
			return false;
		}
	private:
		void addGltfMesh(const CesiumMesh& m) {
			if (m.vertex.isEmpty() || m.indices.isEmpty())
			{
				qWarning() << "empty packed mesh";
				return;
			}

			tinygltf::Primitive primit;

			// vertex
			{
				tinygltf::Accessor accessor;
				writeBuffer(&m.vertex[0], m.vertex.size() * sizeof(osg::Vec3f), TINYGLTF_TARGET_ARRAY_BUFFER);
				accessor.bufferView = gltfModel.bufferViews.size() - 1;
				accessor.byteOffset = 0;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				accessor.count = m.vertex.size();
				accessor.type = TINYGLTF_TYPE_VEC3;
				
				osg::BoundingBoxd box = m.boundingBox();
				accessor.minValues.push_back(box.xMin());
				accessor.minValues.push_back(box.yMin());
				accessor.minValues.push_back(box.zMin());
				accessor.maxValues.push_back(box.xMax());
				accessor.maxValues.push_back(box.yMax());
				accessor.maxValues.push_back(box.zMax());

				gltfModel.accessors.push_back(accessor);
				primit.attributes["POSITION"] = gltfModel.accessors.size() - 1;
			}

			// normal
			if (!m.normals.isEmpty())
			{
				tinygltf::Accessor accessor;
				writeBuffer(&m.normals[0], m.normals.size() * sizeof(osg::Vec3f), TINYGLTF_TARGET_ARRAY_BUFFER);
				accessor.bufferView = gltfModel.bufferViews.size() - 1;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				accessor.count = m.normals.size();
				accessor.type = TINYGLTF_TYPE_VEC3;
				accessor.byteOffset = 0;

				gltfModel.accessors.push_back(accessor);
				primit.attributes["NORMAL"] = gltfModel.accessors.size() - 1;
			}

			// texcoord
			if (m.texcoords.size() > 0) {
				writeBuffer(&m.texcoords[0], m.texcoords.size() * sizeof(osg::Vec2f), TINYGLTF_TARGET_ARRAY_BUFFER);
				tinygltf::Accessor accessor;
				accessor.bufferView = gltfModel.bufferViews.size() - 1;
				accessor.byteOffset = 0;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				accessor.count = m.texcoords.size();
				accessor.type = TINYGLTF_TYPE_VEC2;

				gltfModel.accessors.push_back(accessor);
				primit.attributes["TEXCOORD_0"] = gltfModel.accessors.size() - 1;
			}

			// batch id
			if (m.batchIDs.size() > 0) {
				writeBuffer(&m.batchIDs[0], m.batchIDs.size() * sizeof(float), TINYGLTF_TARGET_ARRAY_BUFFER);

				tinygltf::Accessor accessor;
				accessor.bufferView = gltfModel.bufferViews.size() - 1;
				accessor.byteOffset = 0;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				accessor.count = m.batchIDs.size();
				accessor.type = TINYGLTF_TYPE_SCALAR;

				gltfModel.accessors.push_back(accessor);
				primit.attributes["_BATCHID"] = gltfModel.accessors.size() - 1;
			}

			// TODO: vertex properties
			{

			}

			// index
			{
				size_t icount = m.indices.size();
                writeBuffer(m.indices.data(), icount * sizeof(uint32_t), TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

                tinygltf::Accessor accessor;
                accessor.bufferView = gltfModel.bufferViews.size() - 1;
                accessor.byteOffset = 0;
                accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                accessor.count = icount;
                accessor.type = TINYGLTF_TYPE_SCALAR;
                gltfModel.accessors.push_back(accessor);

				primit.indices = gltfModel.accessors.size() - 1;
			}

			// material
			{
				primit.material = gltfModel.materials.size() - 1;
				primit.mode = TINYGLTF_MODE_TRIANGLES;  //TINYGLTF_MODE_TRIANGLES;
			}

			gltfMesh.primitives.push_back(primit);
		}

		void writeBuffer(const void* data, size_t writeSize, int target) {
			tinygltf::BufferView view;
			view.buffer = 0;
			view.byteOffset = gltfBuffer.data.size();
			view.byteLength = writeSize;
			view.target = target;
			gltfBuffer.data.resize(gltfBuffer.data.size() + writeSize);
			std::memcpy(&gltfBuffer.data[view.byteOffset], data, writeSize);

			while (gltfBuffer.data.size() % 4 != 0) {
                gltfBuffer.data.push_back(' ');
			}

			gltfModel.bufferViews.push_back(view);
		}

		void addGltfMaterial(const CesiumMaterial& m) {
			tinygltf::Material material;

			// base texture
			auto& baseColor = material.pbrMetallicRoughness;
			baseColor.baseColorFactor = {
				m.baseColorFactor.r(),
				m.baseColorFactor.g(),
				m.baseColorFactor.b(),
				m.baseColorFactor.a()
			};

			if (mBaseTextureIndex.index >= 0) {
				baseColor.baseColorTexture.index = mBaseTextureIndex.index;
				baseColor.baseColorTexture.texCoord = 0;
			}
			
			// texture properties
			{
				baseColor.metallicFactor = m.metallicFactor;
				baseColor.roughnessFactor = m.roughnessFactor;
			}

			// normal texture
			if (mNormalTextureIndex.index >= 0) {
				auto& normalTexture = material.normalTexture;
				
				normalTexture.index = mNormalTextureIndex.index;
				normalTexture.texCoord = 0;
			}

			// is double side?
			material.doubleSided = m.doubleSided;
			
			// no light
			{
				material.extensions["KHR_materials_unlit"] = {};
				extensionsRequired.insert("KHR_materials_unlit");
				extensionsUsed.insert("KHR_materials_unlit");
			}

			gltfModel.materials.push_back(material);
		}

		void addImage(tinygltf::Image& image)
		{
            writeBuffer(image.image.data(), image.image.size(), 0);
            image.image.clear();
            image.bufferView = gltfModel.bufferViews.size() - 1;
            image.mimeType = image.mimeType;
			gltfModel.images.push_back(image);

			tinygltf::Sampler sampler;
			sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
			sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
			sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
			sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
			gltfModel.samplers.push_back(sampler);

			// add texture
			tinygltf::Texture tinyTexture;
			tinyTexture.sampler = gltfModel.samplers.size() - 1;
			tinyTexture.source = gltfModel.images.size() - 1;
			gltfModel.textures.push_back(tinyTexture);
		}

		TextureResult saveGltfTexture(const CesiumTexture& texture) {
			TextureResult tr;
			tinygltf::Image image;
			
			if (!texture.toGltfImage(image)) {
				return tr;
			}

			addImage(image);
			tr.index = gltfModel.textures.size() - 1;
			return tr;
		}

		TextureResult mBaseTextureIndex;
		TextureResult mNormalTextureIndex;
		QSet<QString> extensionsRequired;
		QSet<QString> extensionsUsed;
	};

	bool CesiumB3DM::toGltfBinaryWithNoPack(QByteArray& buffer) const {
		if (meshes.isEmpty())
			return false;

		TinyGLTFConvert gltfAdder;

		foreach (const auto mesh, meshes) {
			//gltfAdder.add(mesh->yUp());
            gltfAdder.add(mesh);
		}

		QByteArray gltfBuffer;
		if (!gltfAdder.save(gltfBuffer)) {
			return false;
		}
		
		toB3DM(gltfBuffer, buffer);
		return true;
	}

	void CesiumB3DM::toB3DM(const QByteArray& glb, QByteArray& buffer) const{
		QDataStream dataStream(&buffer, QIODevice::WriteOnly);
		dataStream.setByteOrder(QDataStream::LittleEndian);

		// TODO: batch table

		// feature table
		QByteArray featureTableJsonByte;
		{
			QJsonObject featureTableJson;
			featureTableJson["BATCH_LENGTH"] = 0;
			if (center != osg::Vec3d(0, 0, 0)) {
				featureTableJson["RTC_CENTER"] = QJsonArray({
					center.x(),
					center.y(),
					center.z()
					});
			}

			featureTableJsonByte = QJsonDocument(featureTableJson)
				.toJson(QJsonDocument::Compact);
			while (featureTableJsonByte.size() % 8 != 0) {
				featureTableJsonByte.append(' ');
			}
		}
		
		uint32_t version = 1;
		uint32_t featureTableJSONByteLength = featureTableJsonByte.size();
		uint32_t featureTableBinaryByteLength = 0;
		uint32_t batchTableJSONByteLength = 0;
		uint32_t batchTableBinaryByteLength = 0;

		uint32_t headerLength = 28
			+ featureTableJSONByteLength
			+ featureTableBinaryByteLength
			+ batchTableJSONByteLength
			+ batchTableBinaryByteLength
			+ glb.size();

		
		// b3dm header
		{
			dataStream.writeRawData("b3dm", 4);
			dataStream << version;
			dataStream << headerLength;
			dataStream << featureTableJSONByteLength;
			dataStream << featureTableBinaryByteLength;
			dataStream << batchTableJSONByteLength;
			dataStream << batchTableBinaryByteLength;
		}
		
		dataStream.writeRawData(featureTableJsonByte.data(), featureTableJsonByte.size());
		dataStream.writeRawData(glb.data(), glb.size());
	}
}
