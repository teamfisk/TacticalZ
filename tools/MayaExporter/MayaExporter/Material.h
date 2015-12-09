#ifndef Material_Material_h__
#define Material_Material_h__

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include "MayaIncludes.h"

struct MaterialNode
{
	std::string Name;
	std::array<float, 4> Color;
	std::array<float, 3> Incandescence;
	std::array<float, 3> Specular;
	float ReflectionFactor;
	float SpecularExponent;
	std::string ColorMapFile;
	std::string SpecularMapFile;
	std::string NormalMapFile;
	std::string IncandescenceMapFile;
};

class Material
{
public:
	Material() {};
	~Material() {};
	std::vector<MaterialNode>* DoIt();
	std::vector<std::string>* TexturePaths();
private:
	MPlug m_Plug;
	
	std::vector<MaterialNode> m_AllMaterials;
	std::vector<std::string> m_TexturePaths;

	bool findColorTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findNormalTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findSpecularTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findIncandescenceTexture(MaterialNode& material_node, MFnDependencyNode& node);
	void grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabPhongProperties(MaterialNode& material_node, MFnDependencyNode& node);
};

#endif // Material_Material_h__