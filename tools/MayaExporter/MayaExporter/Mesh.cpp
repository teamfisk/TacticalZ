#pragma once
#include "Mesh.h"

using namespace std;

Mesh::Mesh()
{

}

void Mesh::GetMeshData(MObject object)
{
	// In here, we retrieve triangulated polygons from the mesh
	MFnMesh mesh(object);

	map<UINT, vector<UINT>> vertexToIndex;

	vector<VertexLayout> verticesData;
	vector<UINT>indexArray;

	MIntArray intdexOffsetVertexCount, vertices, triangleList;
	MPointArray dummy;

	UINT vertexIndex;
	MVector normal;
	MPoint pos;
	float2 UV;
	VertexLayout thisVertex;

	for (MItMeshPolygon meshPolyIter(object); !meshPolyIter.isDone(); meshPolyIter.next()) {
		vector<UINT> localVertexToGlobalIndex;
		meshPolyIter.getVertices(vertices);

		meshPolyIter.getTriangles(dummy, triangleList);
		UINT indexOffset = verticesData.size();

		for (UINT i = 0; i < vertices.length(); i++) {
			vertexIndex = meshPolyIter.vertexIndex(i);
			pos = meshPolyIter.point(i);
			pos.get(thisVertex.Pos);

			meshPolyIter.getNormal(i, normal);
			thisVertex.Normal[0] = normal[0];
			thisVertex.Normal[1] = normal[1];
			thisVertex.Normal[2] = normal[2];

			meshPolyIter.getUV(i, UV);
			thisVertex.Uv[0] = UV[0];
			thisVertex.Uv[1] = UV[1];

			verticesData.push_back(thisVertex);
			localVertexToGlobalIndex.push_back(vertexIndex);

			cout << "Pos: " << thisVertex.Pos[0] << "/" << thisVertex.Pos[1] << "/" << thisVertex.Pos[2] << endl;
			cout << "Normals: " << thisVertex.Normal[0] << "/" << thisVertex.Normal[1] << "/" << thisVertex.Normal[2] << endl;
			cout << "UV: " << thisVertex.Uv[0] << "/" << thisVertex.Uv[1] << endl;
		}
		for (UINT i = 0; i < triangleList.length(); i++) {
			UINT k = 0;
			while (localVertexToGlobalIndex[k] != triangleList[i])
				k++;
			indexArray.push_back(indexOffset + k);
		}
	}
}

Mesh::~Mesh()
{

}