#include "Material.h"

void Material::grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node)
{
	material_node.Name = node.name().asChar();

    if (!findColorTexture(material_node, node)) {
        node.findPlug("colorR").getValue(material_node.DiffuseColor[0]);
        node.findPlug("colorG").getValue(material_node.DiffuseColor[1]);
        node.findPlug("colorB").getValue(material_node.DiffuseColor[2]);

    }


	if (!findIncandescenceTexture(material_node, node)) {
        node.findPlug("incandescenceR").getValue(material_node.IncandescenceColor[0]);
        node.findPlug("incandescenceG").getValue(material_node.IncandescenceColor[1]);
        node.findPlug("incandescenceB").getValue(material_node.IncandescenceColor[2]);
	}

    if (findNormalTexture(material_node, node)) {
        MGlobal::displayWarning(MString() + "Material " + node.name() + " has no normal texture.");
    }
}

void Material::grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node)
{
	if (!findSpecularTexture(material_node, node)) {
        node.findPlug("specularColorR").getValue(material_node.SpecularColor[0]);
        node.findPlug("specularColorG").getValue(material_node.SpecularColor[1]);
        node.findPlug("specularColorB").getValue(material_node.SpecularColor[2]);
	}

	m_Plug = node.findPlug("reflectivity");
	m_Plug.getValue(material_node.ReflectionFactor);

	m_Plug = node.findPlug("eccentricity");
	float TempEccent;
	m_Plug.getValue(TempEccent);

	// Blinn works differently from Phong which is used in-game.
	// This is some magic numbers and math to make a conversion estimate between the two.
	// There is no exact conversion between the two, so there are errors.

	// Phong min/max is around Blinn 0.7/0.1
	TempEccent = std::max(std::min(TempEccent, 0.7f), 0.1f);

	material_node.SpecularExponent = std::max(std::min(((2.66f) + (427.0f) * exp((-14.8f) * TempEccent)), 100.0f), 2.0f);
}

void Material::grabPhongProperties(MaterialNode& material_node, MFnDependencyNode& node)
{
    if (!findSpecularTexture(material_node, node)) {
        node.findPlug("specularColorR").getValue(material_node.SpecularColor[0]);
        node.findPlug("specularColorG").getValue(material_node.SpecularColor[1]);
        node.findPlug("specularColorB").getValue(material_node.SpecularColor[2]);
    }

	m_Plug = node.findPlug("reflectivity");
	m_Plug.getValue(material_node.ReflectionFactor);

	m_Plug = node.findPlug("cosinePower ");
	m_Plug.getValue(material_node.SpecularExponent);
}

bool Material::findColorTexture(MaterialNode& material_node, MFnDependencyNode& node)
{
	MPlugArray AllConnections;
  
	m_Plug = node.findPlug("color", true);
	m_Plug.connectedTo(AllConnections, true, false);

	for (int i = 0; i < AllConnections.length(); i++) {
		if (AllConnections[i].node().hasFn(MFn::kFileTexture)) {
			MFnDependencyNode TextureNode(AllConnections[i].node());
            
			std::string FullPath = TextureNode.findPlug("fileTextureName").asString().asChar();
			m_TexturePaths.push_back(FullPath);

            MString workspace;
            MStatus status = MGlobal::executeCommand(MString("workspace -q -rd;"),
                workspace);
            FullPath = FullPath.substr(workspace.length());
            FullPath = FullPath.substr(FullPath.find_first_of("/") + 1);
			material_node.ColorMapFile = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());

            material_node.ColorMapFileLength = material_node.ColorMapFile.length() + 1;
			return true;
		}
	}

	return false;
}

bool Material::findNormalTexture(MaterialNode& material_node, MFnDependencyNode& node)
{
	MPlugArray AllConnections;
	MPlugArray AllBumpConnections;

	m_Plug = node.findPlug("normalCamera", true);
	m_Plug.connectedTo(AllConnections, true, false);

	for (int i = 0; i < AllConnections.length(); i++) {
		if (AllConnections[i].node().apiType() == MFn::kBump) {
			MFnDependencyNode BumpNode(AllConnections[i].node());

			BumpNode.findPlug("bumpValue").connectedTo(AllBumpConnections, true, false);
			for (int j = 0; j < AllBumpConnections.length(); j++) {
				if (AllBumpConnections[j].node().hasFn(MFn::kFileTexture)) {
					MFnDependencyNode TextureNode(AllBumpConnections[j].node());

					std::string FullPath = TextureNode.findPlug("ftn").asString().asChar();
					m_TexturePaths.push_back(FullPath);

                    MString workspace;
                    MStatus status = MGlobal::executeCommand(MString("workspace -q -rd;"),
                        workspace);
                    FullPath = FullPath.substr(workspace.length());
                    FullPath = FullPath.substr(FullPath.find_first_of("/") + 1);
                    material_node.NormalMapFile = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
                    material_node.NormalMapFileLength = material_node.NormalMapFile.length() + 1;
					return true;
				}
			}
		}
	}

	return false;
}

bool Material::findSpecularTexture(MaterialNode& material_node, MFnDependencyNode& node)
{
	MPlugArray AllConnections;

	m_Plug = node.findPlug("specularColor", true);
	m_Plug.connectedTo(AllConnections, true, false);

	for (int i = 0; i < AllConnections.length(); i++) {
		if (AllConnections[i].node().hasFn(MFn::kFileTexture)) {
			MFnDependencyNode TextureNode(AllConnections[i].node());

			std::string FullPath = TextureNode.findPlug("ftn").asString().asChar();
			m_TexturePaths.push_back(FullPath);

            MString workspace;
            MStatus status = MGlobal::executeCommand(MString("workspace -q -rd;"),
                workspace);
            FullPath = FullPath.substr(workspace.length());
            FullPath = FullPath.substr(FullPath.find_first_of("/") + 1);
            material_node.SpecularMapFile = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
            material_node.SpecularMapFileLength = material_node.SpecularMapFile.length() + 1;
			return true;
		}
	}
	return false;
}

bool Material::findIncandescenceTexture(MaterialNode& material_node, MFnDependencyNode& node)
{
	MPlugArray AllConnections;

	m_Plug = node.findPlug("incandescence", true);
	m_Plug.connectedTo(AllConnections, true, false);

	for (int i = 0; i < AllConnections.length(); i++) {
		if (AllConnections[i].node().hasFn(MFn::kFileTexture)) {
			MFnDependencyNode TextureNode(AllConnections[i].node());

			std::string FullPath = TextureNode.findPlug("ftn").asString().asChar();
			m_TexturePaths.push_back(FullPath);

            MString workspace;
            MStatus status = MGlobal::executeCommand(MString("workspace -q -rd;"),
                workspace);
            FullPath = FullPath.substr(workspace.length());
            FullPath = FullPath.substr(FullPath.find_first_of("/") + 1);
            material_node.IncandescenceMapFile = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
            material_node.IncandescenceMapFileLength = material_node.IncandescenceMapFile.length() + 1;
			return true;
		}
	}
	return false;
}

// Returns the absolute path for all textures. Use for copying texture files.
std::vector<std::string>* Material::TexturePaths()
{
	return &m_TexturePaths;
}

// Traverse the DAG and grab all the materials
std::vector<MaterialNode>* Material::DoIt(Mesh mesh)
{
	// All materials we care about inherit from Lambert
	MItDependencyNodes matIt(MFn::kLambert);
    m_AllMaterials.clear();
	while (!matIt.isDone()) {
		MFnDependencyNode MaterialFnDN(matIt.thisNode());
		MaterialNode MaterialStorage;
        bool meshHasMaterial = false;
        //Mesh Indices is a map with <string : MaterialName, vector<int> : indices>
        int totalIndices = 0;
        for (auto aMeshMaterial : mesh.Indices) {
            MGlobal::displayInfo(MString() + "Material: " + aMeshMaterial.first.c_str() + " " + MaterialFnDN.name().asChar());
            if (aMeshMaterial.first.compare(MaterialFnDN.name().asChar()) == 0) {
                meshHasMaterial = true;
                MaterialStorage.IndexStart = totalIndices;
                MaterialStorage.IndexEnd = totalIndices + aMeshMaterial.second.size() - 1;
                MGlobal::displayInfo("Oh noes, breaking in material");
                break;
            }
            totalIndices += aMeshMaterial.second.size();
        }
        if (meshHasMaterial) {
            grabLambertProperties(MaterialStorage, MaterialFnDN);

            if (matIt.thisNode().hasFn(MFn::kPhong)) {         
                grabPhongProperties(MaterialStorage, MaterialFnDN);

            } else if (matIt.thisNode().hasFn(MFn::kBlinn)) {
                grabBlinnProperties(MaterialStorage, MaterialFnDN);

            } else if (matIt.thisNode().hasFn(MFn::kLambert)) {
                MaterialStorage.ReflectionFactor = 0.0f;
                MaterialStorage.SpecularExponent = 0.0f;      
            }

            m_AllMaterials.push_back(MaterialStorage);
        }
		matIt.next();
	}

	return &m_AllMaterials;
}

