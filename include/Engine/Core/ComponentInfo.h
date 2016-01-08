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

    struct Field_t
    {
        std::string Type;
        unsigned int Offset;
        unsigned int Stride;
    };

	std::string Name;
    std::unordered_map<std::string, Field_t> Fields;
	Meta_t Meta;
    std::shared_ptr<char> Defaults = nullptr;
};

template<>
struct std::hash<ComponentInfo>
{
	inline std::size_t operator()(const ComponentInfo& v) const
	{
		return std::hash<std::string>()(v.Name);
	}
};

#endif