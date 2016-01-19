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

class MaterialNode : public OutputData
{
public:
	std::string Name;

	float ReflectionFactor;
    float SpecularExponent;

    unsigned int ColorMapFileLength = 0;
	std::string ColorMapFile;

    unsigned int SpecularMapFileLength = 0;
	std::string SpecularMapFile;

    unsigned int NormalMapFileLength = 0;
	std::string NormalMapFile;

    unsigned int IncandescenceMapFileLength = 0;
	std::string IncandescenceMapFile;

    unsigned int IndexStart;
    unsigned int IndexEnd;

    virtual void WriteBinary(std::ostream& out)
    {
        out.write((char*)&ColorMapFileLength, sizeof(unsigned int));
        out.write((char*)&NormalMapFileLength, sizeof(unsigned int));
        out.write((char*)&SpecularMapFileLength, sizeof(unsigned int));
        out.write((char*)&IncandescenceMapFileLength, sizeof(unsigned int));

        out.write((char*)&SpecularExponent, sizeof(float));
        out.write((char*)&ReflectionFactor, sizeof(float));
        out.write((char*)&IndexStart, sizeof(unsigned int));
        out.write((char*)&IndexEnd, sizeof(unsigned int));

        out.write(ColorMapFile.c_str(), ColorMapFileLength);
        out.write(NormalMapFile.c_str(), NormalMapFileLength);
        out.write(SpecularMapFile.c_str(), SpecularMapFileLength);
        out.write(IncandescenceMapFile.c_str(), IncandescenceMapFileLength);
    }

    virtual void WriteASCII(std::ostream& out) const
    {
        out << "New Material _ not in binary" << endl;
        out << "number of indices: " << Name << " _ not in binary" << endl;

        out << "ColorMapFile length: " << ColorMapFileLength << endl;
        out << "NormalMapFile length: " << NormalMapFileLength << endl;
        out << "SpecularMapFile length: " << SpecularMapFileLength << endl;
        out << "IncandescenceMapFile length: " << IncandescenceMapFileLength << endl;

        out << "SpecularExponent: " << SpecularExponent << endl;
        out << "ReflectionFactor: " << ReflectionFactor << endl;
        out << "IndexStart: " << IndexStart << endl;
        out << "IndexEnd: " << IndexEnd << endl;

        if (ColorMapFileLength > 0)
            out << "ColorMapFile: " << ColorMapFile << endl;

        if (NormalMapFileLength > 0)
            out << "NormalMapFile: " << NormalMapFile << endl;
        
        if (SpecularMapFileLength > 0)
            out << "SpecularMapFile: " << SpecularMapFile << endl;
     
        if (IncandescenceMapFileLength > 0)
            out << "IncandescenceMapFile: " << IncandescenceMapFile << endl;
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
	void grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node);
	void grabPhongProperties(MaterialNode& material_node, MFnDependencyNode& node);
};

#endif // Material_Material_h__