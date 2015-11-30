#ifndef BoostPathHash_h__
#define BoostPathHash_h__
namespace std
{
	template<> struct hash<boost::filesystem::path>
	{
		size_t operator()(const boost::filesystem::path& p) const
		{
			return boost::filesystem::hash_value(p);
		}
	};
}
//struct BoostPathHash
//{
//	
//    template <typename T>
//	std::size_t operator()(const boost::filesystem::path& p) const
//	{
//		return boost::filesystem::hash_value(p);
//	}
//};

#endif
