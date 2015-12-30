#include "Core/ConfigFile.h"

ConfigFile::ConfigFile(std::string path)
{
	m_Path = path;

	boost::filesystem::path defaultFile;
	defaultFile = m_Path.parent_path() / ("Default" + m_Path.filename().string());

	std::fstream file;
	// Read defaults
	file.open(defaultFile.string());
	if (file) {
		try {
			file >> m_Defaults;
		} catch (ini_file::ini_exceptions::ini_file_exception& e) {
			LOG_ERROR("Failed to parse \"%s\":\n%s", defaultFile.string().c_str(), "ini_file_exception");
		}
	} else {
		LOG_WARNING("Failed to find \"%s\"! Relying on hardcoded default values!", defaultFile.string().c_str());
	}
	file.close();
	
	// Read overrides
	file.open(m_Path.string());
	if (file) {
		try {
			file >> m_Overrides;
		} catch (ini_file::ini_exceptions::ini_file_exception& e) {
			LOG_ERROR("Failed to parse \"%s\":\n%s", defaultFile.string().c_str(), "ini_file_exception");
		}
	} else {
		LOG_ERROR("Failed to find \"%s\"! Relying on hardcoded default values!", defaultFile.string().c_str());
	}
	file.close();

	// Merge
	mergeINI(m_Merged, m_Defaults);
	mergeINI(m_Merged, m_Overrides);
}

const ini_file::section* ConfigFile::GetSection(std::string section)
{
	auto sectionIt = m_Merged.find(section);
	if (sectionIt == m_Merged.end()) {
		LOG_WARNING("%s: Unknown section \"%s\"", m_Path.string().c_str(), section.c_str());
		return nullptr;
	}

	return sectionIt->second.get();
}

const ini_file::param* ConfigFile::GetParam(std::string key)
{
	std::string section;
	std::string param;
	if (auto tokens = tokenizeKey(key)) {
		std::tie(section, param) = *tokens;
	} else {
		LOG_WARNING("%s: Malformed config key \"%s\"", m_Path.string().c_str(), key.c_str());
		return nullptr;
	}

	auto sectionIt = m_Merged.find(section);
	if (sectionIt == m_Merged.end()) {
		LOG_WARNING("%s: Unknown section \"%s\"", m_Path.string().c_str(), section.c_str());
		return nullptr;
	}

	auto paramIt = sectionIt->second->find(param);
	if (paramIt == sectionIt->second->end()) {
		LOG_WARNING("%s: Unknown section param \"%s\"", m_Path.string().c_str(), key.c_str());
		return nullptr;
	}

	return paramIt->second.get();
}

void ConfigFile::SaveToDisk()
{
	std::ofstream file(m_Path.string());
	file << m_Overrides;
	file.close();
}

void ConfigFile::mergeINI(ini_file::section_map& to, const ini_file::section_map& from)
{
	for (auto& section : from) {
		for (auto& param : *section.second) {
			auto& p = to[section.first][param.first];
			if (!param.second->get_comment().empty()) {
				p.set_comment(param.second->get_comment());
			}
			p.set_value(param.second->get_value());
		}
	}
}

boost::optional<std::pair<std::string, std::string>> ConfigFile::tokenizeKey(std::string key)
{
	std::size_t delimiter = key.find_last_of('.');
	if (delimiter == std::string::npos) {
		return boost::none;
	}

	std::string section = key.substr(0, delimiter);
	if (section.empty()) {
		return boost::none;
	}
	std::string param = key.substr(delimiter + 1);
	if (param.empty()) {
		return boost::none;
	}

	return std::make_pair(section, param);
}