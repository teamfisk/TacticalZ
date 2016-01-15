#ifndef Mesh_Mesh_h__
#define Mesh_Mesh_h__

#include <map>
#include <vector>

#include "OutputData.h"
#include "MayaIncludes.h"

class VertexLayout : public OutputData
{
public:

	float Pos[3];
	float Normal[3];
	float BiNormal[3];
	float BiTangent[3];
	float Uv[2];
	float BoneIndices[4];
	float BoneWeights[4];

    virtual void WriteBinary(std::ostream& out)
    {
        out.write((char*)&Pos, sizeof(float) * 3);
        out.write((char*)&Normal, sizeof(float) * 3);
        out.write((char*)&BiNormal, sizeof(float) * 3);
        out.write((char*)&BiTangent, sizeof(float) * 3);
        out.write((char*)&Uv, sizeof(float) * 2);
        out.write((char*)&BoneIndices, sizeof(float) * 4);
        out.write((char*)&BoneWeights, sizeof(float) * 4);
    }

    virtual void WriteASCII(std::ostream& out) const
    {
        out << "New Vertex: " << endl;
        out <<  Pos[0] << " " << Pos[1]  << " " << Pos[2] << endl;
        out <<  Normal[0] << " " << Normal[1]  << " " << Normal[2] << endl;
        out <<  BiNormal[0] << " " << BiNormal[1]  << " " << BiNormal[2] << endl;
        out <<  BiTangent[0] << " " << BiTangent[1]  << " " << BiTangent[2] << endl;
        out <<  Uv[0] << " " << Uv[1]  << endl;
        out <<  BoneIndices[0] << " " << BoneIndices[1]  << " " << BoneIndices[2] << " " << BoneIndices[3] << endl;
        out <<  BoneWeights[0] << " " << BoneWeights[1]  << " " << BoneWeights[2] << " " << BoneWeights[3] << endl;

    }
    bool operator==(const VertexLayout& right)
    {
        return
            this->Pos[0] == right.Pos[0] &&  this->Pos[1] == right.Pos[1] &&  this->Pos[2] == right.Pos[2] &&
            this->Normal[0] == right.Normal[0] &&  this->Normal[1] == right.Normal[1] &&  this->Normal[2] == right.Normal[2] &&
            this->BiNormal[0] == right.BiNormal[0] &&  this->BiNormal[1] == right.BiNormal[1] &&  this->BiNormal[2] == right.BiNormal[2] &&
            this->BiTangent[0] == right.BiTangent[0] &&  this->BiTangent[1] == right.BiTangent[1] &&  this->BiTangent[2] == right.BiTangent[2] &&
            this->Uv[0] == right.Uv[0] &&  this->Uv[1] == right.Uv[1] &&
            this->BoneIndices[0] == right.BoneIndices[0] &&  this->BoneIndices[1] == right.BoneIndices[1] &&  this->BoneIndices[2] == right.BoneIndices[2] && this->BoneIndices[3] == right.BoneIndices[3] &&
            this->BoneWeights[0] == right.BoneWeights[0] &&  this->BoneWeights[1] == right.BoneWeights[1] &&  this->BoneWeights[2] == right.BoneWeights[2] && this->BoneWeights[3] == right.BoneWeights[3]
            ;
    }
};




class Mesh
{
public:
	Mesh();
	void GetMeshData(MObject Object, std::vector<VertexLayout>& vertexList, std::vector<unsigned int>& indexList);
    ~Mesh();
private:
    struct WeightInfo {
        float BoneIndices[4] = { 0 };
        float BoneWeights[4] = { 0 };
    };
    std::map<int, WeightInfo>  GetWeightData();

};

#endif