#ifndef ConfigFile_h__
#define ConfigFile_h__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>
#include <ini_file/ini_file.hpp>
#include "../Common.h"
#include "ResourceManager.h"

class ConfigFile : public Resource
{
	friend class ResourceManager;

private:
	ConfigFile(std::string path);

public:
	template <typename T>
	T Get(std::string key, T defaultValue);
	template <typename T>
	void Set(std::string key, T value);

	const ini_file::section* GetSection(std::string section);
	const ini_file::section_map& GetSections() { return m_Merged; }
	const ini_file::param* GetParam(std::string key);

	void SaveToDisk();

private:
	boost::filesystem::path m_Path;
	ini_file::section_map m_Defaults;
	ini_file::section_map m_Overrides;
	ini_file::section_map m_Merged;

	// Merge ini file section map b into a
	void mergeINI(ini_file::section_map& a, const ini_file::section_map& b);
	// Convert an ini key delimited by periods to a section and a param
	boost::optional<std::pair<std::string, std::string>> tokenizeKey(std::string key);
};

template <typename T>
T ConfigFile::Get(std::string key, T defaultValue)
{
	auto param = GetParam(key);
	if (param == nullptr) {
		return defaultValue;
	}

	return boost::lexical_cast<T>(param->get_value());
}

template <typename T>
void ConfigFile::Set(std::string key, T value)
{
	std::string section;
	std::string param;
	if (auto tokens = tokenizeKey(key)) {
		std::tie(section, param) = *tokens;
	} else {
		LOG_WARNING("%s: Malformed config key \"%s\"", m_Path.string().c_str(), key.c_str());
		return;
	}

	m_Overrides[section][param] = boost::lexical_cast<std::string>(value);
	m_Merged[section][param] = boost::lexical_cast<std::string>(value);
}

#endif
