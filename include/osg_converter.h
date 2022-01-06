#pragma once

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <array>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <optional>

#include <QVariant>
#include <QMap>
#include <QList>
#include <QHash>
#include <QVector>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QRegExp>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QRunnable>
#include <QThreadPool>
#include <QSharedPointer>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

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


class InfoVisitor : public osg::NodeVisitor
{
public:
    InfoVisitor(const QString& path)
        :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), path(path)
    {}

    virtual ~InfoVisitor() {}

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
        //std::string path = node.getDatabasePath();
        int n = node.getNumFileNames();
        for (size_t i = 1; i < n; i++)
        {
            QString file_name = path + "/" + QString::fromStdString(node.getFileName(i));
            subNodeNames.append(file_name);
        }
        traverse(node);
    }

public:
    QString path;
    QVector<osg::Geometry*>             geometryArray;
    QSet<osg::Texture*>                 textureArray;
    QMap<osg::Geometry*, osg::Texture*> textureMap;  // 记录 mesh 和 texture 的关系，暂时认为一个模型最多只有一个texture
    QVector<QString>                    subNodeNames;
};


class VertexArrayVisitor : public osg::ArrayVisitor {
public:
    virtual void apply(osg::Vec3dArray& vector) {
        osg::Matrix yMatrix(
            1, 0, 0, 0,
            0, 0, 1, 0,
            0, -1, 0, 0,
            0, 0, 0, 1
        );

        for (unsigned int i = 0; i < vector.size(); i++) {
            osg::Vec3d vec = vector[i];
            vector[i] = yMatrix * vector[i];
        }

    }

    virtual void apply(osg::Vec3Array& vector) {
        osg::Matrix yMatrix(
            1, 0, 0, 0,
            0, 0, 0, 1,
            1, 0, 0, 0,
            0, 0, 0, 1
        );

        for (int i = 0; i < vector.size(); i++) {
            osg::Vec3 vec = vector[i];
            vector[i] = yMatrix * vector[i];
        }
    }
};



class OSGBLevel {
public:
    using OSGBLevelPtr = QSharedPointer<OSGBLevel>;

    double geometricError;
    QString nodeName;
    QVector<OSGBLevel> subNodes;

    int getLevelNumber() {
        int p0 = nodeName.indexOf("_L");
        if (p0 < 0) 
            return -1;
        int p1 = nodeName.indexOf("_", p0 + 2);
        if (p1 < 0) 
            return -1;
        return nodeName.mid(p0 + 2, p1 - p0 - 2).toInt();
    }

    bool getAllOSGBLevels(int maxLevel) {
        if (getLevelNumber() > maxLevel)
            return false;

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
};

double get_geometric_error(TileBox& bbox);

/// <summary>
/// 提取 Tile_+154_+018_L22_0000320.osgb _L后面的数字
/// 提取失败，返回 0
/// </summary>
int get_lvl_num(const QString& file_name);

std::optional<OsgTree> get_all_tree(const QString& file_name, int maxlvl);




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
        const QString& in_path, const QString& out_path,
        double x, double y, int max_lvl) {

        std::optional<OsgTree> root_optonal = get_all_tree(in_path, max_lvl);
        if (!root_optonal.has_value()) {
            qFatal("open file [%s] fail!", in_path);
            return std::optional<OsgbInfo>();
        }

        OsgTree& root = root_optonal.value();

        if (!do_tile_job(root, out_path)) {
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

    bool do_tile_job(OsgTree& tree, const QString& out_path) {
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