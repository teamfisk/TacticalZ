#ifndef ConfigFile_h__
#define ConfigFile_h__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>

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
    std::vector<std::pair<std::string, T>> GetAll(std::string key);
	template <typename T>
	void Set(std::string key, T value);

	void SaveToDisk();

private:
	boost::filesystem::path m_Path;
	boost::property_tree::ptree m_PTreeDefaults;
	boost::property_tree::ptree m_PTreeOverrides;
	boost::property_tree::ptree m_PTreeMerged;
};

template <typename T>
T ConfigFile::Get(std::string key, T defaultValue)
{
	return m_PTreeMerged.get<T>(key, defaultValue);
}

template <typename T>
std::vector<std::pair<std::string, T>> ConfigFile::GetAll(std::string key)
{
    std::vector<std::pair<std::string, T>> out;
    auto parent = m_PTreeMerged.find(key);
    if (parent == m_PTreeMerged.not_found()) {
        return out;
    }
    for (auto& child : parent->second) {
        T value = boost::lexical_cast<T>(child.second.data());
        out.push_back(std::make_pair(child.first, value));
    }
    return out;
}

template <typename T>
void ConfigFile::Set(std::string key, T value)
{
	m_PTreeOverrides.put<T>(key, value);
	m_PTreeMerged.put<T>(key, value);
}

#endif
