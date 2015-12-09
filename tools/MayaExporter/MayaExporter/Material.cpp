#include "Material.h"

void Material::grabLambertProperties(MaterialNode& material_node, MFnDependencyNode& node)
{
	material_node.Name = node.name().asChar();

	if (findColorTexture(material_node, node)) {
		material_node.Color.fill(1.0f);
	}
	else {
		m_Plug = node.findPlug("colorR");
		m_Plug.getValue(material_node.Color[0]);
		m_Plug = node.findPlug("colorG");
		m_Plug.getValue(material_node.Color[1]);
		m_Plug = node.findPlug("colorB");
		m_Plug.getValue(material_node.Color[2]);

		float TempTransp[3];
		m_Plug = node.findPlug("transparencyR");
		m_Plug.getValue(TempTransp[0]);
		m_Plug = node.findPlug("transparencyG");
		m_Plug.getValue(TempTransp[1]);
		m_Plug = node.findPlug("transparencyB");
		m_Plug.getValue(TempTransp[2]);

		MColor TranspNode(TempTransp[0], TempTransp[1], TempTransp[2]);
		float DummyH, DummyS;
		TranspNode.get(MColor::kHSV, DummyH, DummyS, material_node.Color[3]);
	}

	if (findIncandescenceTexture(material_node, node)) {
		material_node.Incandescence.fill(1.0f);
	}
	else {
		m_Plug = node.findPlug("incandescenceR");
		m_Plug.getValue(material_node.Incandescence[0]);
		m_Plug = node.findPlug("incandescenceG");
		m_Plug.getValue(material_node.Incandescence[1]);
		m_Plug = node.findPlug("incandescenceB");
		m_Plug.getValue(material_node.Incandescence[2]);
	}

	findNormalTexture(material_node, node);
}

void Material::grabBlinnProperties(MaterialNode& material_node, MFnDependencyNode& node)
{
	if (findSpecularTexture(material_node, node)) {
		material_node.Specular.fill(1.0f);
	}
	else {
		m_Plug = node.findPlug("specularColorR");
		m_Plug.getValue(material_node.Specular[0]);
		m_Plug = node.findPlug("specularColorG");
		m_Plug.getValue(material_node.Specular[1]);
		m_Plug = node.findPlug("specularColorB");
		m_Plug.getValue(material_node.Specular[2]);
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
	if (findSpecularTexture(material_node, node)) {
		material_node.Specular.fill(1.0f);
	}
	else {
		m_Plug = node.findPlug("specularColorR");
		m_Plug.getValue(material_node.Specular[0]);
		m_Plug = node.findPlug("specularColorG");
		m_Plug.getValue(material_node.Specular[1]);
		m_Plug = node.findPlug("specularColorB");
		m_Plug.getValue(material_node.Specular[2]);
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
			
			std::string FullPath = TextureNode.findPlug("ftn").asString().asChar();
			m_TexturePaths.push_back(FullPath);

			material_node.ColorMapFile = FullPath.substr(FullPath.find_last_of("/"));

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

					material_node.NormalMapFile = FullPath.substr(FullPath.find_last_of("/"));

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

			material_node.SpecularMapFile = FullPath.substr(FullPath.find_last_of("/"));

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

			material_node.SpecularMapFile = FullPath.substr(FullPath.find_last_of("/"));

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
std::vector<MaterialNode>* Material::DoIt()
{
	// All materials we care about inherit from Lambert
	MItDependencyNodes matIt(MFn::kLambert);

	while (!matIt.isDone()) {
		MFnDependencyNode MaterialFnDN(matIt.thisNode());
		MaterialNode MaterialStorage;

		if (matIt.thisNode().hasFn(MFn::kPhong)) {
			grabLambertProperties(MaterialStorage, MaterialFnDN);
			grabPhongProperties(MaterialStorage, MaterialFnDN);

			m_AllMaterials.push_back(MaterialStorage);
		}
		else if (matIt.thisNode().hasFn(MFn::kBlinn)) {
			grabLambertProperties(MaterialStorage, MaterialFnDN);
			grabBlinnProperties(MaterialStorage, MaterialFnDN);

			m_AllMaterials.push_back(MaterialStorage);
		}
		else if (matIt.thisNode().hasFn(MFn::kLambert)) {
			grabLambertProperties(MaterialStorage, MaterialFnDN);

			m_AllMaterials.push_back(MaterialStorage);

			MaterialStorage.Specular.fill(0.0f);
			MaterialStorage.ReflectionFactor = 0.0f;
			MaterialStorage.SpecularExponent = 0.0f;
		}

		matIt.next();
	}

	return &m_AllMaterials;
}

