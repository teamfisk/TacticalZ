#ifndef Material_Material_h__
#define Material_Material_h__

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <map>

#include "MayaIncludes.h"
#include "OutputData.h"
#include "Mesh.h"


//#define ColorMapSplat 1
//#define SpecularMapSplat 1 << 1
//#define NormalMapSplat  1 << 2
//#define IncandescenceMapSplat 1 << 3

class MaterialNode : public OutputData
{
public:
	class Texture : public OutputData {
	public:
		unsigned int FileNameLength = 0;
		std::string FileName;
		float UVTiling[2]{ 1.0f, 1.0f };

		virtual void WriteBinary(std::ostream& out)
		{
			out.write((char*)&FileNameLength, sizeof(unsigned int));
			out.write(FileName.c_str(), FileNameLength);
			out.write((char*)&UVTiling, sizeof(float) * 2);
		}

		virtual void WriteASCII(std::ostream& out) const
		{
			out << "FileNameLength: " << FileNameLength << endl;
			out << "FileName: " << FileName << endl;
			out << "UV tiling: " << UVTiling[0] << " " << UVTiling[1] << endl;
		}
	};

	std::string Name;

	float ReflectionFactor;
    float SpecularExponent;

	float DiffuseColor[3]{ 1.0f, 1.0f, 1.0f };
	float SpecularColor[3]{ 1.0f, 1.0f, 1.0f };
	float IncandescenceColor[3]{ 1.0f, 1.0f, 1.0f };

	unsigned int IndexStart;
	unsigned int IndexEnd;

	char NumColormaps = 0;
	char NumSpecularMap = 0;
	char NumNormalMap = 0;
	char NumIncandescenceMap = 0;

	std::vector<Texture> ColorMaps;
	std::vector<Texture> SpecularMaps;	
	std::vector<Texture> NormalMaps;
	std::vector<Texture> IncandescenceMaps;

    virtual void WriteBinary(std::ostream& out)
    {
        out.write((char*)&SpecularExponent, sizeof(float));
        out.write((char*)&ReflectionFactor, sizeof(float));

        out.write((char*)&DiffuseColor, sizeof(float) * 3);
        out.write((char*)&SpecularColor, sizeof(float) * 3);
        out.write((char*)&IncandescenceColor, sizeof(float) * 3);

        out.write((char*)&IndexStart, sizeof(unsigned int));
        out.write((char*)&IndexEnd, sizeof(unsigned int));

		out.write((char*)&NumColormaps, sizeof(char));
		out.write((char*)&NumSpecularMap, sizeof(char));
		out.write((char*)&NumNormalMap, sizeof(char));
		out.write((char*)&NumIncandescenceMap, sizeof(char));

		for(auto aTexture : ColorMaps) {
			aTexture.WriteBinary(out);
		}
		for (auto aTexture : SpecularMaps) {
			aTexture.WriteBinary(out);
		}
		for (auto aTexture : NormalMaps) {
			aTexture.WriteBinary(out);
		}
		for (auto aTexture : IncandescenceMaps) {
			aTexture.WriteBinary(out);
		}
    }

    virtual void WriteASCII(std::ostream& out) const
    {
        out << "New Material _ not in binary" << endl;
        out << "number of indices: " << Name << " _ not in binary" << endl;

        out << "SpecularExponent: " << SpecularExponent << endl;
        out << "ReflectionFactor: " << ReflectionFactor << endl;
        out << "DiffuseColor: " << DiffuseColor[0] << " "  << DiffuseColor[1] << " "  << DiffuseColor[2] << endl;
        out << "SpecularColor: " << SpecularColor[0] << " "  << SpecularColor[1] << " "  << SpecularColor[2] << endl;
        out << "IncandescenceColor: " << IncandescenceColor[0] << " "  << IncandescenceColor[1] << " "  << IncandescenceColor[2] << endl;
        out << "IndexStart: " << IndexStart << endl;
        out << "IndexEnd: " << IndexEnd << endl;

        if (NumColormaps > 0)
            out << "NumColormaps: " << NumColormaps << endl;

        if (NumSpecularMap > 0)
            out << "NumSpecularMap: " << NumSpecularMap << endl;
        
        if (NumNormalMap > 0)
            out << "NumNormalMap: " << NumNormalMap << endl;
     
        if (NumIncandescenceMap > 0)
            out << "NumIncandescenceMap: " << NumIncandescenceMap << endl;

		out << "ColorMaps _ not in binary " << endl;
		for (auto aTexture : ColorMaps) {
			aTexture.WriteASCII(out);
		}

		out << "SpecularMaps _ not in binary " << endl;
		for (auto aTexture : SpecularMaps) {
			aTexture.WriteASCII(out);
		}

		out << "NormalMaps _ not in binary " << endl;
		for (auto aTexture : NormalMaps) {
			aTexture.WriteASCII(out);
		}

		out << "IncandescenceMaps _ not in binary " << endl;
		for (auto aTexture : IncandescenceMaps) {
			aTexture.WriteASCII(out);
		}
    }
};

class Material
{
public:
	Material() {};
	~Material() {};
	std::vector<MaterialNode>* DoIt(Mesh mesh);
	std::vector<std::string>* TexturePaths();
private:
	MPlug m_Plug;
	
	std::vector<MaterialNode> m_AllMaterials;
	std::vector<std::string> m_TexturePaths;

	bool findColorTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findNormalTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findSpecularTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findIncandescenceTexture(MaterialNode& material_node, MFnDependencyNode& node);
	bool findSplatTextures(std::vector<MaterialNode::Texture>& textureVector, MFnDependencyNode& node);
	void grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabPhongProperties(MaterialNode& material_node, MFnDependencyNode& node);
};

#endif // Material_Material_h__