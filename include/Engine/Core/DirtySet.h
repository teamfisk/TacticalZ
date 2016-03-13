#ifndef DirtySet_h__
#define DirtySet_h__

#include <set>
#include "ComponentInfo.h"

enum class DirtySetType
{
    Transform,
    Network
};

typedef std::unordered_map<DirtySetType, std::set<decltype(ComponentInfo::Field_t::Index)>> DirtyBitField;
typedef std::unordered_map<EntityID, DirtyBitField> DirtySet;

#endif