#pragma once

#define PROC_ADDRESS(libraryName, procName) \
    GetProcAddress(LoadLibrary(TEXT(libraryName)), procName)

#define LIB_FUNCTION(returnType, libraryName, procName, ...) \
    typedef returnType _##procName(__VA_ARGS__); \
    _##procName* procName = (_##procName*)PROC_ADDRESS(libraryName, #procName);

#define STR(x) #x

template<typename T>
inline T RoundUp(const T& in_rValue, uint32_t in_round)
{
    return (in_rValue + in_round - 1) & ~(in_round - 1);
}

template<typename T>
inline T RoundDown(const T& in_rValue, uint32_t in_round)
{
    return in_rValue & ~(in_round - 1);
}

inline size_t StringHash(const std::string_view& str)
{
    return XXH3_64bits(str.data(), str.size());
}

template<typename TValue>
constexpr size_t FirstBitLow(TValue value)
{
    constexpr size_t nbits = sizeof(TValue) * 8;
    constexpr auto zero = TValue{};
    constexpr auto one = static_cast<TValue>(1);

    for (size_t i = 0; i < nbits; i++)
    {
        if ((value & (one << i)) != zero)
        {
            return i;
        }
    }

    return 0;
}

inline std::unique_ptr<uint8_t[]> ReadAllBytes(const char* filePath, size_t& fileSize)
{
    FILE* file = fopen(filePath, "rb");

    if (!file)
        return std::make_unique<uint8_t[]>(0);

    fseek(file, 0, SEEK_END);

    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto data = std::make_unique<uint8_t[]>(fileSize);
    fread(data.get(), 1, fileSize, file);

    fclose(file);

    return data;
}

#ifndef __cpp_lib_atomic_ref
// Polyfill for std::atomic_ref
namespace std {
template <typename value_type>
class atomic_ref
{
public:
    atomic_ref(value_type& ref)
    {
        ptr = reinterpret_cast<std::atomic<value_type>*>(&ref);
    }

    void store(value_type desired)
    {
        ptr->store(desired);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired)
    {
        return ptr->compare_exchange_weak(expected, desired);
    }

    void wait(value_type old)
    {
        ptr->wait(old);
    }

    void notify_one()
    {
        ptr->notify_one();
    }

    bool operator=(const value_type& rhs)
    {
        store(rhs);
    }

private:
    std::atomic<value_type>* ptr;
};
}
#endif

