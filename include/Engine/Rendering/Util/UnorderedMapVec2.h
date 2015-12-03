#ifndef UnorderedMapVec2_h__
#define UnorderedMapVec2_h__

#include <functional>
#include <boost/functional/hash.hpp>
#include <glm/vec2.hpp>

template<>
struct std::hash<glm::vec2>
{
    inline std::size_t operator()(const glm::vec2 &v) const
    {  
        return boost::hash<float>()(v.x) ^ boost::hash<float>()(v.y);
    }

    inline bool operator()(const glm::vec2& a, const glm::vec2& b)const
    {
        return a.x == b.x && a.y == b.y;
    }

};

#endif