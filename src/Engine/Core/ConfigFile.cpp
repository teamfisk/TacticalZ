#include "Core/ConfigFile.h"

ConfigFile::ConfigFile(std::string path)
{
	m_Path = path;

	boost::filesystem::path defaultFile;
	defaultFile = m_Path.parent_path() / ("Default" + m_Path.filename().string());

	// Read defaults
	if (boost::filesystem::exists(defaultFile)) {
		try {
			boost::property_tree::ini_parser::read_ini(defaultFile.string(), m_PTreeDefaults);
		} catch (boost::property_tree::ptree_error& e) {
			LOG_ERROR("Failed to parse \"%s\":\n%s", defaultFile.string().c_str(), e.what());
		}
	} else {
		LOG_ERROR("Failed to find \"%s\"! Relying on hardcoded default values!", defaultFile.string().c_str());
	}

	m_PTreeMerged = m_PTreeDefaults;

	// Read overrides
	if (boost::filesystem::exists(m_Path)) {
		try {
			boost::property_tree::ini_parser::read_ini(m_Path.string(), m_PTreeOverrides);
			for (auto& topLevelNode : m_PTreeOverrides) {
                auto& mergedTopLevelNode = m_PTreeMerged.find(topLevelNode.first);
                for (auto& childOverrideNode : topLevelNode.second) {
                    mergedTopLevelNode->second.put_child(childOverrideNode.first, childOverrideNode.second);
                }
			}
		} catch (boost::property_tree::ptree_error& e) {
			LOG_ERROR("Failed to parse \"%s\":\n%s", m_Path.filename().string().c_str(), e.what());
		}
	}
}

void ConfigFile::SaveToDisk()
{
	boost::property_tree::ini_parser::write_ini(m_Path.string(), m_PTreeOverrides);
}
