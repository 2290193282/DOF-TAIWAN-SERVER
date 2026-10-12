#ifndef OLDSRV_OLD_DATA_PVF_COMMON_LEGACY_STRING_ABI_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_LEGACY_STRING_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <unordered_map>

// IDA unit: 99_misc_global/{StringData,CharStringData,CharString} and
// 05_world_enter_village/StringUnorderedMap<CharStringHashTrait,...>.
// These data types are a direct ABI dependency of CNRDScriptData.  In the
// original 32-bit server a CharString is one buffer pointer and each map is a
// libstdc++ unordered_map subobject of 0x20 bytes.
class StringData {
public:
    void incRef();
    void decRef();
    bool isUnique() const;
    bool isShared() const;
    int32_t getSize() const;
    unsigned char* getBuffer();
    static StringData* create(int32_t size);

protected:
    volatile int ref_count_;
    int32_t size_;
};

class CharStringData : public StringData {
public:
    int32_t getLength() const;
    char* getBuffer();
    static CharStringData* create(int32_t length);
    static CharStringData* createTerminated(int32_t length);
};

class CharString {
public:
    CharString();
    CharString(const CharString& source);
    CharString(const char* source);
    CharString(const char* source, int32_t length);
    CharString(const std::string& source);
    ~CharString();

    const char* c_str() const;
    int32_t length() const;
    CharString& operator=(const char* source);
    CharString& operator=(const CharString& source);
    CharString& operator=(const std::string& source);
    void clear();
    void assign(const char* source);
    void assign(const char* source, int32_t length);
    void assign(const CharString& source);
    void attachData(CharStringData* data);
    void replaceData(CharStringData* data);
    CharStringData* getData() const;

private:
    char* buffer_;
};

struct CharStringHashTrait {
    struct HashKey {
        size_t operator()(const char* value) const;
    };
    struct EqualKey {
        bool operator()(const char* left, const char* right) const;
    };
};

template <typename Trait, typename Value>
class StringUnorderedMap;

template <>
class StringUnorderedMap<CharStringHashTrait, int> {
public:
    class ValueEx {
    public:
        ValueEx();
        ValueEx(const ValueEx& source);
        ~ValueEx();
        ValueEx& operator=(const ValueEx& source);

        CharString key_;
        int value_;
    };

    typedef std::unordered_map<const char*, ValueEx,
                               CharStringHashTrait::HashKey,
                               CharStringHashTrait::EqualKey> Map;
    typedef Map::iterator iterator;

    StringUnorderedMap();
    ~StringUnorderedMap();
    int clear();
    void insert(const char* key, const int& value);
    const int* get(const char* key) const;
    int* get(const char* key);
    // IDA_EA: 0x88C43E4 / 0x88C440A
    // IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE5beginEv /
    //           _ZN18StringUnorderedMapI19CharStringHashTraitiE3endEv
    iterator begin();
    iterator end();

private:
    Map values_;
    // The original 32-bit libstdc++ unordered_map payload occupied 0x20
    // bytes.  GCC 4.9's compatible container is 0x1c bytes, so retain the
    // final ABI word explicitly rather than collapsing adjacent CNRD maps.
    uint32_t abi_compat_padding_;
};

template <>
class StringUnorderedMap<CharStringHashTrait, float> {
public:
    class ValueEx {
    public:
        ValueEx();
        ValueEx(const ValueEx& source);
        ~ValueEx();
        ValueEx& operator=(const ValueEx& source);

        CharString key_;
        float value_;
    };

    StringUnorderedMap();
    ~StringUnorderedMap();
    int clear();
    void insert(const char* key, const float& value);
    const float* get(const char* key) const;

private:
    typedef std::unordered_map<const char*, ValueEx,
                               CharStringHashTrait::HashKey,
                               CharStringHashTrait::EqualKey> Map;
    Map values_;
    uint32_t abi_compat_padding_;
};

template <>
class StringUnorderedMap<CharStringHashTrait, std::string> {
public:
    class ValueEx {
    public:
        ValueEx();
        ValueEx(const ValueEx& source);
        ~ValueEx();
        ValueEx& operator=(const ValueEx& source);

        CharString key_;
        std::string value_;
    };

    StringUnorderedMap();
    ~StringUnorderedMap();
    int clear();
    void insert(const char* key, const std::string& value);
    const std::string* get(const char* key) const;

private:
    typedef std::unordered_map<const char*, ValueEx,
                               CharStringHashTrait::HashKey,
                               CharStringHashTrait::EqualKey> Map;
    Map values_;
    uint32_t abi_compat_padding_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CharString) == 0x04u,
              "CharString IDA ABI size must stay 0x04");
static_assert(sizeof(StringUnorderedMap<CharStringHashTrait, int>) == 0x20u,
              "StringUnorderedMap<int> IDA ABI size must stay 0x20");
static_assert(sizeof(StringUnorderedMap<CharStringHashTrait, float>) == 0x20u,
              "StringUnorderedMap<float> IDA ABI size must stay 0x20");
static_assert(sizeof(StringUnorderedMap<CharStringHashTrait, std::string>) ==
                  0x20u,
              "StringUnorderedMap<string> IDA ABI size must stay 0x20");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_LEGACY_STRING_ABI_H_
