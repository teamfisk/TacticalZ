#include "Core/DeveloperConsole.h"

std::map<std::string, std::function<void(std::string)>> DeveloperConsole::m_VariableBindingSetters;
std::map<std::string, std::function<std::string()>> DeveloperConsole::m_VariableBindingGetters;
