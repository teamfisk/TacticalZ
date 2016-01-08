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
	double biTangent[3];
	double biNormal[3];
	VertexLayout thisVertex;
	MFloatVectorArray biTangents;
	MFloatVectorArray biNormals;

	mesh.getTangents(biTangents, MSpace::kObject, NULL);
	mesh.getBinormals(biNormals, MSpace::kObject, NULL);

	MItMeshFaceVertex faceVert(object);
	int intDummy = 0;
	for (MItMeshPolygon meshPolyIter(object); !meshPolyIter.isDone(); meshPolyIter.next()) {
		vector<UINT> localVertexToGlobalIndex;
		meshPolyIter.getVertices(vertices);

		meshPolyIter.getTriangles(dummy, triangleList);
		UINT indexOffset = verticesData.size();
		//MGlobal::displayInfo("Befor Second Loop");
		for (UINT i = 0; i < vertices.length(); i++) {
			faceVert.setIndex(meshPolyIter.index(), i, intDummy, intDummy);
			//MGlobal::displayInfo("In Second Loop");
			faceVert.position().get(thisVertex.Pos);
			faceVert.getNormal(normal);
			thisVertex.Normal[0] = normal[0];
			thisVertex.Normal[1] = normal[1];
			thisVertex.Normal[2] = normal[2];

			MFloatVector biTangent = biTangents[faceVert.tangentId()];
			//MVector tmp = faceVert.getTangent(MSpace::kObject, NULL);
			//tmp.get(biTangent);
			thisVertex.BiTangent[0] = biTangent[0];
			thisVertex.BiTangent[1] = biTangent[1];
			thisVertex.BiTangent[2] = biTangent[2];

			MFloatVector biNormal = biNormals[faceVert.tangentId()];
			//faceVert.getBinormal().get(biNormal);
			thisVertex.BiNormal[0] = biNormal[0];
			thisVertex.BiNormal[1] = biNormal[1];
			thisVertex.BiNormal[2] = biNormal[2];

			faceVert.getUV(UV);
			thisVertex.Uv[0] = UV[0];
			thisVertex.Uv[1] = UV[1];

			verticesData.push_back(thisVertex);
			localVertexToGlobalIndex.push_back(vertexIndex);

			//cout << "Pos: " << thisVertex.Pos[0] << "/" << thisVertex.Pos[1] << "/" << thisVertex.Pos[2] << endl;
			//cout << "Normals: " << thisVertex.Normal[0] << "/" << thisVertex.Normal[1] << "/" << thisVertex.Normal[2] << endl;
			//cout << "Bi-Normals: " << thisVertex.BiNormal[0] << "/" << thisVertex.BiNormal[1] << "/" << thisVertex.BiNormal[2] << endl;
			//cout << "Bi-Tangents: " << thisVertex.BiTangent[0] << "/" << thisVertex.BiTangent[1] << "/" << thisVertex.BiTangent[2] << endl;
			//cout << "UV: " << thisVertex.Uv[0] << "/" << thisVertex.Uv[1] << endl;
		}
		//MGlobal::displayInfo("Befor Third Loop");

		//for (UINT i = 0; i < triangleList.length(); i++) {
		//	UINT k = 0;
		//	while (localVertexToGlobalIndex[k] != triangleList[i])
		//		k++;
		//	indexArray.push_back(indexOffset + k);
		//}
	}
}

Mesh::~Mesh()
{

}