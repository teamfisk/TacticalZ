#ifndef Mesh_Mesh_h__
#define Mesh_Mesh_h__

#include <map>
#include <vector>

#include "MayaIncludes.h"

struct VertexLayout
{
	float Pos[3];
	float Normal[3];
	float Uv[2];
};

class Mesh
{
public:
	Mesh();
	void GetMeshData(MObject Object);
	~Mesh();
};

#endif