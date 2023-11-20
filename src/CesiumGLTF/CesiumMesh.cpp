#include <CesiumGLTF/CesiumMesh.h>

#include <algorithm>

namespace scially {
    CesiumMesh::VertexData&
		CesiumMesh::findVertexDataByName(const QString& name)
	{
		auto findIter = std::find_if(vertexDatas.begin(), vertexDatas.end(),
            [&name](const VertexData& item) {
                return item.name == name;
		});

		if (findIter == vertexDatas.end()) {
            VertexData v;
            v.name = name;
            v.values.resize(vertex.size());

			vertexDatas.append(v);
            return vertexDatas.last();
		}
		else {
			return *findIter;
		}
	}

	void CesiumMesh::addMesh(const CesiumMesh& mesh)
	{
		const uint32_t indexOffset = vertex.size();

		for (const auto& vd : mesh.vertexDatas) {
            auto& v = findVertexDataByName(vd.name);
            v.values.append(vd.values);
		}

		// copy vertex
		vertex.append(mesh.vertex);

		// copy normals
		normals.append(mesh.normals);

		// copy texcoords
		texcoords.append(mesh.texcoords);

		// copy batchids
		batchIDs.append(mesh.batchIDs);

		// copy indice
		std::for_each(mesh.indices.begin(), mesh.indices.end(), [indexOffset, this](const auto& i) {
			indices.append(i + indexOffset);
		});
	}

	bool CesiumMesh::duplicateVertex(uint32_t idx)
	{
		if (idx >= vertex.size())
			return false;

		vertex.append(vertex[idx]);
		normals.append(normals[idx]);
		texcoords.append(texcoords[idx]);

		for (auto& vd : vertexDatas) {
            vd.values.append(vd.values[idx]);
		}

		batchIDs.append(batchIDs[idx]);

		return true;
	}

	osg::Vec3f CesiumMesh::toWorldNormal(const osg::Vec3f& v) const
	{
		return tileCoordinateSystemToWorld.preMult(v);
	}

	
	osg::Vec3d CesiumMesh::toWorldVertex(const osg::Vec3f& v) const
	{
		return tileCoordinateSystemToWorld.preMult(v);
	}
	
	void CesiumMesh::nanToZero()
	{
		for (auto& v : vertex) {
			if (isnan(v.x()) || isinf(v.x())) {
				v.x() = 0;
			}
			if (isnan(v.y()) || isinf(v.x())) {
				v.y() = 0;
			}
			if (isnan(v.z()) || isinf(v.x())) {
				v.z() = 0;
			}
		}

		for (auto& v : normals) {
			if (isnan(v.x()) || isinf(v.x())) {
				v.x() = 0;
			}
			if (isnan(v.y()) || isinf(v.x())) {
				v.y() = 0;
			}
			if (isnan(v.z()) || isinf(v.x())) {
				v.z() = 0;
			}
		}

		for (auto& v : texcoords) {
			if (isnan(v.x()) || isinf(v.x())) {
				v.x() = 0;
			}
			if (isnan(v.y()) || isinf(v.x())) {
				v.y() = 0;
			}
		}
	}

	osg::BoundingBoxd CesiumMesh::boundingBox() const
	{
		osg::BoundingBoxd box;

        foreach (const auto& v, vertex) {
			box.expandBy(v);
		}

		return box;
	}
}
