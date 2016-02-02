#pragma once
#include "Mesh.h"

using namespace std;

MeshClass::MeshClass()
{

}

std::map<int, MeshClass::WeightInfo> MeshClass::GetWeightData()
{
    MS status;
    map<int, WeightInfo> weightMap;

	MItDependencyNodes it(MFn::kSkinClusterFilter);

	while (!it.isDone()) {

        MObject object = it.thisNode(&status);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + " it.thisNode() ERROR: " + status.errorString());
            break;
        }
        MFnSkinCluster skinCluster(object, &status);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + "skinCluster() ERROR: " + status.errorString());
            break;
        }
        MDagPathArray influences;

        unsigned int nrOfInfluences = skinCluster.influenceObjects(influences,&status);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + "skinCluster.influenceObjects() ERROR: " + status.errorString());
            break;
        }

        unsigned int index;
        index = skinCluster.indexForOutputConnection(0,&status);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + "skinCluster.indexForOutputConnection() ERROR: " + status.errorString());
            break;
        }
        MDagPath skinPath;
        status = skinCluster.getPathAtIndex(index, skinPath);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + "skinCluster.getPathAtIndex() ERROR: " + status.errorString());
            break;
        }

        MItGeometry geomIter(skinPath);
        //for (unsigned int i = 0; i < nrOfInfluences; i++) {
        //    MGlobal::displayInfo(MString() + " Influence object name: " + influences[i].partialPathName().asChar());
        //}
        WeightInfo weightInfo;

        while (!geomIter.isDone()) {
            MObject comp = geomIter.component(&status);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "geomIter.component() ERROR: " + status.errorString());
                break;
            }
            MFloatArray weights;
            unsigned int influenceCount;
            status = skinCluster.getWeights(skinPath, comp, weights, influenceCount);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "skinCluster.getWeights() ERROR: " + status.errorString());
                break;
            }
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

            geomIter.next();
        }
        it.next();
	}
    return weightMap;
}

Mesh MeshClass::GetMeshData(MObjectArray object)
{
    MS status;
    Mesh newMesh;
    vector<VertexLayout>& vertexList = newMesh.Vertices;
    map<string, vector<int>>& indexLists = newMesh.Indices;
    for (int ObjectID = 0; ObjectID < object.length(); ObjectID++) {
        if (!object[ObjectID].hasFn(MFn::kMesh))
            continue;

        MObject node = object[ObjectID];
        MFnDependencyNode thisNode(node);
        MPlugArray connections;
        thisNode.findPlug("inMesh").connectedTo(connections, true, true);
        MPlug weightList, weights;
        MObject weightListObject;
        for (unsigned int i = 0; i < connections.length(); i++) {
            if (connections[i].node().apiType() == MFn::kSkinClusterFilter) {
                MFnSkinCluster skinCluster(connections[i].node());
                weightList = skinCluster.findPlug("weightList", &status);
                weightListObject = weightList.attribute();
                weights = skinCluster.findPlug("weights");
				newMesh.hasSkin = true;
                break;
            }
        }


        // In here, we retrieve triangulated polygons from the mesh
        MFnMesh mesh(object[ObjectID]);
        MDagPathArray dagPaths;
        status = MDagPath::getAllPathsTo(object[ObjectID], dagPaths);
        if (status != MS::kSuccess) {
            MGlobal::displayError(MString() + "MDagPath::getAllPathsTo() ERROR: " + status.errorString());
            break;
        }

        for (int pathID = 0; pathID < dagPaths.length(); pathID++) {
            MDagPath thisMeshPath(dagPaths[pathID]);

            MMatrix transformMatrix = thisMeshPath.inclusiveMatrix(&status);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "thisMeshPath.inclusiveMatrix() ERROR: " + status.errorString() + " for " + thisMeshPath.fullPathName());
                break;
            }

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
            status = mesh.getConnectedShaders(0, shaderList, shaderIndexList);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "mesh.getConnectedShaders() ERROR: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                break;
            }

            if (shaderList.length() == 0) {
                MGlobal::displayError(MString() + "Object: \"" + thisMeshPath.fullPathName() + "\" have no material and will not be exported");
                break;
            }
            map<string, vector<int>> materialFaceIDs;

            MPlugArray plugArray;
            for (int i = 0; i < shaderIndexList.length(); i++) {
                MFnDependencyNode shader(shaderList[shaderIndexList[i]]);
                MPlug p_Plug = shader.findPlug("surfaceShader", status);
                if (status != MS::kSuccess) {
                    MGlobal::displayError(MString() + "shader.findPlug(\"surfaceShader\") ERROR: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                    continue;
                }
                if (p_Plug.connectedTo(plugArray, true, false, &status)) {
                    if (status != MS::kSuccess) {
                        MGlobal::displayError(MString() + "p_Plug.connectedTo() ERROR in if: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                        continue;
                    }
                    MFnDependencyNode node = plugArray[0].node();
                    materialFaceIDs[node.name().asChar()].push_back(i);
                }
                if (status != MS::kSuccess) {
                    MGlobal::displayError(MString() + "p_Plug.connectedTo() ERROR: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                    continue;
                }
            }

           // map<int, WeightInfo> vertexWeights = GetWeightData();
            status = mesh.getTangents(Tangents, MSpace::kObject);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "mesh.getTangents ERROR: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                continue;
            }
            status = mesh.getBinormals(biNormals, MSpace::kObject);
            if (status != MS::kSuccess) {
                MGlobal::displayError(MString() + "mesh.getBinormals ERROR: " + status.errorString()  + " for " + thisMeshPath.fullPathName());
                continue;
            }
            if(Tangents.length() == 0 || biNormals.length() == 0){
                MGlobal::displayError(MString() + "Unknown ERROR with " + thisMeshPath.fullPathName());
                continue;
            }
            MItMeshFaceVertex faceVert(object[ObjectID]);
            int intDummy = 0;

            MItMeshPolygon meshPolyIter(object[ObjectID]);
            MFloatPointArray positions;

            mesh.getPoints(positions);

            for (auto aMaterial : materialFaceIDs) {
                for (auto faceID : aMaterial.second) {
                    vector<array<unsigned int, 2>> localVertexToGlobalIndex;

                    status = meshPolyIter.setIndex(faceID, intDummy);
                    if (status != MS::kSuccess) {
                        MGlobal::displayError(MString() + " meshPolyIter.setIndex() ERROR: " + status.errorString() +  " for faceID " + faceID + " in mesh " + thisMeshPath.fullPathName());
                        break;
                    }

                    status = meshPolyIter.getVertices(vertices);
                    if (status != MS::kSuccess) {
                        MGlobal::displayError(MString() + "  meshPolyIter.getVertices() ERROR: " + status.errorString() +  " for faceID " + faceID + " in mesh " + thisMeshPath.fullPathName());
                        break;
                    }

                    status = meshPolyIter.getTriangles(dummy, triangleList);
                    if (status != MS::kSuccess) {
                        MGlobal::displayError(MString() + "  meshPolyIter.getTriangles() ERROR: " + status.errorString() +  " for faceID " + faceID + " in mesh " + thisMeshPath.fullPathName());
                        break;
                    }
                    //MGlobal::displayInfo("Befor Second Loop");
                    for (unsigned int i = 0; i < vertices.length(); i++) {
                        VertexLayout thisVertex;

                        vertexIndex = meshPolyIter.vertexIndex(i, &status);
                        if (status != MS::kSuccess) {
                            MGlobal::displayError(MString() + "  meshPolyIter.vertexIndex() ERROR: " + status.errorString() + "for local vertex " + i + " in " + faceID + " in mesh " + thisMeshPath.fullPathName());
                            break;
                        }

                        status = faceVert.setIndex(meshPolyIter.index(), i, intDummy, intDummy);
                        if (status != MS::kSuccess) {
                            MGlobal::displayError(MString() + "faceVert.setIndex() ERROR: " + status.errorString() + "for local vertex " + i + " in " + faceID + " in mesh " + thisMeshPath.fullPathName());
                            break;
                        }
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

                        status = faceVert.getNormal(normal, MSpace::kObject);
                        if (status != MS::kSuccess) {
                            MGlobal::displayError(MString() + "faceVert.getNormal() ERROR: " + status.errorString() + "for local vertex " + i + " in " + faceID + " in mesh " + thisMeshPath.fullPathName());
                            break;
                        }
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

                        status = faceVert.getUV(UV);
                        if (status != MS::kSuccess) {
                            MGlobal::displayError(MString() + " faceVert.getUV() ERROR: " + status.errorString() + "for local vertex " + i + " in " + faceID + " in mesh " + thisMeshPath.fullPathName());
                            break;
                        }
                        thisVertex.Uv[0] = UV[0];
                        thisVertex.Uv[1] = UV[1];

                        
                        if (newMesh.hasSkin) {
							thisVertex.useWeights = true;
                            float totalWeight = 0.0f;
                            unsigned int totalBones = 0;
                            MIntArray jointIDs /* ??? */;
                            weights.selectAncestorLogicalIndex(vertexIndex, weightListObject);
                            weights.getExistingArrayAttributeIndices(jointIDs);
                            for (unsigned int i = 0; i < jointIDs.length() && i < 4; i++) {
                                if (weights[i].asFloat() > 0.001f) {
                                    thisVertex.BoneIndices[totalBones] = jointIDs[i];
                                    thisVertex.BoneWeights[totalBones] = weights[i].asFloat();
                                    totalWeight = totalWeight + weights[i].asFloat();
                                    totalBones++;
                                }
                            }
                           
                            for (unsigned int i = 0; i < 4; i++) {
                                thisVertex.BoneWeights[i] = thisVertex.BoneWeights[i] / totalWeight;
                            }
						} else {
							thisVertex.useWeights = false;
						}

                        //float totalWeight = thisVertex.BoneWeights[0] + thisVertex.BoneWeights[1] + thisVertex.BoneWeights[2] + thisVertex.BoneWeights[3];
                        //if (totalWeight > 0.0001f) {
                        //    thisVertex.BoneWeights[0] /= totalWeight;
                        //    thisVertex.BoneWeights[1] /= totalWeight;
                        //    thisVertex.BoneWeights[2] /= totalWeight;
                        //    thisVertex.BoneWeights[3] /= totalWeight;
                        //}

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