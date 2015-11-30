#ifndef BoostPathHash_h__
#define BoostPathHash_h__

struct BoostPathHash
{
    template <typename T>
	std::size_t operator()(const boost::filesystem::path& p) const
	{
		return boost::filesystem::hash_value(p);
	}
};

#endif
