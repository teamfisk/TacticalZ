#include "../Common.h"

struct ComponentInfo
{
	struct Meta_t
	{
		std::string Annotation;
		int Allocation = 0;
	};

	std::string Name;
	std::unordered_map<std::string, std::string> FieldTypes;
	std::unordered_map<std::string, unsigned int> FieldOffsets;
	Meta_t Meta;
};