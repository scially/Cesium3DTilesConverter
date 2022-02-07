#include <GeometryMesh.h>
#include <CoordinateConvert.h>
#include <earcut.hpp>

#include <array>
#include <osg/Vec2d>
#include <osg/Math>
namespace gzpi {

    void GeometryMesh::init(double centerX, double centerY, double height, QSharedPointer<OGRPolygon> polygon)
    {
        OGRLinearRing* pRing = polygon->getExteriorRing();
        int ptNum = pRing->getNumPoints();
        Q_ASSERT(ptNum >= 4);

        int ptCount = 0;
        for (int i = 0; i < ptNum; i++) {
            OGRPoint pt;
            pRing->getPoint(i, &pt);
            double bottom = pt.getZ();
            float pointX = (float)Math::lonToMeter(osg::DegreesToRadians(pt.getX() - centerX), osg::DegreesToRadians(centerY));
            float pointY = (float)Math::latToMeter(osg::DegreesToRadians(pt.getY() - centerY));
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
                index.append({ 2.0f * i, 2.0f * i + 1, 2.0f * (i + 1) + 1 });
                index.append({ 2.0f * (i + 1), 2.0f * i, 2.0f * (i + 1) + 1 });
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
                float pointX = (float)Math::lonToMeter(osg::DegreesToRadians(pt.getX() - centerX), osg::DegreesToRadians(centerY));
                float pointY = (float)Math::latToMeter(osg::DegreesToRadians(pt.getY() - centerX));
                vertex.push_back({ pointX , pointY, (float)bottom });
                vertex.push_back({ pointX , pointY, (float)height });
                // double vertex
                if (i != 0 && i != ptNum - 1) {
                    vertex.push_back({ pointX , pointY, (float)bottom });
                    vertex.push_back({ pointX , pointY, (float)height });
                }
            }
            vertexNum = vertex.size() / 2 - ptCount;
            for (int i = 0; i < vertexNum; i += 2) {
                if (i != vertexNum - 1) {
                    index.push_back({ ptCount + 2.0f * i,       ptCount + 2.0f * i + 1, ptCount + 2.0f * (i + 1) });
                    index.push_back({ ptCount + 2.0f * (i + 1), ptCount + 2.0f * i,     ptCount + 2.0f * (i + 1) });
                }
            }
            calcNormal(ptCount, ptNum);
            ptCount = vertex.size();
        }
        // top and bottom
        {
            using Coord = double;
            using Point = std::array<Coord, 2>;
            std::vector<std::vector<Point>> points(1);
            {
                OGRLinearRing* pRing = polygon->getExteriorRing();
                int ptNum = pRing->getNumPoints();
                for (int i = 0; i < ptNum; i++)
                {
                    OGRPoint pt;
                    pRing->getPoint(i, &pt);
                    double bottom = pt.getZ();
                    float pointX = (float)Math::lonToMeter(osg::DegreesToRadians(pt.getX() - centerX), osg::DegreesToRadians(centerY));
                    float pointY = (float)Math::latToMeter(osg::DegreesToRadians(pt.getY() - centerY));
                    points[0].push_back({ pointX, pointY });
                    vertex.push_back({ pointX , pointY, (float)bottom });
                    vertex.push_back({ pointX , pointY, (float)height });
                    normal.push_back({ 0,0,-1 });
                    normal.push_back({ 0,0,1 });
                }
            }
            int innerCount = polygon->getNumInteriorRings();
            for (int j = 0; j < innerCount; j++)
            {
                points.resize(points.size() + 1);
                OGRLinearRing* pRing = polygon->getInteriorRing(j);
                int ptNum = pRing->getNumPoints();
                for (int i = 0; i < ptNum; i++)
                {
                    OGRPoint pt;
                    pRing->getPoint(i, &pt);
                    double bottom = pt.getZ();
                    float pointX = (float)Math::lonToMeter(osg::DegreesToRadians(pt.getX() - centerX), osg::DegreesToRadians(centerY));
                    float pointY = (float)Math::latToMeter(osg::DegreesToRadians(pt.getY() - centerY));
                    points[j].push_back({ pointX, pointY });
                    vertex.push_back({ pointX , pointY, (float)bottom });
                    vertex.push_back({ pointX , pointY, (float)height });
                    normal.push_back({ 0, 0, -1 });
                    normal.push_back({ 0, 0, 1 });
                }
            }
            std::vector<int> indices = mapbox::earcut<int>(points);
            for (int idx = 0; idx < indices.size(); idx += 3) {
                index.append({
                    ptCount + 2.0f * indices[idx],
                    ptCount + 2.0f * indices[idx + 2],
                    ptCount + 2.0f * indices[idx + 1] });
            }
            for (int idx = 0; idx < indices.size(); idx += 3) {
                index.append({
                    ptCount + 2.0f * indices[idx] + 1,
                    ptCount + 2.0f * indices[idx + 1] + 1,
                    ptCount + 2.0f * indices[idx + 2] + 1});
            }
        }
    }
}
