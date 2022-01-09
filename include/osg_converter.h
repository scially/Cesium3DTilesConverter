#pragma once

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <string>
#include <vector>
#include <cmath>

#include <QVariant>
#include <QMap>
#include <QVector>
#include <QString>
#include <QDir>
#include <QByteArray>
#include <QDebug>
#include <QRunnable>
#include <QThreadPool>
#include <QSharedPointer>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include <tiny_gltf_tools.hpp>

struct MeshInfo
{
    QString name;
    std::array<double, 3> min;
    std::array<double, 3> max;
};


struct OsgbInfo {
    QString output;
    QJsonObject json;
};


class OSGBPageLodVisitor : public osg::NodeVisitor
{
public:
    OSGBPageLodVisitor(const QString& path)
        :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), path(path)
    {}

    virtual ~OSGBPageLodVisitor() {}

    void apply(osg::Geometry& geometry) {
        geometryArray.append(&geometry);
        if (auto ss = geometry.getStateSet()) {
            osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
            if (tex) {
                textureArray.insert(tex);
                textureMap[&geometry] = tex;
            }
        }
    }

    void apply(osg::PagedLOD& node) {
        int n = node.getNumFileNames();
        for (unsigned int i = 1; i < n; i++)
        {
            QString fileName = path + "/" + QString::fromStdString(node.getFileName(i));
            subNodeNames.append(fileName);
        }
        traverse(node);
    }

public:
    QString                             path;
    QVector<osg::Geometry*>             geometryArray;
    QSet<osg::Texture*>                 textureArray;
    QMap<osg::Geometry*, osg::Texture*> textureMap;  // 记录 mesh 和 texture 的关系，暂时认为一个模型最多只有一个texture
    QVector<QString>                    subNodeNames;
};


class OSGBLevel {
public:
    using OSGBLevelPtr = QSharedPointer<OSGBLevel>;

    OSGBLevel() {}
    OSGBLevel(const QString& name, const QString& path) :
        nodeName(name), nodePath(path) {}
    OSGBLevel(const QString& absoluteLocation) {

    }

    double              geometricError;
    QString             nodeName;
    QString             nodePath;
    QVector<OSGBLevel>  subNodes;


    QString absoluteLocation() const {
       return QDir(nodePath).filePath(nodeName);
    }

    /// <summary>
    /// 提取 Tile_+154_+018_L22_0000320.osgb _L后面的数字
    /// 提取失败，返回 0
    /// </summary>
    int getLevelNumber() {
        int p0 = nodeName.indexOf("_L");
        if (p0 < 0) 
            return 0;
        int p1 = nodeName.indexOf("_", p0 + 2);
        if (p1 < 0) 
            return 0;
        return nodeName.mid(p0 + 2, p1 - p0 - 2).toInt();
    }

    bool getAllOSGBLevels(int maxLevel) {
        if (getLevelNumber() <= 0 || getLevelNumber() > maxLevel)
            return false;

        OSGBPageLodVisitor lodVisitor(nodePath);

        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString()};
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

    /// <summary>
    /// 瓦片及PageLOD下的所有子节点转B3DM
    /// </summary>
    /// <param name="outLocation"></param>
    /// <returns></returns>
    bool convertB3DM(const QString& out_path) {
        
        //TODO: 可以把BOX放到OSGBLevel对象下
        QByteArray b3dmBuffer = osgb2b3dm_buf(/*, tree.bbox */);
        // 根据osgb文件名获取对应的b3dm输出路径
        QString out_file = out_path + "/" + get_file_name(tree.file_name).replace(".osgb", ".b3dm");

        if (b3dm_buf.isEmpty())
            return false;
        write_file(out_file, b3dm_buf);
        for (auto& i : tree.sub_nodes) {
            do_tile_job(i, out_path);
        }
        return true;
    }

    /// <summary>
    /// OSGB文件转B3DM
    /// </summary>
    /// <param name="path"></param>
    /// <param name="tile_box"></param>
    /// <returns></returns>
    QByteArray convertB3DM() {
        QByteArray b3dmBuffer;
        
        using nlohmann::json;
        QVector<MeshInfo> v_info;
        QByteArray glbBuffer = osgb2glb_buf(path, v_info);
        if (glbBuffer.isEmpty())
            return QByteArray();

        tile_box.max = {
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::lowest() };

        tile_box.min = {
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max() ,
            std::numeric_limits<double>::max() };

        for (auto& mesh : v_info) {
            for (int i = 0; i < 3; i++) {
                if (mesh.min[i] < tile_box.min[i])
                    tile_box.min[i] = mesh.min[i];
                if (mesh.max[i] > tile_box.max[i])
                    tile_box.max[i] = mesh.max[i];
            }
        }

        int mesh_count = v_info.size();
        std::string feature_json_string;
        feature_json_string += "{\"BATCH_LENGTH\":";
        feature_json_string += std::to_string(mesh_count);
        feature_json_string += "}";
        while (feature_json_string.size() % 4 != 0) {
            feature_json_string.push_back(' ');
        }

        json batch_json;
        std::vector<int> ids;
        for (int i = 0; i < mesh_count; ++i) {
            ids.push_back(i);
        }
        std::vector<std::string> names;
        for (int i = 0; i < mesh_count; ++i) {
            std::string mesh_name = "mesh_";
            mesh_name += std::to_string(i);
            names.push_back(mesh_name);
        }
        batch_json["batchId"] = ids;
        batch_json["name"] = names;
        std::string batch_json_string = batch_json.dump();
        while (batch_json_string.size() % 4 != 0) {
            batch_json_string.push_back(' ');
        }


        int feature_json_len = feature_json_string.size();
        int feature_bin_len = 0;
        int batch_json_len = batch_json_string.size();
        int batch_bin_len = 0;
        int total_len = 28 /*header size*/ + feature_json_len + batch_json_len + glb_buf.size();

        b3dm_buf.append("b3dm");
        int version = 1;
        b3dm_buf.append(reinterpret_cast<const char*>(&version), sizeof(int));  // version
        b3dm_buf.append(reinterpret_cast<const char*>(&total_len), sizeof(int));
        b3dm_buf.append(reinterpret_cast<const char*>(&feature_json_len), sizeof(int));
        b3dm_buf.append(reinterpret_cast<const char*>(&feature_bin_len), sizeof(int));
        b3dm_buf.append(reinterpret_cast<const char*>(&batch_json_len), sizeof(int));
        b3dm_buf.append(reinterpret_cast<const char*>(&batch_bin_len), sizeof(int));

        b3dm_buf.append(QString::fromStdString(feature_json_string));
        b3dm_buf.append(QString::fromStdString(batch_json_string));
        b3dm_buf.append(glb_buf);

        return b3dm_buf;
    }


    QByteArray convertGLB(QVector<MeshInfo>& v_info) {
        std::vector<std::string> rootOSGBLocation = { absoluteLocation().toStdString() };
        osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(rootOSGBLocation);

        if (root == nullptr)
            return QByteArray();

        OSGBPageLodVisitor lodVisitor(nodePath);
        root->accept(lodVisitor);
        
        if (lodVisitor.geometryArray.empty())
            return QByteArray();

        osgUtil::SmoothingVisitor sv;
        root->accept(sv);

        {
            tinygltf::TinyGLTF gltf;
            tinygltf::Model model;
            tinygltf::Buffer buffer;
  
            uint32_t buf_offset = 0;
            uint32_t acc_offset[4] = { 0,0,0,0 };
            for (int j = 0; j < 4; j++)
            {
                for (const auto geometry : lodVisitor.geometryArray) {
                    if (geometry->getNumPrimitiveSets() == 0) {
                        continue;
                    }
                    osg::Array* va = geometry->getVertexArray();

                    if (j == 0) { // indc
                        {
                            unsigned int maxIndex = 0, minIndex = 1 << 30;
                            int idxSize = 0;
                            osg::PrimitiveSet::Type t = geometry->getPrimitiveSet(0)->getType();
                            for (int k = 0; k < geometry->getNumPrimitiveSets(); k++)
                            {
                                osg::PrimitiveSet* ps = geometry->getPrimitiveSet(k);
                                if (t != ps->getType())
                                {
                                    qFatal("PrimitiveSets type are NOT same in osgb");
                                }
                                idxSize += ps->getNumIndices();
                            }
                            for (int k = 0; k < geometry->getNumPrimitiveSets(); k++)
                            {
                                osg::PrimitiveSet* ps = geometry->getPrimitiveSet(k);
                                switch (t)
                                {
                                case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
                                {
                                    const osg::DrawElementsUByte* drawElements = static_cast<const osg::DrawElementsUByte*>(ps);
                                    int IndNum = drawElements->getNumIndices();
                                    for (unsigned int m = 0; m < IndNum; m++)
                                    {
                                        if (idxSize <= 256)
                                            putBufferValue(buffer.data, drawElements->at(m));
                                        else if (idxSize <= 65536)
                                            putBufferValue(buffer.data, (unsigned short)drawElements->at(m));
                                        else
                                            putBufferValue(buffer.data, (unsigned int)drawElements->at(m));
                                        if (drawElements->at(m) > maxIndex) maxIndex = drawElements->at(m);
                                        if (drawElements->at(m) < minIndex) minIndex = drawElements->at(m);
                                    }
                                    break;
                                }
                                case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
                                {
                                    const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(ps);
                                    int IndNum = drawElements->getNumIndices();
                                    for (size_t m = 0; m < IndNum; m++)
                                    {
                                        if (idx_size <= 65536)
                                            put_val(buffer.data, drawElements->at(m));
                                        else
                                            put_val(buffer.data, (unsigned int)drawElements->at(m));
                                        if (drawElements->at(m) > max_index) max_index = drawElements->at(m);
                                        if (drawElements->at(m) < min_index) min_index = drawElements->at(m);
                                    }
                                    break;
                                }
                                case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
                                {
                                    const osg::DrawElementsUInt* drawElements = static_cast<const osg::DrawElementsUInt*>(ps);
                                    unsigned int IndNum = drawElements->getNumIndices();
                                    for (unsigned int m = 0; m < IndNum; m++)
                                    {
                                        put_val(buffer.data, drawElements->at(m));
                                        if (drawElements->at(m) > maxIndex) maxIndex = drawElements->at(m);
                                        if (drawElements->at(m) < minIndex) minIndex = drawElements->at(m);
                                    }
                                    break;
                                }
                                case osg::PrimitiveSet::DrawArraysPrimitiveType: {
                                    osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(ps);
                                    auto mode = da->getMode();
                                    if (mode != GL_TRIANGLES) {
                                        qFatal("GLenum is not GL_TRIANGLES in osgb");
                                    }
                                    if (k == 0) {
                                        int first = da->getFirst();
                                        int count = da->getCount();
                                        int max_num = first + count;
                                        if (max_num >= 65535) {
                                            max_num = 65535; 
                                            idxSize = 65535;
                                        }
                                        minIndex = first;
                                        maxIndex = max_num - 1;
                                        for (int i = first; i < max_num; i++) {
                                            if (max_num < 256)
                                                put_val(buffer.data, (unsigned char)i);
                                            else if (max_num < 65536)
                                                put_val(buffer.data, (unsigned short)i);
                                            else
                                                put_val(buffer.data, i);
                                        }
                                    }
                                    break;
                                }
                                default:
                                    qFatal("missing osg::PrimitiveSet::Type [%d]", t);
                                    break;
                                }
                            }

                            tinygltf::Accessor acc;
                            acc.bufferView = 0;
                            acc.byteOffset = acc_offset[j];
                            alignment_buffer(buffer.data);
                            acc_offset[j] = buffer.data.size();
                            switch (t)
                            {
                            case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
                                if (idx_size <= 256)
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
                                else if (idx_size <= 65536)
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                                else
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                                break;
                            case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
                                if (idx_size <= 65536)
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                                else
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                                break;
                            case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
                                acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                                break;
                            case osg::PrimitiveSet::DrawArraysPrimitiveType: {
                                osg::PrimitiveSet* ps = g->getPrimitiveSet(0);
                                osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(ps);
                                int first = da->getFirst();
                                int count = da->getCount();
                                int max_num = first + count;
                                if (max_num >= 65535) 
                                    max_num = 65535;
                                if (max_num < 256) {
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
                                }
                                else if (max_num < 65536) {
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                                }
                                else {
                                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                                }
                                break;
                            }
                            default:
                                //LOG_E("missing osg::PrimitiveSet::Type [%d]", t);
                                break;
                            }
                            acc.count = idxSize;
                            acc.type = TINYGLTF_TYPE_SCALAR;
                            osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                            acc.maxValues = { double(maxIndex) };
                            acc.minValues = { double(minIndex) };
                            model.accessors.push_back(acc);
                        }
                    }
                    else if (j == 1) {
                        osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                        int vec_size = v3f->size();
                        std::array<double, 3> box_max = { -1e38, -1e38 ,-1e38 };
                        std::array<double, 3> box_min = { 1e38, 1e38 ,1e38 };
                        for (int vidx = 0; vidx < vec_size; vidx++)
                        {
                            osg::Vec3f point = v3f->at(vidx);
                            put_val(buffer.data, point.x());
                            put_val(buffer.data, point.y());
                            put_val(buffer.data, point.z());
                            if (point.x() > box_max[0]) box_max[0] = point.x();
                            if (point.x() < box_min[0]) box_min[0] = point.x();
                            if (point.y() > box_max[1]) box_max[1] = point.y();
                            if (point.y() < box_min[1]) box_min[1] = point.y();
                            if (point.z() > box_max[2]) box_max[2] = point.z();
                            if (point.z() < box_min[2]) box_min[2] = point.z();
                        }
                        tinygltf::Accessor acc;
                        acc.bufferView = 1;
                        acc.byteOffset = acc_offset[j];
                        alignment_buffer(buffer.data);
                        acc_offset[j] = buffer.data.size() - buf_offset;
                        acc.count = vec_size;
                        acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                        acc.type = TINYGLTF_TYPE_VEC3;
                        acc.maxValues = std::vector<double>(box_max.begin(), box_max.end());
                        acc.minValues = std::vector<double>(box_min.begin(), box_min.end());
                        model.accessors.push_back(acc);

                        // calc the box
                        MeshInfo osgb_info;
                        osgb_info.name = QString::fromStdString(g->getName());
                        osgb_info.min = box_min;
                        osgb_info.max = box_max;
                        v_info.push_back(osgb_info);
                    }
                    else if (j == 2) {
                        // normal
                        std::array<double, 3> box_max = { -1e38, -1e38, -1e38 };
                        std::array<double, 3> box_min = { 1e38, 1e38, 1e38 };
                        int normal_size = 0;
                        osg::Array* na = g->getNormalArray();
                        if (na)
                        {
                            osg::Vec3Array* v3f = (osg::Vec3Array*)na;
                            normal_size = v3f->size();
                            for (int vidx = 0; vidx < normal_size; vidx++)
                            {
                                osg::Vec3f point = v3f->at(vidx);
                                put_val(buffer.data, point.x());
                                put_val(buffer.data, point.y());
                                put_val(buffer.data, point.z());

                                if (point.x() > box_max[0]) box_max[0] = point.x();
                                if (point.x() < box_min[0]) box_min[0] = point.x();
                                if (point.y() > box_max[1]) box_max[1] = point.y();
                                if (point.y() < box_min[1]) box_min[1] = point.y();
                                if (point.z() > box_max[2]) box_max[2] = point.z();
                                if (point.z() < box_min[2]) box_min[2] = point.z();
                            }
                        }
                        else { // mesh 没有法线坐标 
                            osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                            int vec_size = v3f->size();
                            normal_size = vec_size;
                            box_max = { 0,0 };
                            box_min = { 0,0 };
                            for (int vidx = 0; vidx < vec_size; vidx++)
                            {
                                float x = 0;
                                put_val(buffer.data, x);
                                put_val(buffer.data, x);
                                put_val(buffer.data, x);
                            }
                        }
                        tinygltf::Accessor acc;
                        acc.bufferView = 2;
                        acc.byteOffset = acc_offset[j];
                        alignment_buffer(buffer.data);
                        acc_offset[j] = buffer.data.size() - buf_offset;
                        acc.count = normal_size;
                        acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                        acc.type = TINYGLTF_TYPE_VEC3;
                        acc.maxValues = std::vector<double>(box_max.begin(), box_max.end());
                        acc.minValues = std::vector<double>(box_min.begin(), box_min.end());
                        model.accessors.push_back(acc);
                    }
                    else if (j == 3) {
                        // text
                        std::array<double, 3> box_max = { -1e38, -1e38 };
                        std::array<double, 3> box_min = { 1e38, 1e38 };
                        int texture_size = 0;
                        osg::Array* na = g->getTexCoordArray(0);
                        if (na) {
                            osg::Vec2Array* v2f = (osg::Vec2Array*)na;
                            texture_size = v2f->size();
                            for (int vidx = 0; vidx < texture_size; vidx++)
                            {
                                osg::Vec2f point = v2f->at(vidx);
                                put_val(buffer.data, point.x());
                                put_val(buffer.data, point.y());
                                if (point.x() > box_max[0]) box_max[0] = point.x();
                                if (point.x() < box_min[0]) box_min[0] = point.x();
                                if (point.y() > box_max[1]) box_max[1] = point.y();
                                if (point.y() < box_min[1]) box_min[1] = point.y();
                            }
                        }
                        else { // mesh 没有纹理坐标
                            osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                            int vec_size = v3f->size();
                            texture_size = vec_size;
                            box_max = { 0,0 };
                            box_min = { 0,0 };
                            for (int vidx = 0; vidx < vec_size; vidx++)
                            {
                                float x = 0;
                                put_val(buffer.data, x);
                                put_val(buffer.data, x);
                            }
                        }
                        tinygltf::Accessor acc;
                        acc.bufferView = 3;
                        acc.byteOffset = acc_offset[j];
                        alignment_buffer(buffer.data);
                        acc_offset[j] = buffer.data.size() - buf_offset;
                        acc.count = texture_size;
                        acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                        acc.type = TINYGLTF_TYPE_VEC2;
                        acc.maxValues = std::vector<double>(box_max.begin(), box_max.end());
                        acc.minValues = std::vector<double>(box_min.begin(), box_min.end());
                        model.accessors.push_back(acc);
                    }
                }
                tinygltf::BufferView bfv;
                bfv.buffer = 0;
                if (j == 0) {
                    bfv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
                }
                else {
                    bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
                }
                bfv.byteOffset = buf_offset;
                alignment_buffer(buffer.data);
                bfv.byteLength = buffer.data.size() - buf_offset;
                buf_offset = buffer.data.size();
                if (infoVisitor.geometry_array.size() > 1) {
                    if (j == 1) { bfv.byteStride = 4 * 3; }
                    if (j == 2) { bfv.byteStride = 4 * 3; }
                    if (j == 3) { bfv.byteStride = 4 * 2; }
                }
                model.bufferViews.push_back(bfv);
            }
            // image
            {
                int buf_view = 4;
                for (auto tex : lodVisitor.textureArray) {
                    std::vector<unsigned char> jpeg_buf;
                    jpeg_buf.reserve(512 * 512 * 3);
                    int width, height, comp;
                    {
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
                                        fill_4BitImage(jpeg_buf, img, width, height);
                                    }
                                    else
                                    {
                                        unsigned row_step = img->getRowStepInBytes();
                                        unsigned row_size = img->getRowSizeInBytes();
                                        for (size_t i = 0; i < height; i++)
                                        {
                                            jpeg_buf.insert(jpeg_buf.end(),
                                                img->data() + row_step * i,
                                                img->data() + row_step * i + row_size);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (!jpeg_buf.empty()) {
                        int buf_size = buffer.data.size();
                        buffer.data.reserve(buffer.data.size() + width * height * comp);
                        stbi_write_jpg_to_func(stbi_write_buf, &buffer.data, width, height, comp, jpeg_buf.data(), 80);
                    }
                    else {
                        std::vector<char> v_data;
                        width = height = 256;
                        v_data.resize(width * height * 3);
                        stbi_write_jpg_to_func(stbi_write_buf, &buffer.data, width, height, 3, v_data.data(), 80);
                    }
                    tinygltf::Image image;
                    image.mimeType = "image/jpeg";

                    image.bufferView = buf_view++;
                    model.images.push_back(image);
                    tinygltf::BufferView bfv;
                    bfv.buffer = 0;
                    bfv.byteOffset = buf_offset;
                    bfv.byteLength = buffer.data.size() - buf_offset;
                    alignment_buffer(buffer.data);
                    buf_offset = buffer.data.size();
                    model.bufferViews.push_back(bfv);
                }
            }
            // mesh 
            {
                int MeshNum = lodVisitor.geometryArray.size();
                for (int i = 0; i < MeshNum; i++) {
                    tinygltf::Mesh mesh;
                    //mesh.name = meshes[i].mesh_name;
                    tinygltf::Primitive primits;
                    primits.attributes = {
                        //std::pair<std::string,int>("_BATCHID", 2 * i + 1),
                        std::pair<std::string,int>("POSITION", 1 * MeshNum + i),
                        std::pair<std::string,int>("NORMAL",   2 * MeshNum + i),
                        std::pair<std::string,int>("TEXCOORD_0",   3 * MeshNum + i),
                    };
                    primits.indices = i;
                    primits.material = 0;
                    if (lodVisitor.textureArray.size() > 1) {
                        auto geomtry = lodVisitor.geometryArray[i];
                        auto tex = lodVisitor.textureMap[geomtry];
                        for (auto texture : lodVisitor.textureArray) {
                            if (tex != texture) {
                                primits.material++;
                            }
                            else {
                                break;
                            }
                        }
                    }
                    primits.mode = TINYGLTF_MODE_TRIANGLES;
                    mesh.primitives = { primits };
                    model.meshes.push_back(mesh);
                }

                // 加载所有的模型
                for (int i = 0; i < MeshNum; i++) {
                    tinygltf::Node node;
                    node.mesh = i;
                    // 转 Up YAxis
                    // reference: https://blog.csdn.net/taiyang1987912/article/details/109758623
                    node.matrix = { 1,0,0,0,0,0,-1,0,0,1,0,0,0,0,0,1 };
                    model.nodes.push_back(node);
                }
            }
            // scene
            {
                // 一个场景
                tinygltf::Scene sence;
                for (int i = 0; i < lodVisitor.geometryArray.size(); i++) {
                    sence.nodes.push_back(i);
                }
                // 所有场景
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
           
            // use shader material
            make_gltf2_shader(model, lodVisitor.textureArray.size(), buffer, buf_offset);
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
            model.asset.generator = "fanfan";

            return QByteArray::fromStdString(gltf.Serialize(&model));
        }
    }
};

double get_geometric_error(TileBox& bbox);



std::string vs_str();

std::string fs_str();

std::string program(int vs, int fs);

std::string tech_string();

void make_gltf2_shader(tinygltf::Model& model, int mat_size, tinygltf::Buffer& buffer, uint32_t& buf_offset);

tinygltf::Material make_color_material_osgb(double r, double g, double b);


QByteArray osgb2glb_buf(const QString& path, QVector<MeshInfo>& v_info);

/// <summary>
/// osgb转b3dm
/// </summary>
QByteArray osgb2b3dm_buf(const QString& pat, TileBox& tile_box);

Box convert_bbox(const TileBox& tile);

/// <summary>
/// 先生成 b3dm, 再统一外扩模型的 bbox
/// </summary>
bool do_tile_job(OsgTree& tree, const QString& out_path);

TileBox extend_tile_box(OsgTree& tree);

QJsonObject get_boundingBox(const TileBox& bbox);

QJsonObject get_boundingRegion(const TileBox& bbox, double x, double y);

void calc_geometric_error(OsgTree& tree);

QJsonObject encode_tile_json(OsgTree& tree, double x, double y);


bool osgb2b3dm(const QString& in, const QString& out);

// 所有接口都是 utf8 字符串
bool osgb2glb(const QString& in, const QString& out);

bool osgb_batch_convert(
    const QString& input_dir,
    const QString& output_dir,
    int max_lvl,
    int thread_count,
    double center_x,
    double center_y,
    std::optional<double> region_offset);


Box box_to_tileset_box(const Range& r);


class OSGBTo3DTilesTask : public QRunnable {
public:
    OSGBTo3DTilesTask(
        const QString& input, const QString& output,
        int maxLevel, double centerX, double centerY)
        : mInput(input), mOutput(output), mMaxLevel(maxLevel), mCenterX(centerX), mCenterY(centerY)
    {
        setAutoDelete(false);
    }

    virtual void run() override {
        qInfo() << "start process tile: " << mInput;
        mOsgbInfo = osgb23dtile_path(mInput, mOutput, mCenterX, mCenterY, mMaxLevel);
    }

    bool isSuccess() const {
        return mOsgbInfo.has_value();
    }

    OsgbInfo value() const {
        return mOsgbInfo.value();
    }


private:
    /// <summary>
    /// 将Tile文件夹下的根节点OSGB文件转为B3DM
    /// </summary>
    /// <param name="input">Tile文件夹下的根节点OSGB完整路径</param>
    /// <param name="output">输出根目录</param>
    /// <param name="maxLevel">处理到OSGB的最大层级</param>
    /// <param name="centerX">模型中心点X坐标(度)</param>
    /// <param name="centerY">模型中心点Y坐标(度)</param>
    std::optional<OsgbInfo> osgb23dtile_path(
        const QString& input, const QString& output,
        double x, double y, int maxLevel) {

        OSGBLevel levelTree(input);
        if (levelTree.getAllOSGBLevels(maxLevel)) {
            qFatal("open file [%s] fail!", input);
            return nullptr;
        }

        if (!do_tile_job(levelTree, output)) {
            return std::optional<OsgbInfo>();
        }
        // 返回 json 和 最大bbox
        extend_tile_box(root);

        // prevent for root node disappear
        calc_geometric_error(root);

        root.geometricError = 1000.0;
        root.bbox.extend(0.2);

        OsgbInfo osgb_info;
        osgb_info.box = root.bbox;
        osgb_info.json = encode_tile_json(root, x, y);
        osgb_info.output = out_path;
        return osgb_info;
    }

    bool do_tile_job(OSGBLevel& levelTree, const QString& out_path) {
        // 转 tile 
        QByteArray b3dm_buf = osgb2b3dm_buf(tree.file_name, tree.bbox);
        // 根据osgb文件名获取对应的b3dm输出路径
        QString out_file = out_path + "/" + get_file_name(tree.file_name).replace(".osgb", ".b3dm");

        if (b3dm_buf.isEmpty())
            return false;
        write_file(out_file, b3dm_buf);
        for (auto& i : tree.sub_nodes) {
            do_tile_job(i, out_path);
        }
        return true;
    }

    QString mInput;
    QString mOutput;
    int mMaxLevel;
    double mCenterX;
    double mCenterY;

    std::optional<OsgbInfo> mOsgbInfo;
};