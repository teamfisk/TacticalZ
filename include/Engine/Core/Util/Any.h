#ifndef Util_Any_h__
#define Util_Any_h__

#include <memory>

struct Any
{
    Any() { }

    template <typename T>
    Any(const T& value)
    {
        Data = std::shared_ptr<char>(new char[sizeof(T)]);
        Size = sizeof(T);
        memcpy(Data.get(), &value, Size);
    }

    template <typename T>
    Any(T&& value)
    {
        Data = std::shared_ptr<char>(new char[sizeof(T)]);
        Size = sizeof(T);
        memcpy(Data.get(), &value, Size);
    }

    template <typename T>
    Any& operator=(const T& value)
    {
        return Any(value);
    }

    template <typename T>
    Any& operator=(T&& value)
    {
        return Any(value);
    }

    std::shared_ptr<char> Data = nullptr;
    std::size_t Size = 0;
};

#endif