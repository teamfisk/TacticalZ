#ifndef EnumClassHash_h__
#define EnumClassHash_h__

// http://stackoverflow.com/a/24847480/417018
struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

#endif
