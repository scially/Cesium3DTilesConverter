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