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

	virtual void WriteBinary(std::ostream& out) {

	}
	virtual void WriteASCII(std::ostream& out) const {

	}
};

class Mesh
{
public:
	Mesh();
	void GetMeshData(MObject Object);
	~Mesh();
};

#endif