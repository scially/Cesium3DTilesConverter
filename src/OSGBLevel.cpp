#include <OSGBLevel.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDataStream>
#include <DxtImage.h>

namespace gzpi {

    QString OSGBLevel::absoluteLocation() const {
        return QDir(nodePath).filePath(nodeName);
    }

    int OSGBLevel::getLevelNumber() {
        int p0 = nodeName.indexOf("_L");
        if (p0 < 0)
            return 0;
        int p1 = nodeName.indexOf("_", p0 + 2);
        if (p1 < 0)
            return 0;
        return nodeName.mid(p0 + 2, p1 - p0 - 2).toInt();
    }

    bool OSGBLevel::getAllOSGBLevels(int maxLevel) {
        if (getLevelNumber() < 0 || getLevelNumber() >= maxLevel)
            return false;

        OSGBPageLodVisitor lodVisitor(nodePath);

        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString() };
        osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(rootOSGBLocation);

        if (root == nullptr)
            return false;

        root->accept(lodVisitor);

        for (unsigned int i = 0; i < lodVisitor.subNodeNames.size(); i++) {
            OSGBLevel subLevel(lodVisitor.subNodeNames[i]);
            if (subLevel.getAllOSGBLevels(maxLevel)) {
                subNodes.append(subLevel);
            }
        }
        return true;
    }


    bool OSGBLevel::writeB3DM(const QString& outLocation) {
        QByteArray b3dmBuffer;

        if (!convertB3DM(b3dmBuffer))
            return false;

        // 根据osgb文件名获取对应的b3dm输出路径    
        QFile b3dmFile(outLocation + "/" + nodeName.replace(".osgb", ".b3dm"));
        if (!b3dmFile.open(QIODevice::ReadWrite)) {
            qFatal("Can't open file [%1]", b3dmFile.fileName());
            return false;
        }
        b3dmFile.write(b3dmBuffer);

        for (auto& node : subNodes) {
            node.writeB3DM(outLocation);
        }
        return true;
    }

    bool OSGBLevel::convertB3DM(QByteArray& b3dmBuffer) {
        QByteArray glbBuffer;
        QDataStream b3dmStream(&b3dmBuffer, QIODevice::WriteOnly);
        
        OSGBMesh mesh;
        convertGLB(glbBuffer, mesh);
        if (glbBuffer.isEmpty())
            return false;

        bBoxMin = mesh.min;
        bBoxMax = mesh.max;

        QString feature = QString(R"({"BATCH_LENGTH":%1})").arg(1);
        while (feature.size() % 4 != 0) {
            feature.append(' ');
        }

        QJsonObject batch;
        QJsonArray ids = { 1 };
        QJsonArray names = { QString("mesh_%1").arg(1) };

        batch["batchId"] = ids;
        batch["name"] = names;

        QString batchMeta = QJsonDocument(batch).toJson(QJsonDocument::Compact);
        while (batchMeta.size() % 4 != 0) {
            batchMeta.append(' ');
        }


        int totalSize = 28 /*header size*/ + feature.size() + batchMeta.size() + glbBuffer.size();
            
        b3dmStream.writeRawData("b3dm", 4);
        b3dmStream << 1;          // version
        b3dmStream << totalSize;
        b3dmStream << feature.size();
        b3dmStream << 0;
        b3dmStream << batchMeta.size();
        b3dmStream << 0;
        b3dmStream.writeRawData(feature.toStdString().data(), feature.size());
        b3dmStream.writeRawData(batchMeta.toStdString().data(), batchMeta.size());
        b3dmStream.writeRawData(glbBuffer.data(), glbBuffer.size());
        return true;
    }

    bool OSGBLevel::convertGLB(QByteArray & glbBuffer, OSGBMesh & mesh) {

        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString() };

        osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(rootOSGBLocation);
        if (!root.valid()) {
            return false;
        }

        OSGBPageLodVisitor lodVisitor(nodePath);
        root->accept(lodVisitor);
        if (lodVisitor.geometryArray.empty())
            return false;

        osgUtil::SmoothingVisitor sv;
        root->accept(sv);

        tinygltf::TinyGLTF gltf;
        tinygltf::Model model;
        tinygltf::Buffer buffer;

        osg::Vec3f point_max, point_min;
        OSGBuildState osgState = {
            &buffer,
            &model,
            osg::Vec3f(-1e38,-1e38,-1e38),
            osg::Vec3f(1e38,1e38,1e38),
            -1,
            -1
        };

        // mesh
        model.meshes.resize(1);
        int primitiveIdx = 0;
        for (auto g : lodVisitor.geometryArray)
        {
            if (!g->getVertexArray() || g->getVertexArray()->getDataSize() == 0)
                continue;

            osgState.appendOSGGeometry(g);
            // update primitive material index
            if (lodVisitor.textureArray.size())
            {
                for (unsigned int k = 0; k < g->getNumPrimitiveSets(); k++)
                {
                    auto tex = lodVisitor.textureMap[g];
                    // if hava texture
                    if (tex)
                    {
                        for (auto texture : lodVisitor.textureArray)
                        {
                            model.meshes[0].primitives[primitiveIdx].material++;
                            if (tex == texture)
                                break;
                        }
                    }
                    primitiveIdx++;
                }
            }
        }
        // empty geometry or empty vertex-array
        if (model.meshes[0].primitives.empty())
            return false;

        mesh.min = osg::Vec3d(osgState.pointMin.x(), osgState.pointMin.y(), osgState.pointMin.z());
        mesh.max = osg::Vec3d(osgState.pointMax.x(), osgState.pointMax.y(), osgState.pointMax.z());
        // image
        {
            for (auto tex : lodVisitor.textureArray)
            {
                unsigned bufferStart = buffer.size();
                std::vector<unsigned char> jpegBuffer(512 * 512 * 3);
                int width, height, comp;
                if (tex) {
                    if (tex->getNumImages() > 0) {
                        osg::Image* img = tex->getImage(0);
                        if (img) {
                            width = img->s();
                            height = img->t();
                            comp = img->getPixelSizeInBits();
                            if (comp == 8) comp = 1;
                            if (comp == 24) comp = 3;
                            if (comp == 4) {
                                comp = 3;
                                fill4BitImage(jpegBuffer, img, width, height);
                            }
                            else
                            {
                                unsigned row_step = img->getRowStepInBytes();
                                unsigned row_size = img->getRowSizeInBytes();
                                for (size_t i = 0; i < height; i++)
                                {
                                    jpegBuffer.insert(jpegBuffer.end(),
                                        img->data() + row_step * i,
                                        img->data() + row_step * i + row_size);
                                }
                            }
                        }
                    }
                }

                auto stbImgWriteBuffer = [](void* context, void* data, int len) {
                    std::vector<char>* buf = (std::vector<char>*)context;
                    buf->insert(buf->end(), (char*)data, (char*)data + len);
                };
                if (!jpegBuffer.empty()) {
                    buffer.data.reserve(buffer.size() + width * height * comp);
                    stbi_write_jpg_to_func(stbImgWriteBuffer, &buffer.data, width, height, comp, jpegBuffer.data(), 80);
                }
                else {
                    std::vector<unsigned char> vData(256 * 256 * 3);
                    stbi_write_jpg_to_func(stbImgWriteBuffer, &buffer.data, 256, 256, 3, vData.data(), 80);

                    tinygltf::Image image;
                    image.mimeType = "image/jpeg";
                    image.bufferView = model.bufferViews.size();
                    model.images.push_back(image);
                    tinygltf::BufferView bfv;
                    bfv.buffer = 0;
                    bfv.byteOffset = bufferStart;
                    buffer.alignment();
                    bfv.byteLength = buffer.size() - bufferStart;
                    model.bufferViews.push_back(bfv);
                }
            }
            // node
            {
                tinygltf::Node node;
                node.mesh = 0;
                model.nodes.push_back(node);
            }
            // scene
            {
                tinygltf::Scene sence;
                sence.nodes.push_back(0);
                model.scenes = { sence };
                model.defaultScene = 0;
            }
            // sample
            {
                tinygltf::Sampler sample;
                sample.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
                sample.minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
                sample.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
                sample.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
                model.samplers = { sample };
            }
            // use KHR_materials_unlit
            model.extensionsRequired = { "KHR_materials_unlit" };
            model.extensionsUsed = { "KHR_materials_unlit" };
            for (int i = 0; i < lodVisitor.textureArray.size(); i++)
            {
                tinygltf::Material mat = makeColorMaterialFromRGB(1.0, 1.0, 1.0);
                mat.b_unlit = true; // use KHR_materials_unlit
                tinygltf::Parameter baseColorTexture;
                baseColorTexture.json_int_value = { std::pair<std::string, int>("index",i) };
                mat.values["baseColorTexture"] = baseColorTexture;
                model.materials.push_back(mat);
            }

            // finish buffer
            model.buffers.push_back(std::move(buffer));
            // texture
            {
                int texture_index = 0;
                for (auto tex : lodVisitor.textureArray)
                {
                    tinygltf::Texture texture;
                    texture.source = texture_index++;
                    texture.sampler = 0;
                    model.textures.push_back(texture);
                }
            }
            model.asset.version = "2.0";
            model.asset.generator = "gzpi";

            glbBuffer = QByteArray::fromStdString(gltf.Serialize(&model));
            return true;
        }

       
    }

    tinygltf::Material OSGBLevel::makeColorMaterialFromRGB(double r, double g, double b) {
        tinygltf::Material material;
        material.name = "default";
        tinygltf::Parameter baseColorFactor;
        baseColorFactor.number_array = { r, g, b, 1.0 };
        material.values["baseColorFactor"] = baseColorFactor;

        tinygltf::Parameter metallicFactor;
        metallicFactor.number_value = new double(0);
        material.values["metallicFactor"] = metallicFactor;
        tinygltf::Parameter roughnessFactor;
        roughnessFactor.number_value = new double(1);
        material.values["roughnessFactor"] = roughnessFactor;
        //
        return material;
    }
}