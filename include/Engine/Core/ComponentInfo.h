#ifndef ComponentInfo_h__
#define ComponentInfo_h__

#include "../Common.h"
#include "Entity.h"
#include <boost/shared_array.hpp>

enum class DirtySet
{
    Transform,
    Network
};

struct ComponentInfo
{
    typedef int EnumType;

	struct Meta_t
	{
		std::string Annotation;
		unsigned int Allocation = 0;
        std::map<std::string, std::string> FieldAnnotations;
        std::map<std::string, std::map<std::string, EnumType>> FieldEnumDefinitions;
        bool NetworkReplicated = true;
	};

    struct Field_t
    {
        std::string Name;
        std::string Type;
        unsigned char Index;
        unsigned int Offset;
        unsigned int Stride;
    };

	std::string Name;
    std::unordered_map<std::string, Field_t> Fields;
    std::vector<std::string> FieldsInOrder;
    std::vector<std::string> StringFields;
    unsigned int Stride = 0;
    boost::shared_array<char> Defaults = nullptr;
	std::shared_ptr<Meta_t> Meta = nullptr;

    std::size_t GetHeaderSize() const
    {
        std::size_t size = 0;

        // A component block starts with an entity ID
        size += sizeof(EntityID);

        return size;
    }
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