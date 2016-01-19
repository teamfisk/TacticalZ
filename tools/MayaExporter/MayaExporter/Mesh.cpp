#pragma once
#include "Mesh.h"

using namespace std;

MeshClass::MeshClass()
{

}

std::map<int, MeshClass::WeightInfo> MeshClass::GetWeightData()
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

Mesh MeshClass::GetMeshData(MObjectArray object)
{
    Mesh newMesh;
    vector<VertexLayout>& vertexList = newMesh.Vertices;
    map<string, vector<int>>& indexLists = newMesh.Indices;
    for (int ObjectID = 0; ObjectID < object.length(); ObjectID++) {
        if (!object[ObjectID].hasFn(MFn::kMesh))
            continue;

        // In here, we retrieve triangulated polygons from the mesh
        MFnMesh mesh(object[ObjectID]);
        MDagPathArray dagPaths;
        MDagPath::getAllPathsTo(object[ObjectID], dagPaths);
        for (int pathID = 0; pathID < dagPaths.length(); pathID++) {
            MGlobal::displayInfo(dagPaths[pathID].fullPathName());
            MDagPath thisMeshPath(dagPaths[pathID]);
            MMatrix transformMatrix = thisMeshPath.inclusiveMatrix();

            map<unsigned int, vector<unsigned int>> vertexToIndex;;

            MIntArray intdexOffsetVertexCount, vertices, triangleList;
            MPointArray dummy;
            unsigned int vertexIndex;
            MVector normal;
            MPoint pos;
            float2 UV;
            double biTangent[3];
            double biNormal[3];
            MFloatVectorArray Tangents;
            MFloatVectorArray biNormals;

            MObjectArray shaderList;
            MIntArray shaderIndexList;
            mesh.getConnectedShaders(0, shaderList, shaderIndexList);

            map<string, vector<int>> materialFaceIDs;
            MGlobal::displayInfo(MString() + "shaderIndexList: " + shaderIndexList.length());
            MGlobal::displayInfo(MString() + "shaderList: " + shaderList.length());
            MPlugArray plugArray;
            for (int i = 0; i < shaderIndexList.length(); i++) {
                MFnDependencyNode shader(shaderList[shaderIndexList[i]]);
                MPlug p_Plug = shader.findPlug("surfaceShader");
                if (p_Plug.connectedTo(plugArray, true, false)) {
                    MFnDependencyNode node = plugArray[0].node();
                    materialFaceIDs[node.name().asChar()].push_back(i);
                }
            }

            map<int, WeightInfo> vertexWeights = GetWeightData();

            mesh.getTangents(Tangents, MSpace::kTransform, NULL);
            mesh.getBinormals(biNormals, MSpace::kTransform, NULL);

            MItMeshFaceVertex faceVert(object[ObjectID]);

            int intDummy = 0;

            MItMeshPolygon meshPolyIter(object[ObjectID]);
            MFloatPointArray positions;

            mesh.getPoints(positions);

            for (auto aMaterial : materialFaceIDs) {
                for (auto faceID : aMaterial.second) {

                    vector<array<unsigned int, 2>> localVertexToGlobalIndex;
                    meshPolyIter.setIndex(faceID, intDummy);

                    meshPolyIter.getVertices(vertices);
                    meshPolyIter.getTriangles(dummy, triangleList);
                    //MGlobal::displayInfo("Befor Second Loop");
                    for (unsigned int i = 0; i < vertices.length(); i++) {
                        VertexLayout thisVertex;
                        vertexIndex = meshPolyIter.vertexIndex(i);
                        faceVert.setIndex(meshPolyIter.index(), i, intDummy, intDummy);
                        //MGlobal::displayInfo("In Second Loop");
                        //pos = faceVert.position(MSpace::kTransform);
                        //mesh.getPoint(vertexIndex, pos, MSpace::kPostTransform);
                        pos = positions[vertexIndex];
                        pos = pos * transformMatrix;
                        if (abs(pos.x) > 0.0001)
                            thisVertex.Pos[0] = pos.x;
                        if (abs(pos.y) > 0.0001)
                            thisVertex.Pos[1] = pos.y;
                        if (abs(pos.z) > 0.0001)
                            thisVertex.Pos[2] = pos.z;

                        faceVert.getNormal(normal, MSpace::kTransform);
                        if (abs(normal[0]) > 0.0001)
                            thisVertex.Normal[0] = normal[0];
                        if (abs(normal[1]) > 0.0001)
                            thisVertex.Normal[1] = normal[1];
                        if (abs(normal[2]) > 0.0001)
                            thisVertex.Normal[2] = normal[2];

                        MFloatVector Tangent = Tangents[faceVert.tangentId()];
                        //MVector tmp = faceVert.getTangent(MSpace::kObject, NULL);
                        //tmp.get(biTangent);
                        if (abs(Tangent[0]) > 0.0001)
                            thisVertex.Tangent[0] = Tangent[0];
                        if (abs(Tangent[1]) > 0.0001)
                            thisVertex.Tangent[1] = Tangent[1];
                        if (abs(Tangent[2]) > 0.0001)
                            thisVertex.Tangent[2] = Tangent[2];

                        MFloatVector biNormal = biNormals[faceVert.tangentId()];
                        //faceVert.getBinormal().get(biNormal);
                        if (abs(biNormal[0]) > 0.0001)
                            thisVertex.BiNormal[0] = biNormal[0];
                        if (abs(biNormal[1]) > 0.0001)
                            thisVertex.BiNormal[1] = biNormal[1];
                        if (abs(biNormal[2]) > 0.0001)
                            thisVertex.BiNormal[2] = biNormal[2];

                        faceVert.getUV(UV);
                        thisVertex.Uv[0] = UV[0];
                        thisVertex.Uv[1] = UV[1];

                        thisVertex.BoneIndices[0] = vertexWeights[faceVert.vertId()].BoneIndices[0];
                        thisVertex.BoneIndices[1] = vertexWeights[faceVert.vertId()].BoneIndices[1];
                        thisVertex.BoneIndices[2] = vertexWeights[faceVert.vertId()].BoneIndices[2];
                        thisVertex.BoneIndices[3] = vertexWeights[faceVert.vertId()].BoneIndices[3];

                        if (abs(vertexWeights[faceVert.vertId()].BoneWeights[0]) > 0.0001)
                            thisVertex.BoneWeights[0] = vertexWeights[faceVert.vertId()].BoneWeights[0];
                        if (abs(vertexWeights[faceVert.vertId()].BoneWeights[1]) > 0.0001)
                            thisVertex.BoneWeights[1] = vertexWeights[faceVert.vertId()].BoneWeights[1];
                        if (abs(vertexWeights[faceVert.vertId()].BoneWeights[2]) > 0.0001)
                            thisVertex.BoneWeights[2] = vertexWeights[faceVert.vertId()].BoneWeights[2];
                        if (abs(vertexWeights[faceVert.vertId()].BoneWeights[3]) > 0.0001)
                            thisVertex.BoneWeights[3] = vertexWeights[faceVert.vertId()].BoneWeights[3];

                        float totalWeight = thisVertex.BoneWeights[0] + thisVertex.BoneWeights[1] + thisVertex.BoneWeights[2] + thisVertex.BoneWeights[3];
                        if (totalWeight < 1.00f && totalWeight > 0.01f) {
                            thisVertex.BoneWeights[0] /= totalWeight;
                            thisVertex.BoneWeights[1] /= totalWeight;
                            thisVertex.BoneWeights[2] /= totalWeight;
                            thisVertex.BoneWeights[3] /= totalWeight;
                        }

                        std::vector<VertexLayout>::iterator it = std::find(vertexList.begin(), vertexList.end(), thisVertex);
                        array<unsigned int, 2> tmp;
                        if (it != vertexList.end()) {
                            tmp[0] = vertexIndex;
                            tmp[1] = it - vertexList.begin();
                            localVertexToGlobalIndex.push_back(tmp);
                        } else {
                            tmp[0] = vertexIndex;
                            tmp[1] = vertexList.size();
                            localVertexToGlobalIndex.push_back(tmp);
                            vertexList.push_back(thisVertex);
                        }
                        //MGlobal::displayInfo(MString() + "localVertexToGlobalIndex[localVertexToGlobalIndex.size()-1]: " + localVertexToGlobalIndex[localVertexToGlobalIndex.size()-1][0] + " " + localVertexToGlobalIndex[localVertexToGlobalIndex.size()-1][1]);
                        //cout << "Pos: " << thisVertex.Pos[0] << "/" << thisVertex.Pos[1] << "/" << thisVertex.Pos[2] << endl;
                        //cout << "Normals: " << thisVertex.Normal[0] << "/" << thisVertex.Normal[1] << "/" << thisVertex.Normal[2] << endl;
                        //cout << "Bi-Normals: " << thisVertex.BiNormal[0] << "/" << thisVertex.BiNormal[1] << "/" << thisVertex.BiNormal[2] << endl;
                        //cout << "Bi-Tangents: " << thisVertex.BiTangent[0] << "/" << thisVertex.BiTangent[1] << "/" << thisVertex.BiTangent[2] << endl;
                        //cout << "UV: " << thisVertex.Uv[0] << "/" << thisVertex.Uv[1] << endl;
                    }
                    for (unsigned int i = 0; i < triangleList.length(); i++) {
                        unsigned int  k = 0;
                        if (localVertexToGlobalIndex.size() > 0) {
                            //MGlobal::displayInfo(MString() + "triangleList[i] : " + triangleList[i]);
                            while (localVertexToGlobalIndex[k][0] != triangleList[i] && k < localVertexToGlobalIndex.size()) {
                                k++;
                            }
                            //MGlobal::displayInfo(MString() + "localVertexToGlobalIndex[k] : " + localVertexToGlobalIndex[k][0] + " " + localVertexToGlobalIndex[k][1]);
                            indexLists[aMaterial.first.c_str()].push_back(localVertexToGlobalIndex[k][1]);
                        }
                    }
                }
                //  MGlobal::displayInfo( MString() + "localVertexToGlobalIndex.size(): " + localVertexToGlobalIndex.size());
                //    if (localVertexToGlobalIndex.size() > 0) {
                //        MGlobal::displayInfo(MString() + "triangleList.length(): " + triangleList.length());
                //        for (unsigned int i = triangleList.length() - 1; i >= 0; i--) {
                //            MGlobal::displayInfo(MString() + "i: " + i);
                //            unsigned int  k = localVertexToGlobalIndex.size() - 1;
                //            MGlobal::displayInfo(MString() + "triangleList[i] : " + triangleList[i]);
                //            while (localVertexToGlobalIndex[k] != triangleList[i] && k >= 0) {
                //                MGlobal::displayInfo(MString() + "k: " + k);
                //                k--;
                //            }
                //            MGlobal::displayInfo(MString() + "localVertexToGlobalIndex[k] : " + localVertexToGlobalIndex[k]);
                //            indexList.push_back(indexOffset + k);
                //        }
                //    }
            }
        }
    }

    int totalIndices = 0;
    for (auto aList : newMesh.Indices) {
        totalIndices += aList.second.size();
    }
    newMesh.NumIndices = totalIndices;
    newMesh.NumVertices = newMesh.Vertices.size();

    return newMesh;
}

MeshClass::~MeshClass()
{

}