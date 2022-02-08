#pragma once

#include <OSGBConvert.h>
#include <OSGBPageLodVisitor.h>
#include <osgDB/ReadFile>
#include <osg/Image>
#include <vector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDataStream>
#include <QDebug>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <stb_image_write.h>
#include <tiny_gltf.h>


namespace scially {

    QString OSGBConvert::absoluteLocation() const {
        return QDir(nodePath).filePath(nodeName);
    }

    bool OSGBConvert::writeB3DM(const QByteArray &buffer, const QString& outLocation) {

        if (buffer.isEmpty()) {
            qCritical() << "B3DM buffer is empty...\n";
            return false;
        }

        //
        QFile b3dmFile(outLocation + "/" + nodeName.replace(".osgb", ".b3dm"));
        if (!b3dmFile.open(QIODevice::ReadWrite)) {
            qCritical() << "Can't open file [" << b3dmFile.fileName() << "]\n";
            return false;
        }
        int writeBytes = b3dmFile.write(buffer);

        if (writeBytes <= 0) {
            qCritical() << "Can't write file [" << b3dmFile.fileName() << "]\n";
            return false;
        }
        return true;
    }

    QByteArray OSGBConvert::toB3DM() {
        QByteArray b3dmBuffer;
        QDataStream b3dmStream(&b3dmBuffer, QIODevice::WriteOnly);
        b3dmStream.setByteOrder(QDataStream::LittleEndian);

        QByteArray glbBuffer = convertGLB();

        if (glbBuffer.isEmpty())
            return QByteArray();

        QString featureTable = R"({"BATCH_LENGTH":1})";
        while (featureTable.size() % 4 != 0) {
            featureTable.append(' ');
        }

        QString batchTable = R"({"batchId":[0],"name":["mesh_0"]})";
        while (batchTable.size() % 4 != 0) {
            batchTable.append(' ');
        }

        int totalSize = 28 /*header size*/ + featureTable.size() + batchTable.size() + glbBuffer.size();

        b3dmStream.writeRawData("b3dm", 4);
        b3dmStream << 1;          // version
        b3dmStream << totalSize;
        b3dmStream << featureTable.size();
        b3dmStream << 0;
        b3dmStream << batchTable.size();
        b3dmStream << 0;

        // DataStream << will write byte length first, so invoke wrteRawData
        b3dmStream.writeRawData(featureTable.toStdString().data(), featureTable.size());
        b3dmStream.writeRawData(batchTable.toStdString().data(), batchTable.size());
        b3dmStream.writeRawData(glbBuffer.data(), glbBuffer.size());

        return b3dmBuffer;
    }

    QByteArray OSGBConvert::convertGLB() {
        QByteArray glbBuffer;

        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString() };
        osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(rootOSGBLocation);
        if (!root.valid()) {
            qCritical() << "Read OSGB File [" << absoluteLocation() << "] Fail...\n";
            return QByteArray();
        }

        OSGBPageLodVisitor lodVisitor(nodePath);
        root->accept(lodVisitor);
        if (lodVisitor.geometryArray.empty()) {
            qCritical() << "Read OSGB File [" << absoluteLocation() << "] geometries is Empty...\n";
            return QByteArray();
        }

        osgUtil::SmoothingVisitor sv;
        root->accept(sv);

        tinygltf::TinyGLTF gltf;
        tinygltf::Model model;
        tinygltf::Buffer buffer;


        OSGBuildState osgState = {
            &buffer,
            &model,
            osg::Vec3f(-1e38, -1e38, -1e38),
            osg::Vec3f(1e38, 1e38, 1e38),
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
                    if (tex != nullptr)
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
            return QByteArray();

        region.setMax(osgState.pointMax);
        region.setMin(osgState.pointMin);

        // image
        {
            for (auto tex : lodVisitor.textureArray)
            {
                unsigned bufferStart = buffer.size();
                std::vector<unsigned char> jpegBuffer;
                int width, height, comp;
                if (tex != nullptr) {
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
                                internal::fill4BitImage(jpegBuffer, img, width, height);
                            }
                            else
                            {
                                unsigned rowStep = img->getRowStepInBytes();
                                unsigned rowSize = img->getRowSizeInBytes();
                                for (int i = 0; i < height; i++)
                                {
                                    jpegBuffer.insert(jpegBuffer.end(),
                                        img->data() + rowStep * i,
                                        img->data() + rowSize * i + rowSize);
                                }
                            }
                        }
                    }
                }

                const auto stbImgWriteBuffer = [](void* context, void* data, int len) {
                    auto buf = (std::vector<char>*)context;
                    buf->insert(buf->end(), (char*)data, (char*)data + len);
                };

                if (!jpegBuffer.empty()) {
                    buffer.data.reserve(buffer.size() + width * height * comp);
                    stbi_write_jpg_to_func(stbImgWriteBuffer, &buffer.data, width, height, comp, jpegBuffer.data(), 80);
                }
                else {
                    std::vector<unsigned char> vData(256 * 256 * 3);
                    stbi_write_jpg_to_func(stbImgWriteBuffer, &buffer.data, 256, 256, 3, vData.data(), 80);
                }

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

            // use pbr material
            {
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
            }

            // finish buffer
            model.buffers.push_back(buffer);
            // texture
            {
                int textureIndex = 0;
                for (auto tex : lodVisitor.textureArray)
                {
                    tinygltf::Texture texture;
                    texture.source = textureIndex++;
                    texture.sampler = 0;
                    model.textures.push_back(texture);
                }
            }
            model.asset.version = "2.0";
            model.asset.generator = "hwang";

            glbBuffer = QByteArray::fromStdString(gltf.Serialize(&model));
            return glbBuffer;
        }


    }

    tinygltf::Material OSGBConvert::makeColorMaterialFromRGB(double r, double g, double b) {
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

namespace internal {

    Color RGB565_RGB(unsigned short color0) {
        unsigned char r0 = ((color0 >> 11) & 0x1F) << 3;
        unsigned char g0 = ((color0 >> 5) & 0x3F) << 2;
        unsigned char b0 = (color0 & 0x1F) << 3;
        return Color{ r0, g0, b0 };
    }

    Color Mix_Color(
            unsigned short color0, unsigned short color1,
            Color c0, Color c1, int idx) {
        Color finalColor;
        if (color0 > color1)
        {
            switch (idx)
            {
                case 0:
                    finalColor = Color{ c0.r, c0.g, c0.b };
                    break;
                case 1:
                    finalColor = Color{ c1.r, c1.g, c1.b };
                    break;
                case 2:
                    finalColor = Color{
                            (2 * c0.r + c1.r) / 3,
                            (2 * c0.g + c1.g) / 3,
                            (2 * c0.b + c1.b) / 3 };
                    break;
                case 3:
                    finalColor = Color{
                            (c0.r + 2 * c1.r) / 3,
                            (c0.g + 2 * c1.g) / 3,
                            (c0.b + 2 * c1.b) / 3 };
                    break;
            }
        }
        else
        {
            switch (idx)
            {
                case 0:
                    finalColor = Color{ c0.r, c0.g, c0.b };
                    break;
                case 1:
                    finalColor = Color{ c1.r, c1.g, c1.b };
                    break;
                case 2:
                    finalColor = Color{ (c0.r + c1.r) / 2, (c0.g + c1.g) / 2, (c0.b + c1.b) / 2 };
                    break;
                case 3:
                    finalColor = Color{ 0, 0, 0 };
                    break;
            }
        }
        return finalColor;
    }
    void resizeImage(std::vector<unsigned char>& jpeg_buf, int width, int height, int new_w, int new_h) {
        std::vector<unsigned char> new_buf(new_w * new_h * 3);
        int scale = width / new_w;
        for (int row = 0; row < new_h; row++)
        {
            for (int col = 0; col < new_w; col++) {
                int pos = row * new_w + col;
                int old_pos = (row * width + col) * scale;
                for (int i = 0; i < 3; i++)
                {
                    new_buf[3 * pos + i] = jpeg_buf[3 * old_pos + i];
                }
            }
        }
        jpeg_buf = new_buf;
    }

    void fill4BitImage(std::vector<unsigned char>& jpeg_buf, osg::Image* img, int& width, int& height) {
        jpeg_buf.resize(width * height * 3);
        unsigned char* pData = img->data();
        int imgSize = img->getImageSizeInBytes();
        int x_pos = 0;
        int y_pos = 0;
        for (int i = 0; i < imgSize; i += 8)
        {
            // 64 bit matrix
            unsigned short color0, color1;
            std::memcpy(&color0, pData, 2);
            pData += 2;
            memcpy(&color1, pData, 2);
            pData += 2;
            Color c0 = RGB565_RGB(color0);
            Color c1 = RGB565_RGB(color1);
            for (size_t i = 0; i < 4; i++)
            {
                unsigned char idx[4];
                idx[3] = (*pData >> 6) & 0x03;
                idx[2] = (*pData >> 4) & 0x03;
                idx[1] = (*pData >> 2) & 0x03;
                idx[0] = (*pData) & 0x03;
                // 4 pixel color
                for (size_t pixel_idx = 0; pixel_idx < 4; pixel_idx++)
                {
                    Color cf = Mix_Color(color0, color1, c0, c1, idx[pixel_idx]);
                    int cell_x_pos = x_pos + pixel_idx;
                    int cell_y_pos = y_pos + i;
                    int byte_pos = (cell_x_pos + cell_y_pos * width) * 3;
                    jpeg_buf[byte_pos] = cf.r;
                    jpeg_buf[byte_pos + 1] = cf.g;
                    jpeg_buf[byte_pos + 2] = cf.b;
                }
                pData++;
            }
            x_pos += 4;
            if (x_pos >= width) {
                x_pos = 0;
                y_pos += 4;
            }
        }
        int max_size = 2048;
        if (width > max_size || height > max_size) {
            int new_w = width, new_h = height;
            while (new_w > max_size || new_h > max_size)
            {
                new_w /= 2;
                new_h /= 2;
            }
            resizeImage(jpeg_buf, width, height, new_w, new_h);
            width = new_w;
            height = new_h;
        }
    }
}
