#include <GeometryMesh.h>
#include <Utils.h>
#include <Batched3DModel.h>
#include <earcut.hpp>

#include <array>
#include <algorithm>
#include <osg/Vec2d>
#include <osg/Math>
#include <tiny_gltf.h>
#include <stb_image_write.h>
#include <DxtImage.h>

namespace scially {

    void GeometryMesh::calcNormal(int baseCnt, int ptNum)
    {
        // normal stand for one triangle
        for (int i = 0; i < ptNum; i += 2) {
            osg::Vec2 nor1(vertex[baseCnt + 2 * (i + 1)][0], vertex[baseCnt + 2 * (i + 1)][1]);
            nor1 = nor1 - osg::Vec2(vertex[baseCnt + 2 * i][0], vertex[baseCnt + 2 * i][1]);
            osg::Vec3 nor3 = osg::Vec3(-nor1.y(), nor1.x(), 0);
            nor3.normalize();

            normal.append({ nor3.x(), nor3.y(), nor3.z() });
            normal.append({ nor3.x(), nor3.y(), nor3.z() });
            normal.append({ nor3.x(), nor3.y(), nor3.z() });
            normal.append({ nor3.x(), nor3.y(), nor3.z() });
        }
    }


    osg::ref_ptr<osg::Geometry> GeometryMesh::triangleMesh() {
        osg::Vec3Array* va = new osg::Vec3Array(vertex.size());
        for (int i = 0; i < vertex.size(); i++) {
            (*va)[i].set(vertex[i][0], vertex[i][1], vertex[i][2]);
        }
        osg::Vec3Array* vn = new osg::Vec3Array(normal.size());
        for (int i = 0; i < normal.size(); i++) {
            (*vn)[i].set(normal[i][0], normal[i][1], normal[i][2]);
        }
        osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry);
        geometry->setVertexArray(va);
        geometry->setNormalArray(vn);
        osg::DrawElementsUShort* drawElements = new osg::DrawElementsUShort(osg::DrawArrays::TRIANGLES);
        for (int i = 0; i < index.size(); i++) {
            drawElements->addElement(index[i][0]);
            drawElements->addElement(index[i][1]);
            drawElements->addElement(index[i][2]);
        }
        geometry->addPrimitiveSet(drawElements);
        //osgUtil::SmoothingVisitor::smooth(*geometry);
        return geometry;
    }

    void GeometryMesh::init(double centerX, double centerY, double height, OGRPolygon* polygon)
    {
        OGRLinearRing* pRing = polygon->getExteriorRing();
        int ptNum = pRing->getNumPoints();
        Q_ASSERT(ptNum >= 4);

        int ptCount = 0;
        for (int i = 0; i < ptNum; i++) {
            OGRPoint pt;
            pRing->getPoint(i, &pt);
            double bottom = pt.getZ();
            float pointX = (float)lonToMeter(pt.getX() - centerX, centerY);
            float pointY = (float)latToMeter(pt.getY() - centerY);
            vertex.append({ pointX , pointY, (float)bottom });
            vertex.append({ pointX , pointY, (float)height });
            // double vertex
            if (i != 0 && i != ptNum - 1) {
                vertex.append({ pointX , pointY, (float)bottom });
                vertex.append({ pointX , pointY, (float)height });
            }
        }
        int vertexNum = vertex.size() / 2;
        for (int i = 0; i < vertexNum; i += 2) {
            if (i != vertexNum - 1) {
                index.append({ 2 * i, 2 * i + 1, 2 * (i + 1) + 1 });
                index.append({ 2 * (i + 1), 2 * i, 2 * (i + 1) + 1 });
            }
        }
        calcNormal(0, vertexNum);
        ptCount += 2 * vertexNum;

        int innerCount = polygon->getNumInteriorRings();
        for (int j = 0; j < innerCount; j++) {
            OGRLinearRing* pRing = polygon->getInteriorRing(j);
            int ptNum = pRing->getNumPoints();
             Q_ASSERT(ptNum >= 4);

            for (int i = 0; i < ptNum; i++) {
                OGRPoint pt;
                pRing->getPoint(i, &pt);
                double bottom = pt.getZ();
                float pointX = (float)lonToMeter(pt.getX() - centerX, centerY);
                float pointY = (float)latToMeter(pt.getY() - centerX);
                vertex.append({ pointX , pointY, (float)bottom });
                vertex.append({ pointX , pointY, (float)height });
                // double vertex
                if (i != 0 && i != ptNum - 1) {
                    vertex.append({ pointX , pointY, (float)bottom });
                    vertex.append({ pointX , pointY, (float)height });
                }
            }
            vertexNum = vertex.size() / 2 - ptCount;
            for (int i = 0; i < vertexNum; i += 2) {
                if (i != vertexNum - 1) {
                    index.append({ ptCount + 2 * i,       ptCount + 2 * i + 1, ptCount + 2 * (i + 1) });
                    index.append({ ptCount + 2 * (i + 1), ptCount + 2 * i,     ptCount + 2 * (i + 1) });
                }
            }
            calcNormal(ptCount, ptNum);
            ptCount = vertex.size();
        }
        // top and bottom
        {
            using EarCutPoint = std::array<double, 2>;
            using EarCutPolygon = std::vector<std::vector<EarCutPoint>>;
            EarCutPolygon ePolygon(1);
            {
                OGRLinearRing* pRing = polygon->getExteriorRing();
                int ptNum = pRing->getNumPoints();
                for (int i = 0; i < ptNum; i++)
                {
                    OGRPoint pt;
                    pRing->getPoint(i, &pt);
                    double bottom = pt.getZ();
                    float pointX = (float)lonToMeter(pt.getX() - centerX, centerY);
                    float pointY = (float)latToMeter(pt.getY() - centerY);
                    ePolygon[0].push_back({ pointX, pointY });
                    vertex.append({ pointX , pointY, (float)bottom });
                    vertex.append({ pointX , pointY, (float)height });
                    normal.append({ 0, 0, -1 });
                    normal.append({ 0, 0, 1 });
                }
            }
            int innerCount = polygon->getNumInteriorRings();
            for (int j = 0; j < innerCount; j++)
            {
                ePolygon.resize(ePolygon.size() + 1);
                OGRLinearRing* pRing = polygon->getInteriorRing(j);
                int ptNum = pRing->getNumPoints();
                for (int i = 0; i < ptNum; i++)
                {
                    OGRPoint pt;
                    pRing->getPoint(i, &pt);
                    double bottom = pt.getZ();
                    float pointX = (float)lonToMeter(pt.getX() - centerX, centerY);
                    float pointY = (float)latToMeter(pt.getY() - centerY);
                    ePolygon[j].push_back({ pointX, pointY });
                    vertex.push_back({ pointX , pointY, (float)bottom });
                    vertex.push_back({ pointX , pointY, (float)height });
                    normal.push_back({ 0, 0, -1 });
                    normal.push_back({ 0, 0, 1 });
                }
            }
            std::vector<int> indices = mapbox::earcut<int>(ePolygon);
            for (int idx = 0; idx < indices.size(); idx += 3) {
                index.append({
                    ptCount + 2 * indices[idx],
                    ptCount + 2 * indices[idx + 2],
                    ptCount + 2 * indices[idx + 1] });
            }
            for (int idx = 0; idx < indices.size(); idx += 3) {
                index.append({
                    ptCount + 2 * indices[idx] + 1,
                    ptCount + 2 * indices[idx + 1] + 1,
                    ptCount + 2 * indices[idx + 2] + 1});
            }
        }
    }

    void GeometryMesh::add(double centerX, double centerY, double height, OGRPolygon* polygon) {
        static long long id = 0; // mesh name
        GeometryMesh* mesh = new GeometryMesh;
        mesh->meshName = QString("mesh_%1").arg(id++);
        mesh->height = height;
        mesh->init(centerX, centerY, height, polygon);
        meshes.append(mesh);
    }

    void GeometryMesh::add(double centerX, double centerY, double height, OGRMultiPolygon* polygons) {
        int subCount = polygons->getNumGeometries();
        for (int j = 0; j < subCount; j++) {
            OGRPolygon* polygon = (OGRPolygon*)polygons->getGeometryRef(j);
            add(centerX, centerY, height, polygon);
        }
    }

    QVector<int> GeometryMesh::getBatchID() const {
        QVector<int> ids;
        for (int i = 0; i < meshes.size(); i++)
            ids.append(i);

        return ids;
    }
    QVector<QString> GeometryMesh::getBatchName() const {
        QVector<QString> name;
        for (int i = 0; i < meshes.size(); i++)
            name.append(meshes[i]->meshName);

        return name;
    }

    QVector<double> GeometryMesh::getBatchHeights() const {
        QVector<double> heights;
        for (int i = 0; i < meshes.size(); i++)
            heights.append(meshes[i]->height);

        return heights;
    }

    QByteArray GeometryMesh::getGlbBuffer() const {
        QVector<osg::ref_ptr<osg::Geometry>> osgGeoms;
        for (auto mesh : meshes) {
            osgGeoms.append(mesh->triangleMesh());
        }

        tinygltf::TinyGLTF gltf;
        tinygltf::Model model;
        tinygltf::Buffer buffer;

        uint32_t bufOffset = 0;
        auto calcOffset = [&]() -> int {
            return buffer.data.size() - bufOffset;
        };

        uint32_t accOffset[4] = { 0,0,0,0 };
        const int bufTimes = 4;
        for (int j = 0; j < bufTimes; j++)
        {
            for (int i = 0; i < meshes.size(); i++) {
                if (osgGeoms[i]->getNumPrimitiveSets() == 0)
                    continue;

                if (j == 0) {
                    // indc
                    osg::PrimitiveSet* ps = osgGeoms[i]->getPrimitiveSet(0);
                    int idxsize = ps->getNumIndices();
                    int maxIdx = 0;

                    const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(ps);
                    int indNum = drawElements->getNumIndices();
                    for (int m = 0; m < indNum; m++)
                    {
                        unsigned short idx = drawElements->at(m);
                        buffer.append(idx);
                        maxIdx = std::max(maxIdx, (int)idx);
                    }

                    tinygltf::Accessor acc;
                    acc.bufferView = 0;
                    acc.byteOffset = accOffset[j];
                    buffer.alignment();
                    accOffset[j] = calcOffset();
                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                    acc.count = idxsize;
                    acc.type = TINYGLTF_TYPE_SCALAR;
                    acc.maxValues = { (double)maxIdx };
                    acc.minValues = { 0.0 };
                    model.accessors.push_back(acc);
                }
                else if (j == 1) {
                    osg::Array* va = osgGeoms[i]->getVertexArray();
                    osg::Vec3Array* v3f = (osg::Vec3Array*)va;
                    int vecSize = v3f->size();
                    std::vector<double> boxMax = { -1e38, -1e38 ,-1e38 };
                    std::vector<double> boxMin = { 1e38, 1e38 ,1e38 };
                    for (int vidx = 0; vidx < vecSize; vidx++) {
                        osg::Vec3f point = v3f->at(vidx);
                        QVector<float> vertex = { point.x(), point.y(), point.z() };
                        for (int i = 0; i < 3; i++)
                        {
                            buffer.append(vertex[i]);
                            boxMax[i] = std::max(boxMax[i], (double)vertex[i]);
                            boxMin[i] = std::min(boxMin[i], (double)vertex[i]);
                        }
                    }
                    tinygltf::Accessor acc;
                    acc.bufferView = 1;
                    acc.byteOffset = accOffset[j];
                    buffer.alignment();
                    accOffset[j] = calcOffset();
                    acc.count = vecSize;
                    acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    acc.type = TINYGLTF_TYPE_VEC3;
                    acc.maxValues = boxMax;
                    acc.minValues = boxMin;
                    model.accessors.push_back(acc);
                }
                else if (j == 2) {
                    // normal
                    osg::Array* na = osgGeoms[i]->getNormalArray();
                    if (na == nullptr)
                        continue;

                    osg::Vec3Array* v3f = (osg::Vec3Array*)na;
                    std::vector<double> boxMax = { -1e38, -1e38 ,-1e38 };
                    std::vector<double> boxMin = { 1e38, 1e38 ,1e38 };
                    int normalSize = v3f->size();
                    for (int vidx = 0; vidx < normalSize; vidx++)
                    {
                        osg::Vec3f point = v3f->at(vidx);
                        QVector<float> normal = { point.x(), point.y(), point.z() };

                        for (int i = 0; i < 3; i++)
                        {
                            buffer.append(normal[i]);
                            boxMax[i] = std::max(boxMax[i], (double)normal[i]);
                            boxMin[i] = std::min(boxMin[i], (double)normal[i]);
                        }
                    }
                    tinygltf::Accessor acc;
                    acc.bufferView = 2;
                    acc.byteOffset = accOffset[j];
                    buffer.alignment();
                    accOffset[j] = calcOffset();
                    acc.count = normalSize;
                    acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    acc.type = TINYGLTF_TYPE_VEC3;
                    acc.minValues = boxMin;
                    acc.maxValues = boxMax;
                    model.accessors.push_back(acc);
                }
                else if (j == 3) {
                    // batch id
                    unsigned short batchID = i;
                    for (const auto& vertex : meshes[i]->vertex) {
                        buffer.append(batchID);
                    }
                    tinygltf::Accessor acc;
                    acc.bufferView = 3;
                    acc.byteOffset = accOffset[j];
                    buffer.alignment();
                    accOffset[j] = calcOffset();
                    acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                    acc.count = meshes[i]->vertex.size();
                    acc.type = TINYGLTF_TYPE_SCALAR;
                    acc.maxValues = { (double)i };
                    acc.minValues = { (double)batchID };
                    model.accessors.push_back(acc);
                }
            }
            tinygltf::BufferView bfv;
            bfv.buffer = 0;
            if (j == 0 || j == 3) {
                bfv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
            }
            else {
                bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
                bfv.byteStride = 4 * 3;
            }
            bfv.byteOffset = bufOffset;
            buffer.alignment();
            bfv.byteLength = calcOffset();
            bufOffset = buffer.data.size();
            model.bufferViews.push_back(bfv);
        }

        for (int i = 0; i < meshes.size(); i++) {
            tinygltf::Mesh mesh;
            mesh.name = meshes[i]->meshName.toStdString();
            tinygltf::Primitive primits;
            primits.attributes = {
                std::pair<std::string,int>("POSITION", 1 * meshes.size() + i),
                std::pair<std::string,int>("NORMAL",   2 * meshes.size() + i),
                std::pair<std::string,int>("_BATCHID", 3 * meshes.size() + i),
            };
            primits.indices = i; 
            primits.material = 0;

            primits.mode = TINYGLTF_MODE_TRIANGLES;
            mesh.primitives = {
                primits
            };
            model.meshes.push_back(mesh);
        }

        for (int i = 0; i < meshes.size(); i++) {
            tinygltf::Node node;
            node.mesh = i;
            model.nodes.push_back(node);
        }
        tinygltf::Scene sence;
        for (int i = 0; i < meshes.size(); i++) {
            sence.nodes.push_back(i);
        }
        model.scenes = { sence };
        model.defaultScene = 0;
        
        tinygltf::Material material;
        material.name = "default";
        tinygltf::Parameter metallicFactor;
        metallicFactor.number_value = new double(0.3);
        material.values["metallicFactor"] = metallicFactor;
        tinygltf::Parameter roughnessFactor;
        roughnessFactor.number_value = new double(0.7);
        material.values["roughnessFactor"] = roughnessFactor;
    
        model.materials = { material };
        

        model.buffers.push_back(std::move(buffer));
        model.asset.version = "2.0";
        model.asset.generator = "hwang";

        std::string buf = gltf.Serialize(&model);
        return QByteArray::fromStdString(buf);
    }

    QByteArray GeometryMesh::toB3DM(bool withHeight) {
        Batched3DModel b3dm;
        
        b3dm.batchLength = meshes.size();
        b3dm.names = getBatchName();
        b3dm.batchID = getBatchID();
        
        if (withHeight)
            b3dm.heights = getBatchHeights();

        b3dm.glbBuffer = getGlbBuffer();      
        return b3dm.write(withHeight);
    }

    GeometryMesh::~GeometryMesh() {
        for (auto iter = meshes.begin(); iter != meshes.end(); iter++) {
            if (*iter != nullptr)
                delete* iter;
        }
    }
}
