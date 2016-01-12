#pragma once
#include "Mesh.h"

using namespace std;

Mesh::Mesh()
{

}
std::map<int, Mesh::WeightInfo> Mesh::GetWeightData()
{
    map<int, WeightInfo> weightMap;

	MItDependencyNodes it(MFn::kSkinClusterFilter);

	while (!it.isDone()) {

        MObject object = it.item();
        MFnSkinCluster skinCluster(object);
        MDagPathArray influences;

        unsigned int nrOfInfluences = skinCluster.influenceObjects(influences);

        unsigned int index;
        index = skinCluster.indexForOutputConnection(0);
        MDagPath skinPath;
        skinCluster.getPathAtIndex(index, skinPath);

        MItGeometry geomIter(skinPath);
        //for (unsigned int i = 0; i < nrOfInfluences; i++) {
        //    MGlobal::displayInfo(MString() + " Influence object name: " + influences[i].partialPathName().asChar());
        //}
        WeightInfo weightInfo;

        while (!geomIter.isDone()) {
            MObject comp = geomIter.component();
            MFloatArray weights;
            unsigned int influenceCount;
            skinCluster.getWeights(skinPath, comp, weights, influenceCount);
            MFnDependencyNode test(comp);

            unsigned int nrOfWeights = 0;

            for (unsigned int j = 0; j < weights.length() && nrOfWeights != 4; j++) {
                if (weights[j] > 0.00001) {
                    weightInfo.BoneWeights[nrOfWeights] = weights[j];
                    weightInfo.BoneIndices[nrOfWeights] = j;
                    nrOfWeights++;
                }
            }

            float totalWeight = 0.0f;
            for (unsigned int i = 0; i < 4; i++) {
                totalWeight += weightInfo.BoneWeights[i];
            }
            for (unsigned int i = 0; i < 4; i++) {
                weightInfo.BoneWeights[i] /= totalWeight;
            }
            weightMap[geomIter.index()] = weightInfo;


            for (unsigned int k = 0; k!=nrOfWeights; k++) {
                //MGlobal::displayInfo(MString() + "influence: " + weightInfo.BoneIndices[k] + " weight: " + weightInfo.BoneWeights[k]);
            }
            geomIter.next();


        }

        it.next();
	}
    return weightMap;
}

void Mesh::GetMeshData(MObject object, std::vector<VertexLayout>& vertexList, std::vector<unsigned int>& indexList)
{
	// In here, we retrieve triangulated polygons from the mesh
	MFnMesh mesh(object);
	map<unsigned int, vector<unsigned int>> vertexToIndex;;

	MIntArray intdexOffsetVertexCount, vertices, triangleList;
	MPointArray dummy;
    unsigned int vertexIndex;
	MVector normal;
	MPoint pos;
	float2 UV;
	double biTangent[3];
	double biNormal[3];
	VertexLayout thisVertex;
	MFloatVectorArray biTangents;
	MFloatVectorArray biNormals;

    std::map<int, WeightInfo> vertexWeights = GetWeightData();

	mesh.getTangents(biTangents, MSpace::kObject, NULL);
	mesh.getBinormals(biNormals, MSpace::kObject, NULL);

	MItMeshFaceVertex faceVert(object);

	int intDummy = 0;

    for (MItMeshPolygon meshPolyIter(object); !meshPolyIter.isDone(); meshPolyIter.next()) {

        vector<unsigned int> localVertexToGlobalIndex;
        unsigned int indexOffset = vertexList.size();

        meshPolyIter.getVertices(vertices);
        meshPolyIter.getTriangles(dummy, triangleList);
        MGlobal::displayInfo(MString() + "vertices.length(): " + vertices.length());
        //MGlobal::displayInfo("Befor Second Loop");
        for (unsigned int i = 0; i < vertices.length(); i++) {
            vertexIndex = meshPolyIter.vertexIndex(i);
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

            thisVertex.BoneIndices[0] = vertexWeights[faceVert.vertId()].BoneIndices[0];
            thisVertex.BoneIndices[1] = vertexWeights[faceVert.vertId()].BoneIndices[1];
            thisVertex.BoneIndices[2] = vertexWeights[faceVert.vertId()].BoneIndices[2];
            thisVertex.BoneIndices[3] = vertexWeights[faceVert.vertId()].BoneIndices[3];

            thisVertex.BoneWeights[0] = vertexWeights[faceVert.vertId()].BoneWeights[0];
            thisVertex.BoneWeights[1] = vertexWeights[faceVert.vertId()].BoneWeights[1];
            thisVertex.BoneWeights[2] = vertexWeights[faceVert.vertId()].BoneWeights[2];
            thisVertex.BoneWeights[3] = vertexWeights[faceVert.vertId()].BoneWeights[3];

            std::vector<VertexLayout>::iterator it = std::find(vertexList.begin(), vertexList.end(), thisVertex);
            
            if (it != vertexList.end()) {
                localVertexToGlobalIndex.push_back(vertexIndex);
            } else {
                localVertexToGlobalIndex.push_back(vertexIndex);
                vertexList.push_back(thisVertex);
            }

            //cout << "Pos: " << thisVertex.Pos[0] << "/" << thisVertex.Pos[1] << "/" << thisVertex.Pos[2] << endl;
            //cout << "Normals: " << thisVertex.Normal[0] << "/" << thisVertex.Normal[1] << "/" << thisVertex.Normal[2] << endl;
            //cout << "Bi-Normals: " << thisVertex.BiNormal[0] << "/" << thisVertex.BiNormal[1] << "/" << thisVertex.BiNormal[2] << endl;
            //cout << "Bi-Tangents: " << thisVertex.BiTangent[0] << "/" << thisVertex.BiTangent[1] << "/" << thisVertex.BiTangent[2] << endl;
            //cout << "UV: " << thisVertex.Uv[0] << "/" << thisVertex.Uv[1] << endl;
        }
        //MGlobal::displayInfo("Befor Third Loop");
        MGlobal::displayInfo(MString() + "localVertexToGlobalIndex.size(): " + localVertexToGlobalIndex.size());
        MGlobal::displayInfo(MString() + "vertexList.size(): " + vertexList.size());

        for (unsigned int i = 0; i < triangleList.length(); i++) {
            unsigned int  k = 0;
            MGlobal::displayInfo(MString() + "triangleList[i]: " + triangleList[i]);
            if (localVertexToGlobalIndex.size() > 0) {
                while (localVertexToGlobalIndex[k] != triangleList[i] && k < localVertexToGlobalIndex.size()) {
                    MGlobal::displayInfo(MString() + "localVertexToGlobalIndex[k]: " + localVertexToGlobalIndex[k]);
                    k++;
                }
                indexList.push_back(indexOffset + k);
            } 
        }
    }
}

Mesh::~Mesh()
{

}