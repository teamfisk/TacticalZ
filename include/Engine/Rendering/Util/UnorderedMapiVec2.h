#pragma once
#ifndef UnorderedMapiVec2_h__
#define UnorderedMapiVec2_h__

#include <functional>
#include <boost/functional/hash.hpp>
#include <glm/vec2.hpp>

template<>
struct std::hash<glm::ivec2>
{
    inline std::size_t operator()(const glm::ivec2 &v) const
    {
        return boost::hash<float>()(v.x) ^ boost::hash<float>()(v.y);
    }

    inline bool operator()(const glm::ivec2& a, const glm::ivec2& b)const
    {
        return a.x == b.x && a.y == b.y;
    }

};

#endif