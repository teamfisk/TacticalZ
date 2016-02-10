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

	enum class MaterialType { Basic = 1, SplatMapping, SingleTextures };

	MaterialType type = MaterialType::Basic;

	std::string Name;

	float ReflectionFactor;
    float SpecularExponent;

	float DiffuseColor[3]{ 1.0f, 1.0f, 1.0f };
	float SpecularColor[3]{ 1.0f, 1.0f, 1.0f };
	float IncandescenceColor[3]{ 1.0f, 1.0f, 1.0f };

	unsigned int IndexStart;
	unsigned int IndexEnd;

	unsigned char NumColorMaps = 0;
	unsigned char NumSpecularMaps = 0;
	unsigned char NumNormalMaps = 0;
	unsigned char NumIncandescenceMaps = 0;
	Texture SplatMap;
	std::vector<Texture> ColorMaps;
	std::vector<Texture> SpecularMaps;	
	std::vector<Texture> NormalMaps;
	std::vector<Texture> IncandescenceMaps;

    virtual void WriteBinary(std::ostream& out)
    {
		
		out.write((char*)&type, sizeof(MaterialType));
        out.write((char*)&SpecularExponent, sizeof(float));
        out.write((char*)&ReflectionFactor, sizeof(float));

        out.write((char*)&DiffuseColor, sizeof(float) * 3);
        out.write((char*)&SpecularColor, sizeof(float) * 3);
        out.write((char*)&IncandescenceColor, sizeof(float) * 3);

        out.write((char*)&IndexStart, sizeof(unsigned int));
        out.write((char*)&IndexEnd, sizeof(unsigned int));
		if (type != MaterialType::Basic) {
			if (type == MaterialType::SplatMapping) {
				SplatMap.WriteBinary(out);
			}
			out.write((char*)&NumColorMaps, sizeof(unsigned char));
			out.write((char*)&NumSpecularMaps, sizeof(unsigned char));
			out.write((char*)&NumNormalMaps, sizeof(unsigned char));
			out.write((char*)&NumIncandescenceMaps, sizeof(unsigned char));
		}

		if (type != MaterialType::Basic) {
			for (auto aTexture : ColorMaps) {
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
    }

    virtual void WriteASCII(std::ostream& out) const
    {
        out << "New Material _ not in binary" << endl;

		out << "MaterialType(enum): ";
		switch (type) {
		case MaterialType::Basic:
			out << "Basic";
			break;
		case MaterialType::SplatMapping:
			out << "SplatMapping";
			break;
		case MaterialType::SingleTextures:
			out << "SingleTextures";
			break;
		};

		out << endl;

        out << "Material Name: " << Name << " _ not in binary" << endl;

        out << "SpecularExponent: " << SpecularExponent << endl;
        out << "ReflectionFactor: " << ReflectionFactor << endl;
        out << "DiffuseColor: " << DiffuseColor[0] << " "  << DiffuseColor[1] << " "  << DiffuseColor[2] << endl;
        out << "SpecularColor: " << SpecularColor[0] << " "  << SpecularColor[1] << " "  << SpecularColor[2] << endl;
        out << "IncandescenceColor: " << IncandescenceColor[0] << " "  << IncandescenceColor[1] << " "  << IncandescenceColor[2] << endl;
        out << "IndexStart: " << IndexStart << endl;
        out << "IndexEnd: " << IndexEnd << endl;

		switch (type) {
		case MaterialType::SplatMapping:
			out << "SplatMap _ not in binary " << endl;
			SplatMap.WriteASCII(out);
			//Intended fall trought
		case MaterialType::SingleTextures:
			out << "NumColormaps (is unsigned char in Binary): " << ((unsigned int)NumColorMaps) << endl;
			out << "NumSpecularMap (is unsigned char in Binary): " << ((unsigned int)NumSpecularMaps) << endl;
			out << "NumNormalMap (is unsigned char in Binary): " << ((unsigned int)NumNormalMaps) << endl;
			out << "NumIncandescenceMap (is unsigned char in Binary): " << ((unsigned int)NumIncandescenceMaps )<< endl;

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
			break;
		};
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
	bool findSplatTextures(MaterialNode& material_node, std::vector<MaterialNode::Texture>& textureVector, MFnDependencyNode& node);
	void grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabPhongProperties(MaterialNode& material_node, MFnDependencyNode& node);
};

#endif // Material_Material_h__