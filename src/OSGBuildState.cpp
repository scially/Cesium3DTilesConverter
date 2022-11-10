#include <OSGBuildState.h>

#include <QtDebug>

namespace scially {

    bool OSGBuildState::appendOSGGeometry(const osg::Geometry* geom) {
        osg::PrimitiveSet::Type type = geom->getPrimitiveSet(0)->getType();
        OSGPrimitiveState pmtState = { -1, -1, -1 };
        for (unsigned int k = 0; k < geom->getNumPrimitiveSets(); k++)
        {
            const osg::PrimitiveSet* ps = geom->getPrimitiveSet(k);
            if (type != ps->getType())
            {
                qCritical("PrimitiveSets type are not same in osgb");
                return false;
            }
            appendPrimitive(geom, ps, pmtState);
        }
        return true;
    }


    void OSGBuildState::expandBBox3d(osg::Vec3f& minPoint, osg::Vec3f& maxPoint, const osg::Vec3f& point)
    {
        maxPoint.x() = std::max(point.x(), maxPoint.x());
        minPoint.x() = std::min(point.x(), minPoint.x());
        maxPoint.y() = std::max(point.y(), maxPoint.y());
        minPoint.y() = std::min(point.y(), minPoint.y());
        maxPoint.z() = std::max(point.z(), maxPoint.z());
        minPoint.z() = std::min(point.z(), minPoint.z());
    }

    void OSGBuildState::expandBBox2d(osg::Vec2f& minPoint, osg::Vec2f& maxPoint, const osg::Vec2f& point)
    {
        maxPoint.x() = std::max(point.x(), maxPoint.x());
        minPoint.x() = std::min(point.x(), minPoint.x());
        maxPoint.y() = std::max(point.y(), maxPoint.y());
        minPoint.y() = std::min(point.y(), minPoint.y());
    }

    void OSGBuildState::appendVec2Array(const osg::Vec2Array* v2f, osg::Vec2f& minPoint, osg::Vec2f& maxPoint)
    {
        int vecStart = 0;
        int vecEnd = v2f->size();
        if (drawArrayFirst >= 0)
        {
            vecStart = drawArrayFirst;
            vecEnd = drawArrayCount + vecStart;
        }
        maxPoint = osg::Vec2f(-1e38, -1e38);
        minPoint = osg::Vec2f(1e38, 1e38);
        unsigned int bufferStart = buffer->data.size();
        for (int vidx = vecStart; vidx < vecEnd; vidx++)
        {
            osg::Vec2f point = v2f->at(vidx);
            buffer->append(point.x());
            buffer->append(point.y());

            expandBBox2d(minPoint, maxPoint, point);
        }
        buffer->alignment();

        tinygltf::Accessor acc;
        acc.bufferView = model->bufferViews.size();
        acc.count = vecEnd - vecStart;
        acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        acc.type = TINYGLTF_TYPE_VEC2;
        acc.maxValues = { maxPoint.x(), maxPoint.y() };
        acc.minValues = { minPoint.x(), minPoint.y() };
        model->accessors.push_back(acc);

        tinygltf::BufferView bfv;
        bfv.buffer = 0;
        bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        bfv.byteOffset = bufferStart;
        bfv.byteLength = buffer->data.size() - bufferStart;
        model->bufferViews.push_back(bfv);
    }

    void OSGBuildState::appendVec3Array(const osg::Vec3Array* v3f, osg::Vec3f& minPoint, osg::Vec3f& maxPoint)
    {
        unsigned int vecStart = 0;
        unsigned int vecEnd = v3f->size();
        if (drawArrayFirst >= 0)
        {
            vecStart = drawArrayFirst;
            vecEnd = drawArrayCount + vecStart;
        }

        maxPoint = osg::Vec3f(-1e38, -1e38, -1e38);
        minPoint = osg::Vec3f(1e38, 1e38, 1e38);

        unsigned int bufferStart = buffer->data.size();
        for (unsigned int vidx = vecStart; vidx < vecEnd; vidx++)
        {
            osg::Vec3f point = v3f->at(vidx);
            buffer->append(point.x());
            buffer->append(point.y());
            buffer->append(point.z());

            expandBBox3d(minPoint, maxPoint, point);
        }
        buffer->alignment();

        tinygltf::Accessor acc;
        acc.bufferView = model->bufferViews.size();
        acc.count = vecEnd - vecStart;
        acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        acc.type = TINYGLTF_TYPE_VEC3;
        acc.maxValues = { maxPoint.x(), maxPoint.y(), maxPoint.z() };
        acc.minValues = { minPoint.x(), minPoint.y(), minPoint.z() };
        model->accessors.push_back(acc);

        tinygltf::BufferView bfv;
        bfv.buffer = 0;
        bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        bfv.byteOffset = bufferStart;
        bfv.byteLength = buffer->data.size() - bufferStart;
        model->bufferViews.push_back(bfv);
    }


    bool OSGBuildState::appendPrimitive(const osg::Geometry* g, const osg::PrimitiveSet* ps, OSGPrimitiveState& pmtState)
    {
        tinygltf::Primitive primits;
        // indecis
        primits.indices = model->accessors.size();
        // reset draw_array state
        drawArrayFirst = -1;
        osg::PrimitiveSet::Type t = ps->getType();
        switch (t)
        {
            case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                const osg::DrawElementsUByte* drawElements = dynamic_cast<const osg::DrawElementsUByte*>(ps);
                appendOSGIndex(drawElements, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
                break;
            }
            case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                const osg::DrawElementsUShort* drawElements = dynamic_cast<const osg::DrawElementsUShort*>(ps);
                appendOSGIndex(drawElements, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                break;
            }
            case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
            {
                const osg::DrawElementsUInt* drawElements = dynamic_cast<const osg::DrawElementsUInt*>(ps);
                appendOSGIndex(drawElements, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);
                break;
            }
            case osg::PrimitiveSet::DrawArraysPrimitiveType:
            {
                primits.indices = -1;
                const osg::DrawArrays* da = dynamic_cast<const osg::DrawArrays*>(ps);
                GLenum mode = da->getMode();
                if(mode != GL_TRIANGLES){
                    qCritical("GLenum is not GL_TRIANGLES in osgb\n");
                    return false;
                }
                drawArrayFirst = da->getFirst();
                drawArrayCount = da->getCount();
                break;
            }
            default:
            {
                qCritical("Unsupport osg::PrimitiveSet::Type [%d]", t);
                return false;
            }
        }
        // vertex: full vertex and part indecis
        if (pmtState.vertexAccessor > -1 && drawArrayFirst == -1)
        {
            primits.attributes["POSITION"] = pmtState.vertexAccessor;
        }
        else
        {
            osg::Vec3f maxPoint(-1e38, -1e38, -1e38);
            osg::Vec3f minPoint(1e38, 1e38, 1e38);
            osg::Vec3Array* vertexArr = (osg::Vec3Array*)g->getVertexArray();
            primits.attributes["POSITION"] = model->accessors.size();
            // reuse vertex accessor if multi indecis
            if (pmtState.vertexAccessor == -1 && drawArrayFirst == -1)
            {
                pmtState.vertexAccessor = model->accessors.size();
            }
            appendVec3Array(vertexArr, minPoint, maxPoint);
            // merge mesh bbox
            expandBBox3d(this->pointMin, this->pointMax, maxPoint);
            expandBBox3d(this->pointMin, this->pointMax, minPoint);
        }
        // normal
        osg::Vec3Array* normalArr = (osg::Vec3Array*)g->getNormalArray();
        if (normalArr != nullptr)
        {
            if (pmtState.normalAccessor > -1 && drawArrayFirst == -1)
            {
                primits.attributes["NORMAL"] = pmtState.normalAccessor;
            }
            else
            {
                osg::Vec3f maxPoint(-1e38, -1e38, -1e38);
                osg::Vec3f minPoint(1e38, 1e38, 1e38);
                primits.attributes["NORMAL"] = model->accessors.size();
                // reuse vertex accessor if multi indecis
                if (pmtState.normalAccessor == -1 && drawArrayFirst == -1)
                {
                    pmtState.normalAccessor = model->accessors.size();
                }
                appendVec3Array(normalArr, minPoint, maxPoint);
            }
        }
        // textcoord
        osg::Vec2Array* texArr = (osg::Vec2Array*)g->getTexCoordArray(0);
        if (texArr != nullptr)
        {
            if (pmtState.textcdAccessor > -1 && drawArrayFirst == -1)
            {
                primits.attributes["TEXCOORD_0"] = pmtState.textcdAccessor;
            }
            else
            {
                primits.attributes["TEXCOORD_0"] = model->accessors.size();
                // reuse textcoord accessor if multi indecis
                if (pmtState.textcdAccessor == -1 && drawArrayFirst == -1)
                {
                    pmtState.textcdAccessor = model->accessors.size();
                }
                osg::Vec2f maxPoint, minPoint;
                appendVec2Array(texArr, minPoint, maxPoint);
            }
        }
        // material
        primits.material = -1;

        switch (ps->getMode())
        {
            case GL_TRIANGLES:
                primits.mode = TINYGLTF_MODE_TRIANGLES;
                break;
            case GL_TRIANGLE_STRIP:
                primits.mode = TINYGLTF_MODE_TRIANGLE_STRIP;
                break;
            case GL_TRIANGLE_FAN:
                primits.mode = TINYGLTF_MODE_TRIANGLE_FAN;
                break;
            default:
                qWarning("Unsupport Primitive Mode: [%d]", ps->getMode());
                return false;
        }
        model->meshes.back().primitives.push_back(primits);
        return true;
    }

}
