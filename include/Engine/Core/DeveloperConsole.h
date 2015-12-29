#ifndef DeveloperConsole_h__
#define DeveloperConsole_h__

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "../Common.h"
#include "ResourceManager.h"
#include "ConfigFile.h"

class DeveloperConsole
{
public:
	DeveloperConsole() = delete;

	static void MergeConfig(std::string configFile)
	{
        auto config = ResourceManager::Load<ConfigFile>(configFile);
        m_ConfigFiles.push_back(config);
	}

    template <typename T>
    static typename std::enable_if<std::is_enum<T>::value, void>::type
    BindVariable(std::string path, T& variable)
    {
        BindVariable<T, std::underlying_type<T>::type>(path, variable);
    }

    template <typename T, typename R = T>
    static typename std::enable_if<!std::is_enum<R>::value, void>::type
    BindVariable(std::string path, T& variable)
    {
        m_VariableBindingSetters[path] = [&variable](std::string value) {
            variable = static_cast<T>(boost::lexical_cast<R>(value));
        };

        m_VariableBindingGetters[path] = [&variable]() {
            return boost::lexical_cast<std::string>(static_cast<R>(variable));
        };
    }

	static void Consume(const std::string& command)
	{
		if (command.empty()) {
			return;
		}

		boost::char_separator<char> argumentSeparator(" ");
		tokenizer tokens(command, argumentSeparator);

		for (tokenizer::const_iterator it = tokens.begin(); it != tokens.end(); it++) {
			consumeToken(it, tokens.end());
		}
	}

    static void Consume(std::istream& stream)
    {
        std::string input;
        std::getline(stream, input);
		Consume(input);

        /*if (stream.peek() == '\n') {
            printValue(key);
        } else {
            std::string newValue;
            stream >> newValue;

            if (m_VariableBindingSetters.find(key) != m_VariableBindingSetters.end()) {
                m_VariableBindingSetters.at(key)(newValue);
            } else {
                std::cout << "Unknown path: " << key << std::endl;
            }
            return;

            for (auto& config : m_ConfigFiles) {
                config->Set(key, newValue);
                config->SaveToDisk();
                LOG_DEBUG("Key %s set to new value %s and saved to disk.", key.c_str(), newValue.c_str());
            }
        }    */
    }

private:
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

    static std::vector<ConfigFile*> m_ConfigFiles;
    static std::map<std::string, std::function<void(std::string)>> m_VariableBindingSetters;
    static std::map<std::string, std::function<std::string()>> m_VariableBindingGetters;

    static void consumeToken(tokenizer::iterator& token, const tokenizer::iterator end)
    {
        tokenizer::iterator next = token;
        next++;

        if (next == end) {
            printValue(token);
        } else {
            setValue(token, end);
        }
    }

    static void printValue(tokenizer::iterator token)
    {
        std::string key = *token;
        std::string value;

        if (m_VariableBindingGetters.find(key) != m_VariableBindingGetters.end()) {
            std::cout << key << " = " << m_VariableBindingGetters.at(key)() << std::endl;
        } else {
            std::cout << "Unknown path: " << key << std::endl;
        }

        return;

        for (auto& config : m_ConfigFiles) {
            value = config->Get<std::string>(key, "");
        }

        if (!value.empty()) {
            std::cout << key << " = " << value << std::endl;
        } else {
            std::cerr << "Unknown path: " << key << std::endl;
        }
    }

    static void setValue(tokenizer::iterator& token, const tokenizer::iterator end)
    {
        std::string key = *token;
        std::string value = *(++token);

        if (m_VariableBindingSetters.find(key) != m_VariableBindingSetters.end()) {
            m_VariableBindingSetters.at(key)(value);
        } else {
            std::cout << "Unknown path: " << key << std::endl;
        }
    }
};

#endif