#include <osg_converter.h>


void stbi_write_buf(void* context, void* data, int size) {

    std::vector<char>* buf = (std::vector<char>*)context;
    buf->insert(buf->end(), (char*)data, (char*)data + size);
}

double get_geometric_error(TileBox& bbox) {
    double max_err = std::max((bbox.max[0] - bbox.min[0]), (bbox.max[1] - bbox.min[1]));
    max_err = std::max(max_err, (bbox.max[2] - bbox.min[2]));
    if (max_err == 0)
    {
        qWarning("bbox maybe empty!");
        return 0;
    }
    return max_err / 20.0;
} 


int get_lvl_num(const QString& file_path) {
    QString stem = get_file_name(file_path);

    int p0 = stem.indexOf("_L");

    if (p0 < 0) return 0;

    int p1 = stem.indexOf("_", p0 + 2);

    if (p1 < 0) return 0;

    return stem.mid(p0 + 2, p1 - p0 - 2).toInt();
}

OSGBLevel::OSGBLevelPtr getAllOSGBLevels(const QString& file_name, int maxlvl) {

    if (get_lvl_num(get_file_name(file_name)) > maxlvl)
        return OSGBLevel::OSGBLevelPtr::create();

    OsgTree root_tile;

    InfoVisitor infoVisitor(get_parent_name(file_name));

    std::vector<std::string> osgb_files = { file_name.toStdString() };
    // add block to release Node
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(osgb_files);

    if (root == nullptr)
        return std::optional<OsgTree>();

    root_tile.file_name = file_name;
    root->accept(infoVisitor);

    for (int i = 0; i < infoVisitor.sub_node_names.size(); i++) {
        std::optional<OsgTree> tree = get_all_tree(infoVisitor.sub_node_names[i], maxlvl);
        if (tree.has_value()) {
            root_tile.sub_nodes.push_back(tree.value());
        }
    }
    return root_tile;
}


std::string vs_str() {
    return
        R"(
precision highp float;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
attribute vec3 a_position;
attribute vec2 a_texcoord0;
attribute float a_batchid;
varying vec2 v_texcoord0;
void main(void)
{   
    v_texcoord0 = a_texcoord0;
    gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(a_position, 1.0);
}
)";
}

std::string fs_str() {
    return
        R"(
precision highp float;
varying vec2 v_texcoord0;
uniform sampler2D u_diffuse;
void main(void)
{
  gl_FragColor = texture2D(u_diffuse, v_texcoord0);
}
)";
}

std::string program(int vs, int fs) {
    char buf[512];
    std::string fmt = R"(
{
"attributes": [
"a_position",
"a_texcoord0"
],
"vertexShader": %d,
"fragmentShader": %d
}
)";
    sprintf(buf, fmt.data(), vs, fs);
    return buf;
}

std::string tech_string() {
    return
        R"(
{
  "attributes": {
    "a_batchid": {
      "semantic": "_BATCHID",
      "type": 5123
    },
    "a_position": {
      "semantic": "POSITION",
      "type": 35665
    },
    "a_texcoord0": {
      "semantic": "TEXCOORD_0",
      "type": 35664
    }
  },
  "program": 0,
  "states": {
    "enable": [
      2884,
      2929
    ]
  },
  "uniforms": {
    "u_diffuse": {
      "type": 35678
    },
    "u_modelViewMatrix": {
      "semantic": "MODELVIEW",
      "type": 35676
    },
    "u_projectionMatrix": {
      "semantic": "PROJECTION",
      "type": 35676
    }
  }
})";
}

void make_gltf2_shader(tinygltf::Model& model, int mat_size, tinygltf::Buffer& buffer, uint32_t& buf_offset) {
    model.extensionsRequired = { "KHR_techniques_webgl" };
    model.extensionsUsed = { "KHR_techniques_webgl" };
    // add vs shader
    {
        tinygltf::BufferView bfv_vs;
        bfv_vs.buffer = 0;
        bfv_vs.byteOffset = buf_offset;
        bfv_vs.target = TINYGLTF_TARGET_ARRAY_BUFFER;

        std::string vs_shader = vs_str();

        buffer.data.insert(buffer.data.end(), vs_shader.begin(), vs_shader.end());
        bfv_vs.byteLength = vs_shader.size();
        alignment_buffer(buffer.data);
        buf_offset = buffer.data.size();
        model.bufferViews.push_back(bfv_vs);

        tinygltf::Shader shader;
        shader.bufferView = model.bufferViews.size() - 1;
        shader.type = TINYGLTF_SHADER_TYPE_VERTEX_SHADER;
        model.extensions.KHR_techniques_webgl.shaders.push_back(shader);
    }
    // add fs shader
    {
        tinygltf::BufferView bfv_fs;
        bfv_fs.buffer = 0;
        bfv_fs.byteOffset = buf_offset;
        bfv_fs.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        std::string fs_shader = fs_str();
        buffer.data.insert(buffer.data.end(), fs_shader.begin(), fs_shader.end());
        bfv_fs.byteLength = fs_shader.size();
        alignment_buffer(buffer.data);
        buf_offset = buffer.data.size();
        model.bufferViews.push_back(bfv_fs);

        tinygltf::Shader shader;
        shader.bufferView = model.bufferViews.size() - 1;
        shader.type = TINYGLTF_SHADER_TYPE_FRAGMENT_SHADER;
        model.extensions.KHR_techniques_webgl.shaders.push_back(shader);
    }
    // tech
    {
        tinygltf::Technique tech;
        tech.tech_string = tech_string();
        model.extensions.KHR_techniques_webgl.techniques = { tech };
    }
    // program
    {
        tinygltf::Program prog;
        prog.prog_string = program(0, 1);
        model.extensions.KHR_techniques_webgl.programs = { prog };
    }

    for (int i = 0; i < mat_size; i++)
    {
        tinygltf::Material material;
        material.name = "osgb";
        char shaderBuffer[512];
        sprintf(shaderBuffer, R"(
{
"extensions": {
"KHR_techniques_webgl": {
"technique": 0,
"values": {
"u_diffuse": {
"index": %d,
"texCoord": 0
}
}
}
}
}
)", i);
        material.shaderMaterial = shaderBuffer;
        model.materials.push_back(material);
    }
}

tinygltf::Material make_color_material_osgb(double r, double g, double b) {
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


QByteArray osgb2glb_buf(const QString& path, QVector<MeshInfo>& v_info) {
    QString parent_path = get_parent_name(path);
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFile(path.toStdString());

    if (root == nullptr)
        return QByteArray();

    InfoVisitor infoVisitor(parent_path);
    root->accept(infoVisitor);
    if (infoVisitor.geometry_array.empty())
        return QByteArray();

    osgUtil::SmoothingVisitor sv;
    root->accept(sv);

    {
        tinygltf::TinyGLTF gltf;
        tinygltf::Model model;
        tinygltf::Buffer buffer;
        // buffer_view {index,vertex,normal(texcoord,image)}
        uint32_t buf_offset = 0;
        uint32_t acc_offset[4] = { 0,0,0,0 };
        for (int j = 0; j < 4; j++)
        {
            for (auto g : infoVisitor.geometry_array) {
                if (g->getNumPrimitiveSets() == 0) {
                    continue;
                }
                osg::Array* va = g->getVertexArray();
                // 转 UpAxis Y:
                // va->accept(VertexArrayVisitor());

                if (j == 0) {
                    // indc
                    {
                        int max_index = 0, min_index = 1 << 30;
                        int idx_size = 0;
                        osg::PrimitiveSet::Type t = g->getPrimitiveSet(0)->getType();
                        for (int k = 0; k < g->getNumPrimitiveSets(); k++)
                        {
                            osg::PrimitiveSet* ps = g->getPrimitiveSet(k);
                            if (t != ps->getType())
                            {
                                qFatal("PrimitiveSets type are NOT same in osgb");
                            }
                            idx_size += ps->getNumIndices();
                        }
                        for (int k = 0; k < g->getNumPrimitiveSets(); k++)
                        {
                            osg::PrimitiveSet* ps = g->getPrimitiveSet(k);
                            switch (t)
                            {
                            case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
                            {
                                const osg::DrawElementsUByte* drawElements = static_cast<const osg::DrawElementsUByte*>(ps);
                                int IndNum = drawElements->getNumIndices();
                                for (size_t m = 0; m < IndNum; m++)
                                {
                                    if (idx_size <= 256)
                                        put_val(buffer.data, drawElements->at(m));
                                    else if (idx_size <= 65536)
                                        put_val(buffer.data, (unsigned short)drawElements->at(m));
                                    else
                                        put_val(buffer.data, (unsigned int)drawElements->at(m));
                                    if (drawElements->at(m) > max_index) max_index = drawElements->at(m);
                                    if (drawElements->at(m) < min_index) min_index = drawElements->at(m);
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
                                for (size_t m = 0; m < IndNum; m++)
                                {
                                    put_val(buffer.data, drawElements->at(m));
                                    if (drawElements->at(m) > (unsigned)max_index) max_index = drawElements->at(m);
                                    if (drawElements->at(m) < (unsigned)min_index) min_index = drawElements->at(m);
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
                                        max_num = 65535; idx_size = 65535;
                                    }
                                    min_index = first;
                                    max_index = max_num - 1;
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
                            {
                                qFatal("missing osg::PrimitiveSet::Type [%d]", t);
                                break;
                            }
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
                            if (max_num >= 65535) max_num = 65535;
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
                        acc.count = idx_size;
                        acc.type = TINYGLTF_TYPE_SCALAR;
                        osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                        int vec_size = v3f->size();
                        acc.maxValues = { (double)max_index };
                        acc.minValues = { (double)min_index };
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
            for (auto tex : infoVisitor.texture_array) {
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
            int MeshNum = infoVisitor.geometry_array.size();
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
                if (infoVisitor.texture_array.size() > 1) {
                    auto geomtry = infoVisitor.geometry_array[i];
                    auto tex = infoVisitor.texture_map[geomtry];
                    for (auto texture : infoVisitor.texture_array) {
                        if (tex != texture) {
                            primits.material++;
                        }
                        else {
                            break;
                        }
                    }
                }
                primits.mode = TINYGLTF_MODE_TRIANGLES;
                mesh.primitives = {
                    primits
                };
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
            for (int i = 0; i < infoVisitor.geometry_array.size(); i++) {
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
        /// --------------
        if (0)
        {
            for (int i = 0; i < infoVisitor.texture_array.size(); i++)
            {
                tinygltf::Material mat = make_color_material_osgb(1.0, 1.0, 1.0);
                // 可能会出现多材质的情况
                tinygltf::Parameter baseColorTexture;
                baseColorTexture.json_int_value = { std::pair<std::string,int>("index",i) };
                mat.values["baseColorTexture"] = baseColorTexture;
                model.materials.push_back(mat);
            }
        }
        // use shader material
        else {
            make_gltf2_shader(model, infoVisitor.texture_array.size(), buffer, buf_offset);
        }
        // finish buffer
        model.buffers.push_back(std::move(buffer));
        // texture
        {
            int texture_index = 0;
            for (auto tex : infoVisitor.texture_array)
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

QByteArray osgb2b3dm_buf(const QString& path, TileBox& tile_box) {
    QByteArray b3dm_buf;
    using nlohmann::json;

    QVector<MeshInfo> v_info;
    QByteArray glb_buf = osgb2glb_buf(path, v_info);
    if (glb_buf.isEmpty())
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


Box convert_bbox(const TileBox& tile) {
    double center_mx = (tile.max[0] + tile.min[0]) / 2;
    double center_my = (tile.max[1] + tile.min[1]) / 2;
    double center_mz = (tile.max[2] + tile.min[2]) / 2;
    double x_meter = (tile.max[0] - tile.min[0]) * 1;
    double y_meter = (tile.max[1] - tile.min[1]) * 1;
    double z_meter = (tile.max[2] - tile.min[2]) * 1;
    if (x_meter < 0.01) { x_meter = 0.01; }
    if (y_meter < 0.01) { y_meter = 0.01; }
    if (z_meter < 0.01) { z_meter = 0.01; }

    return  {
        center_mx,center_my,center_mz,
        x_meter / 2, 0, 0,
        0, y_meter / 2, 0,
        0, 0, z_meter / 2
    };
}




void expend_box(TileBox& box1, const TileBox& box2) {

    for (int i = 0; i < 3; i++) {
        box1.min[i] = std::min(box1.min[i], box2.min[i]);
        box1.max[i] = std::max(box1.max[i], box2.max[i]);
    }
}

TileBox extend_tile_box(OsgTree& tree) {
    TileBox box = tree.bbox;
    for (auto& node : tree.sub_nodes) {
        TileBox sub_tile = extend_tile_box(node);
        expend_box(box, sub_tile);
    }
    tree.bbox = box;
    return box;
}

QJsonObject get_boundingBox(const TileBox& bbox) {
    QJsonObject box_json;

    Box v_box = convert_bbox(bbox);
    box_json["box"] = array2json(v_box);

    return box_json;
}

QJsonObject get_boundingRegion(const TileBox& bbox, double x, double y) {
    QJsonObject bounding_json;
    QJsonObject box_json;

    Range v_box;
    v_box[0] = meter_to_longti(bbox.min[0], y) + x;
    v_box[1] = meter_to_lati(bbox.min[1]) + y;
    v_box[2] = meter_to_longti(bbox.max[0], y) + x;
    v_box[3] = meter_to_lati(bbox.max[1]) + y;
    v_box[4] = bbox.min[2];
    v_box[5] = bbox.max[2];

    box_json["region"] = array2json(v_box);
    bounding_json["boundingVolume"] = box_json;

    return bounding_json;
}

void calc_geometric_error(OsgTree& tree) {
    // depth first
    for (auto& i : tree.sub_nodes) {
        calc_geometric_error(i);
    }

    if (tree.sub_nodes.isEmpty()) {
        tree.geometricError = 0.0;
    }
    else {
        bool has = false;
        OsgTree leaf;
        for (auto& i : tree.sub_nodes) {
            if (abs(i.geometricError) > EPS)
            {
                has = true;
                leaf = i;
            }
        }

        if (has == false)
            tree.geometricError = get_geometric_error(tree.bbox);
        else
            tree.geometricError = leaf.geometricError * 2.0;
    }
}

QJsonObject encode_tile_json(OsgTree& tree, double x, double y) {
    if (tree.bbox.max.empty() || tree.bbox.min.empty()) {
        return QJsonObject();
    }

    QString file_name = get_file_name(tree.file_name);
    QString parent_str = get_parent_name(tree.file_name);
    QString file_path = get_file_name(parent_str);

    QJsonObject title_json;
    title_json["geometricError"] = tree.geometricError;

    TileBox cBox = tree.bbox;
    QJsonObject content_box = get_boundingBox(cBox);


    QJsonObject content;
    QString uri = "./" + file_name;
    content["uri"] = QString("./" + file_name).replace(".osgb", ".b3dm");
    content["boundingVolume"] = content_box;
    title_json["content"] = content;
    title_json["boundingVolume"] = content_box;


    QJsonArray children;
    for (auto& i : tree.sub_nodes) {
        QJsonObject node_json = encode_tile_json(i, x, y);
        if (!node_json.isEmpty()) {
            children.push_back(node_json);
        }
    }
    title_json["children"] = children;

    return title_json;
}




bool osgb2b3dm(const QString& in, const QString& out) {
    TileBox tile_box;

    QByteArray b3dm_buf = osgb2b3dm_buf(in, tile_box);

    if (b3dm_buf.isEmpty())
        return false;

    bool ret = write_file(out, b3dm_buf);
    if (!ret)
        return false;

    QString b3dm_file_name = get_file_name(out);
    // write tileset.json
    QString tileset = b3dm_file_name.replace(".b3dm", ".json");

    double center_mx = (tile_box.max[0] + tile_box.min[0]) / 2;
    double center_my = (tile_box.max[2] + tile_box.min[2]) / 2;
    double center_mz = (tile_box.max[1] + tile_box.min[1]) / 2;

    double width_meter = tile_box.max[0] - tile_box.min[0];
    double height_meter = tile_box.max[2] - tile_box.min[2];
    double z_meter = tile_box.max[1] - tile_box.min[1];
    if (width_meter < 0.01) { width_meter = 0.01; }
    if (height_meter < 0.01) { height_meter = 0.01; }
    if (z_meter < 0.01) { z_meter = 0.01; }

    Matrix matrix = {
        center_mx,          center_my,          center_mz,
        width_meter / 2,    0,                  0,
        0,                  height_meter / 2,   0,
        0,                  0,                  z_meter / 2
    };

    Box box = {
        center_mx,          center_my,          center_mz,
        width_meter / 2,    0,                  0,
        0,                  height_meter / 2,   0,
        0,                  0,                  z_meter / 2
    };


    return write_tileset_box(
        matrix, box, 100,
        b3dm_file_name,
        tileset);
}


bool osgb2glb(const QString& in, const QString& out) {

    QVector<MeshInfo> v_info;
    QByteArray b3dm_buf = osgb2glb_buf(in, v_info);
    if (b3dm_buf.isEmpty())
        return false;

    return write_file(out, b3dm_buf);
}

bool osgb_batch_convert(
    const QString& input_path,
    const QString& output_path,
    int max_lvl,
    int max_thread,
    double center_x,
    double center_y,
    std::optional<double> region_offset) {

    double rad_x = degree2rad(center_x);
    double rad_y = degree2rad(center_y);

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->setMaxThreadCount(max_thread);

    long task_count = 0;
    QVector<OsgbInfo> osgb_infos;

    QDir input_dir(input_path + "/Data");
    if (!input_dir.exists()) {
        qFatal("dir %s not exist", input_path + "/Data");
        return false;
    }

    if (!create_dir(output_path)) {
        qFatal("dir %s create failed", output_path);
        return false;
    }

    QDirIterator dir_iter(input_dir);

    QVector<Osgb23dtilesTask*> tasks;

    while (dir_iter.hasNext()) {

        dir_iter.next();
        QDir sub_dir(dir_iter.filePath());

        if (sub_dir.exists()) {
            QString tilename = sub_dir.dirName();
            // Qt DirIterator 会遍历 . 和 ..
            if (tilename == "." || tilename == "..")
                continue;

            QString tilepath = dir_iter.filePath() + "/" + tilename + ".osgb";
            QFile root_osgb_file(tilepath);
            if (root_osgb_file.exists()) {
                QString b3dm_path = output_path + "/" + tilename;
                if (!create_dir(b3dm_path)) {
                    qWarning() << "dir" << b3dm_path << "create failed";
                    continue;
                }

                Osgb23dtilesTask* task = new Osgb23dtilesTask(tilepath, b3dm_path, max_lvl, rad_x, rad_y);
                tasks.push_back(task);
                threadPool->start(task);
                // auto future = std::async(std::launch::async, osgb23dtile_path, tilepath, b3dm_path, rad_x, rad_y, max_lvl);
                // futures.push_back(std::move(future));
            }
        }
    }
    TileBox root_tile;
    root_tile.max = std::array<double, 3>{
        std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::lowest()};

    root_tile.min = std::array<double, 3>{
        std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max()};

    threadPool->waitForDone();

    QVector<OsgbInfo> osgb_tiles;
    for (auto task : tasks) {
        if (!task->isSuccess())
            continue;

        OsgbInfo tile = task->value();
        osgb_tiles.push_back(tile);
        for (int i = 0; i < 3; i++) {
            root_tile.max[i] = std::max(tile.box.max[i], root_tile.max[i]);
            root_tile.min[i] = std::min(tile.box.min[i], root_tile.min[i]);
        }
        delete task;
    }

    double tras_height = 0.0;
    if (region_offset.has_value()) {
        tras_height = region_offset.value() - root_tile.min[2];
    }

    Matrix v = transfrom_xyz(degree2rad(center_x), degree2rad(center_y), tras_height);
    Box box = box_to_tileset_box({ root_tile.max[0],root_tile.max[1],root_tile.max[2],root_tile.min[0],root_tile.min[1],root_tile.min[2] });
    QJsonObject merge_json = make_tile_json(2000, v, box, std::optional<QString>(), std::optional<QJsonArray>());
    QJsonArray merge_json_children;
    for (auto& tile : osgb_tiles) {
        QJsonObject children_json = make_tiles_json(1000, tile.json);
        write_file(tile.output + "/tileset.json", QJsonDocument(children_json).toJson());

        // 1 取出Tile的Box和Name
        QString tilename = get_file_name(tile.output);
        QJsonObject tile_json;
        tile_json["boundingVolume"] = tile.json["boundingVolume"];
        tile_json["geometricError"] = 1000;
        QJsonObject content_json;
        content_json["uri"] = "./" + tilename + "/tileset.json";
        tile_json["content"] = content_json;
        merge_json_children.append(tile_json);
    }

    //merge_json = make_tiles_json(2000, children_json);
    merge_json = make_tile_json(2000, v, box, std::optional<QString>(), merge_json_children);
    merge_json = make_tiles_json(2000, merge_json);
    write_file(output_path + "./tileset.json", QJsonDocument(merge_json).toJson());
    return true;
}

Box box_to_tileset_box(const Range& r) {
    Box box;
    box[0] = (r[0] + r[3]) / 2.0;
    box[1] = (r[1] + r[4]) / 2.0;
    box[2] = (r[2] + r[5]) / 2.0;

    box[3] = (r[3] - r[0]) / 2.0;
    box[4] = (0.0);
    box[5] = (0.0);

    box[6] = (0.0);
    box[7] = (r[4] - r[1]) / 2.0;
    box[8] = (0.0);

    box[9] = (0.0);
    box[10] = (0.0);
    box[11] = (r[5] - r[2]) / 2.0;

    return box;
}