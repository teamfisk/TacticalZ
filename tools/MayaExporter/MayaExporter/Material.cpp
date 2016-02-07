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

			MaterialNode::Texture newTexture;

			newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
			newTexture.FileNameLength = newTexture.FileName.length() + 1;

			MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

			uvRepeatFile.connectedTo(AllConnections, true, false);

			for (int i = 0; i < AllConnections.length(); i++) {
				if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
					MFnDependencyNode place2DTexture(AllConnections[i].node());
					MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

					newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
					newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
				}
			}

			material_node.ColorMaps.push_back(newTexture);
			if(material_node.type == MaterialNode::MaterialType::Basic)
				material_node.type = MaterialNode::MaterialType::SingleTextures;
			return true;

		} else if (AllConnections[i].node().hasFn(MFn::kLayeredTexture)) {
			MGlobal::displayInfo(MString() + "find splat map");
			return findSplatTextures(material_node, material_node.ColorMaps, MFnDependencyNode(AllConnections[i].node()));
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

					MaterialNode::Texture newTexture;

					newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
					newTexture.FileNameLength = newTexture.FileName.length() + 1;

					MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

					uvRepeatFile.connectedTo(AllConnections, true, false);

					for (int i = 0; i < AllConnections.length(); i++) {
						if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
							MFnDependencyNode place2DTexture(AllConnections[i].node());
							MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

							newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
							newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
						}
					}

					material_node.NormalMaps.push_back(newTexture);
					return true;
				} else if (AllConnections[i].node().hasFn(MFn::kLayeredTexture)) {
					MGlobal::displayInfo(MString() + "find splat map");
					return findSplatTextures(material_node, material_node.NormalMaps, MFnDependencyNode(AllConnections[i].node()));
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

			MaterialNode::Texture newTexture;

			newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
			newTexture.FileNameLength = newTexture.FileName.length() + 1;

			MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

			uvRepeatFile.connectedTo(AllConnections, true, false);

			for (int i = 0; i < AllConnections.length(); i++) {
				if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
					//C:\Users\kamisama\Desktop\TacticalZ\assets\test
					MFnDependencyNode place2DTexture(AllConnections[i].node());
					MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

					newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
					newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
				}
			}

			material_node.SpecularMaps.push_back(newTexture);
			if (material_node.type == MaterialNode::MaterialType::Basic)
				material_node.type = MaterialNode::MaterialType::SingleTextures;
			return true;
		} else if (AllConnections[i].node().hasFn(MFn::kLayeredTexture)) {
			MGlobal::displayInfo(MString() + "find splat map");
			return findSplatTextures(material_node, material_node.SpecularMaps, MFnDependencyNode(AllConnections[i].node()));
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

			MaterialNode::Texture newTexture;

			newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
			newTexture.FileNameLength = newTexture.FileName.length() + 1;

			MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

			uvRepeatFile.connectedTo(AllConnections, true, false);

			for (int i = 0; i < AllConnections.length(); i++) {
				if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
					MFnDependencyNode place2DTexture(AllConnections[i].node());
					MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

					newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
					newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
				}
			}

			material_node.IncandescenceMaps.push_back(newTexture);
			if (material_node.type == MaterialNode::MaterialType::Basic)
				material_node.type = MaterialNode::MaterialType::SingleTextures;
			return true;

		} else if (AllConnections[i].node().hasFn(MFn::kLayeredTexture)) {
			MGlobal::displayInfo(MString() + "find splat map");
			return findSplatTextures(material_node, material_node.IncandescenceMaps, MFnDependencyNode(AllConnections[i].node()));
		}
	}
	return false;
}

//C:\Users\kamisama\Desktop\TacticalZ\assets\test

bool Material::findSplatTextures(MaterialNode& material_node, std::vector<MaterialNode::Texture>& textureVector, MFnDependencyNode& node) {
	//Get all Inputs in LayeredTexture
	MPlug inputs = node.findPlug("inputs");
	MGlobal::displayInfo(MString() + "inputs.numElements(): " + inputs.numElements());

	MPlugArray AllConnections;
	MStatus test;
	//Try to find splat texture if using custom splatmap build up.
	inputs[0].child(1).connectedTo(AllConnections, true, false);
	for (int i = 0; i < AllConnections.length(); i++) {
		if (AllConnections[i].node().hasFn(MFn::kMultiplyDivide)) {
			MGlobal::displayInfo(MString() + "found kMultiplyDivide");
			MFnDependencyNode multiplyDivide(AllConnections[i].node());
			multiplyDivide.findPlug("input1", &test).child(0).connectedTo(AllConnections, true, false);;
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

					MaterialNode::Texture newTexture;

					newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
					newTexture.FileNameLength = newTexture.FileName.length() + 1;

					MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

					uvRepeatFile.connectedTo(AllConnections, true, false);

					for (int i = 0; i < AllConnections.length(); i++) {
						if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
							MFnDependencyNode place2DTexture(AllConnections[i].node());
							MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

							newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
							newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
						}
					}
					material_node.SplatMap = newTexture;
					material_node.type = MaterialNode::MaterialType::SplatMapping;
				}
			}
		}
	}

	for (unsigned int i = 0; i < inputs.numElements(); i++) {
		//Get connections to color in input[i]
		inputs[i].child(0).connectedTo(AllConnections, true, false);

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

				MaterialNode::Texture newTexture;

				newTexture.FileName = FullPath.erase(FullPath.find_last_of("."), FullPath.find_last_of(".") - FullPath.size());
				newTexture.FileNameLength = newTexture.FileName.length() + 1;

				MPlug uvRepeatFile = TextureNode.findPlug("repeatUV");

				uvRepeatFile.connectedTo(AllConnections, true, false);

				for (int i = 0; i < AllConnections.length(); i++) {
					if (AllConnections[i].node().hasFn(MFn::kPlace2dTexture)) {
						MFnDependencyNode place2DTexture(AllConnections[i].node());
						MPlug uvRepeat = place2DTexture.findPlug("repeatUV");

						newTexture.UVTiling[0] = uvRepeat.child(0).asFloat();
						newTexture.UVTiling[1] = uvRepeat.child(1).asFloat();
					}
				}
				textureVector.push_back(newTexture);
				break;
			}
		} 
		if (AllConnections.length() == 0) {
			MaterialNode::Texture newTexture;
			newTexture.FileNameLength = 0;
			textureVector.push_back(newTexture);
		}
	}
	return true;
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
			MaterialStorage.NumColorMaps = MaterialStorage.ColorMaps.size();
			MaterialStorage.NumNormalMaps = MaterialStorage.NormalMaps.size();
			MaterialStorage.NumSpecularMaps = MaterialStorage.SpecularMaps.size();
			MaterialStorage.NumIncandescenceMaps = MaterialStorage.IncandescenceMaps.size();
            m_AllMaterials.push_back(MaterialStorage);
        }
		matIt.next();
	}

	return &m_AllMaterials;
}

