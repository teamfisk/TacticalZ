#ifndef ComponentInfo_h__
#define ComponentInfo_h__

#include "../Common.h"

struct ComponentInfo
{
	struct Meta_t
	{
		std::string Annotation;
		unsigned int Allocation = 0;
        unsigned int Stride = 0;
	};

	std::string Name;
	std::unordered_map<std::string, std::string> FieldTypes;
	std::unordered_map<std::string, unsigned int> FieldOffsets;
	Meta_t Meta;
    std::shared_ptr<char*> Defaults = nullptr;
};

#endif