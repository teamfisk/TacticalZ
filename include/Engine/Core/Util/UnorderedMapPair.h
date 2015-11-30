#ifndef UnorderedMapPair_h__
#define UnorderedMapPair_h__

#include <boost/functional/hash.hpp>

template<typename S, typename T> struct std::hash<std::pair<S, T>>
{
	inline std::size_t operator()(const std::pair<S, T> &v) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, v.first);
		boost::hash_combine(seed, v.second);
		return seed;
	}
};

#endif