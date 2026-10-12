#include "old_data/PVF/common/legacy_string_abi.h"

#include <stdlib.h>
#include <string.h>

namespace {

CharStringData* EmptyCharStringData() {
    // The old server keeps s_emptyCharStringData alive for the process.  Hold
    // one reference here so empty CharString instances retain that lifetime.
    static CharStringData* data = []() {
        CharStringData* created = CharStringData::createTerminated(0);
        created->incRef();
        return created;
    }();
    return data;
}

}  // namespace

// IDA_EA: 0x8AD6884
// IDA_NAME: _ZN10StringData6incRefEv
// DEMANGLED: StringData::incRef(void)
void StringData::incRef() {
    __sync_add_and_fetch(&ref_count_, 1);
}

// IDA_EA: 0x8AD6890
// IDA_NAME: _ZN10StringData6decRefEv
// DEMANGLED: StringData::decRef(void)
void StringData::decRef() {
    if (__sync_sub_and_fetch(&ref_count_, 1) <= 0) {
        ::free(this);
    }
}

// IDA_EA: 0x8AD68C0
// IDA_NAME: _ZNK10StringData8isUniqueEv
// DEMANGLED: StringData::isUnique(void)const
bool StringData::isUnique() const {
    return ref_count_ == 1;
}

// IDA_EA: 0x8AD68D0
// IDA_NAME: _ZNK10StringData8isSharedEv
// DEMANGLED: StringData::isShared(void)const
bool StringData::isShared() const {
    return ref_count_ > 1;
}

// IDA_EA: 0x8AD68E0
// IDA_NAME: _ZNK10StringData7getSizeEv
// DEMANGLED: StringData::getSize(void)const
int32_t StringData::getSize() const {
    return size_;
}

// IDA_EA: 0x8AD68EC
// IDA_NAME: _ZN10StringData9getBufferEv
// DEMANGLED: StringData::getBuffer(void)
unsigned char* StringData::getBuffer() {
    return reinterpret_cast<unsigned char*>(this + 1);
}

// IDA_EA: 0x8AD68F8
// IDA_NAME: _ZN10StringData6createEi
// DEMANGLED: StringData::create(int)
StringData* StringData::create(int32_t size) {
    StringData* data = static_cast<StringData*>(::malloc(size + 8));
    if (data == 0) {
        return 0;
    }
    data->ref_count_ = 0;
    data->size_ = size;
    return data;
}

// IDA_EA: 0x8AD6934
// IDA_NAME: _ZNK14CharStringData9getLengthEv
// DEMANGLED: CharStringData::getLength(void)const
int32_t CharStringData::getLength() const {
    return size_ - 1;
}

// IDA_EA: 0x8AD6942
// IDA_NAME: _ZN14CharStringData9getBufferEv
// DEMANGLED: CharStringData::getBuffer(void)
char* CharStringData::getBuffer() {
    return reinterpret_cast<char*>(this + 1);
}

// IDA_EA: 0x8AD694E
// IDA_NAME: _ZN14CharStringData6createEi
// DEMANGLED: CharStringData::create(int)
CharStringData* CharStringData::create(int32_t length) {
    return static_cast<CharStringData*>(StringData::create(length + 1));
}

// IDA_EA: 0x8AD697E
// IDA_NAME: _ZN14CharStringData16createTerminatedEi
// DEMANGLED: CharStringData::createTerminated(int)
CharStringData* CharStringData::createTerminated(int32_t length) {
    CharStringData* data = CharStringData::create(length);
    data->getBuffer()[length] = 0;
    return data;
}

// IDA_EA: 0x807FFFC
// IDA_NAME: _ZNK10CharString5c_strEv
// DEMANGLED: CharString::c_str(void)const
const char* CharString::c_str() const {
    return buffer_;
}

// IDA_EA: 0x8AD5530
// IDA_NAME: _ZNK10CharString6lengthEv
// DEMANGLED: CharString::length(void)const
int32_t CharString::length() const {
    return getData()->getLength();
}

// IDA_EA: 0x8AD69AA
// IDA_NAME: _ZN10CharStringC2Ev
// DEMANGLED: CharString::CharString(void)
CharString::CharString() : buffer_(0) {
    attachData(EmptyCharStringData());
}

// IDA_EA: 0x8AD69C6
// IDA_NAME: _ZN10CharStringC2ERKS_
// DEMANGLED: CharString::CharString(CharString const&)
CharString::CharString(const CharString& source) : buffer_(0) {
    attachData(source.getData());
}

// IDA_EA: 0x8AD69EE
// IDA_NAME: _ZN10CharStringC2EPKc
// DEMANGLED: CharString::CharString(char const*)
CharString::CharString(const char* source) : buffer_(0) {
    const size_t source_length = ::strlen(source);
    if (source_length != 0) {
        CharStringData* data = CharStringData::create(
            static_cast<int32_t>(source_length));
        ::memcpy(data->getBuffer(), source, source_length + 1);
        attachData(data);
    } else {
        attachData(EmptyCharStringData());
    }
}

// IDA_EA: 0x8AD6A6C
// IDA_NAME: _ZN10CharStringC2EPKci
// DEMANGLED: CharString::CharString(char const*,int)
CharString::CharString(const char* source, int32_t source_length)
    : buffer_(0) {
    if (source_length != 0) {
        CharStringData* data = CharStringData::createTerminated(source_length);
        ::memcpy(data->getBuffer(), source, source_length);
        attachData(data);
    } else {
        attachData(EmptyCharStringData());
    }
}

// IDA_EA: 0x8AD6AD6
// IDA_NAME: _ZN10CharStringC2ERKSs
// DEMANGLED: CharString::CharString(std::string const&)
CharString::CharString(const std::string& source) : buffer_(0) {
    const int32_t source_length = static_cast<int32_t>(source.size());
    if (source_length != 0) {
        CharStringData* data = CharStringData::create(source_length);
        ::memcpy(data->getBuffer(), source.c_str(), source_length + 1);
        attachData(data);
    } else {
        attachData(EmptyCharStringData());
    }
}

// IDA_EA: 0x8AD6BBA
// IDA_NAME: _ZN10CharStringD2Ev
// DEMANGLED: CharString::~CharString()
CharString::~CharString() {
    getData()->decRef();
}

// IDA_EA: 0x8AD6F42
// IDA_NAME: _ZN10CharString6assignEPKc
// DEMANGLED: CharString::assign(char const*)
void CharString::assign(const char* source) {
    assign(source, static_cast<int32_t>(::strlen(source)));
}

// IDA_EA: 0x8AD6F6C
// IDA_NAME: _ZN10CharString6assignEPKci
// DEMANGLED: CharString::assign(char const*,int)
void CharString::assign(const char* source, int32_t source_length) {
    if (source_length != 0) {
        CharStringData* data = CharStringData::createTerminated(source_length);
        ::memcpy(data->getBuffer(), source, source_length);
        replaceData(data);
    } else {
        clear();
    }
}

// IDA_EA: 0x8AD6FCE
// IDA_NAME: _ZN10CharString6assignERKS_
// DEMANGLED: CharString::assign(CharString const&)
void CharString::assign(const CharString& source) {
    if (getData() != source.getData()) {
        replaceData(source.getData());
    }
}

// IDA_EA: 0x8AD706E
// IDA_NAME: _ZN10CharString5clearEv
// DEMANGLED: CharString::clear(void)
void CharString::clear() {
    if (length() != 0) {
        replaceData(EmptyCharStringData());
    }
}

// IDA_EA: 0x8AD70C0
// IDA_NAME: _ZN10CharStringaSEPKc
// DEMANGLED: CharString::operator=(char const*)
CharString& CharString::operator=(const char* source) {
    assign(source);
    return *this;
}

// IDA_EA: 0x8AD70DE
// IDA_NAME: _ZN10CharStringaSERKS_
// DEMANGLED: CharString::operator=(CharString const&)
CharString& CharString::operator=(const CharString& source) {
    assign(source);
    return *this;
}

// IDA_EA: 0x8AD70FC
// IDA_NAME: _ZN10CharStringaSERKSs
// DEMANGLED: CharString::operator=(std::string const&)
CharString& CharString::operator=(const std::string& source) {
    assign(source.c_str(), static_cast<int32_t>(source.size()));
    return *this;
}

// IDA_EA: 0x8AD715C
// IDA_NAME: _ZN10CharString10attachDataEP14CharStringData
// DEMANGLED: CharString::attachData(CharStringData *)
void CharString::attachData(CharStringData* data) {
    data->incRef();
    buffer_ = data->getBuffer();
}

// IDA_EA: 0x8AD7180
// IDA_NAME: _ZN10CharString11replaceDataEP14CharStringData
// DEMANGLED: CharString::replaceData(CharStringData *)
void CharString::replaceData(CharStringData* data) {
    CharStringData* old_data = getData();
    data->incRef();
    buffer_ = data->getBuffer();
    old_data->decRef();
}

// IDA_EA: 0x8AD71BC
// IDA_NAME: _ZNK10CharString7getDataEv
// DEMANGLED: CharString::getData(void)const
CharStringData* CharString::getData() const {
    return reinterpret_cast<CharStringData*>(buffer_ - 8);
}

// IDA_EA: 0x80802A6
// IDA_NAME: _ZNK19CharStringHashTrait7HashKeyclEPKc
// DEMANGLED: CharStringHashTrait::HashKey::operator()(char const*)const
size_t CharStringHashTrait::HashKey::operator()(const char* value) const {
    int32_t hash = 0;
    for (;;) {
        const signed char current = static_cast<signed char>(*value++);
        if (current == 0) {
            break;
        }
        hash = 65599 * hash + current;
    }
    return static_cast<size_t>(hash);
}

// IDA_EA: 0x80802EE
// IDA_NAME: _ZNK19CharStringHashTrait8EqualKeyclEPKcS2_
// DEMANGLED: CharStringHashTrait::EqualKey::operator()(char const*,char const*)const
bool CharStringHashTrait::EqualKey::operator()(const char* left,
                                                const char* right) const {
    return left == right || ::strcmp(left, right) == 0;
}

// IDA_EA: 0x80809C0
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE7ValueExC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::ValueEx::ValueEx(void)
StringUnorderedMap<CharStringHashTrait, int>::ValueEx::ValueEx()
    : key_(), value_() {}

// IDA_EA: 0x80809D4
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE7ValueExD2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::ValueEx::~ValueEx()
StringUnorderedMap<CharStringHashTrait, int>::ValueEx::~ValueEx() {}

// IDA_EA: 0x8081404
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE7ValueExC2ERKS2_
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::ValueEx::ValueEx(StringUnorderedMap<CharStringHashTrait,int>::ValueEx const&)
StringUnorderedMap<CharStringHashTrait, int>::ValueEx::ValueEx(
    const ValueEx& source)
    : key_(source.key_), value_(source.value_) {}

StringUnorderedMap<CharStringHashTrait, int>::ValueEx&
StringUnorderedMap<CharStringHashTrait, int>::ValueEx::operator=(
    const ValueEx& source) {
    if (this != &source) {
        key_ = source.key_;
        value_ = source.value_;
    }
    return *this;
}

// IDA_EA: 0x8080348
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiEC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::StringUnorderedMap(void)
StringUnorderedMap<CharStringHashTrait, int>::StringUnorderedMap()
    : values_() {
    values_.rehash(10);
}

// IDA_EA: 0x80803B2
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiED2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::~StringUnorderedMap()
StringUnorderedMap<CharStringHashTrait, int>::~StringUnorderedMap() {}

// IDA_EA: 0x8080966
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE5clearEv
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::clear(void)
int StringUnorderedMap<CharStringHashTrait, int>::clear() {
    values_.clear();
    return 0;
}

// IDA_EA: 0x80809FE
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE6insertEPKcRKi
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::insert(char const*,int const&)
void StringUnorderedMap<CharStringHashTrait, int>::insert(const char* key,
                                                           const int& value) {
    if (values_.find(key) != values_.end()) {
        return;
    }
    ValueEx entry;
    entry.key_ = key;
    entry.value_ = value;
    values_.insert(std::make_pair(entry.key_.c_str(), entry));
}

// IDA_EA: 0x8080E7A
// IDA_NAME: _ZNK18StringUnorderedMapI19CharStringHashTraitiE3getEPKc
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::get(char const*)const
const int* StringUnorderedMap<CharStringHashTrait, int>::get(
    const char* key) const {
    Map::const_iterator found = values_.find(key);
    return found == values_.end() ? 0 : &found->second.value_;
}

// IDA_EA: 0x88C4348
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE3getEPKc
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::get(char const*)
int* StringUnorderedMap<CharStringHashTrait, int>::get(const char* key) {
    Map::iterator found = values_.find(key);
    return found == values_.end() ? 0 : &found->second.value_;
}

// IDA_EA: 0x88C43E4
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE5beginEv
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::begin(void)
StringUnorderedMap<CharStringHashTrait, int>::iterator
StringUnorderedMap<CharStringHashTrait, int>::begin() {
    return values_.begin();
}

// IDA_EA: 0x88C440A
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitiE3endEv
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,int>::end(void)
StringUnorderedMap<CharStringHashTrait, int>::iterator
StringUnorderedMap<CharStringHashTrait, int>::end() {
    return values_.end();
}

// IDA_EA: 0x8080B2E
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfE7ValueExC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::ValueEx::ValueEx(void)
StringUnorderedMap<CharStringHashTrait, float>::ValueEx::ValueEx()
    : key_(), value_() {}

// IDA_EA: 0x8080B42
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfE7ValueExD2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::ValueEx::~ValueEx()
StringUnorderedMap<CharStringHashTrait, float>::ValueEx::~ValueEx() {}

// IDA_EA: 0x80815D6
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfE7ValueExC2ERKS2_
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::ValueEx::ValueEx(StringUnorderedMap<CharStringHashTrait,float>::ValueEx const&)
StringUnorderedMap<CharStringHashTrait, float>::ValueEx::ValueEx(
    const ValueEx& source)
    : key_(source.key_), value_(source.value_) {}

StringUnorderedMap<CharStringHashTrait, float>::ValueEx&
StringUnorderedMap<CharStringHashTrait, float>::ValueEx::operator=(
    const ValueEx& source) {
    if (this != &source) {
        key_ = source.key_;
        value_ = source.value_;
    }
    return *this;
}

// IDA_EA: 0x80803EE
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfEC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::StringUnorderedMap(void)
StringUnorderedMap<CharStringHashTrait, float>::StringUnorderedMap()
    : values_() {
    values_.rehash(10);
}

// IDA_EA: 0x8080458
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfED2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::~StringUnorderedMap()
StringUnorderedMap<CharStringHashTrait, float>::~StringUnorderedMap() {}

// IDA_EA: 0x808097A
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfE5clearEv
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::clear(void)
int StringUnorderedMap<CharStringHashTrait, float>::clear() {
    values_.clear();
    return 0;
}

// IDA_EA: 0x8080B6C
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitfE6insertEPKcRKf
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::insert(char const*,float const&)
void StringUnorderedMap<CharStringHashTrait, float>::insert(
    const char* key,
    const float& value) {
    if (values_.find(key) != values_.end()) {
        return;
    }
    ValueEx entry;
    entry.key_ = key;
    entry.value_ = value;
    values_.insert(std::make_pair(entry.key_.c_str(), entry));
}

// IDA_EA: 0x8080EDE
// IDA_NAME: _ZNK18StringUnorderedMapI19CharStringHashTraitfE3getEPKc
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,float>::get(char const*)const
const float* StringUnorderedMap<CharStringHashTrait, float>::get(
    const char* key) const {
    Map::const_iterator found = values_.find(key);
    return found == values_.end() ? 0 : &found->second.value_;
}

// IDA_EA: 0x8080C9C
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsE7ValueExC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::ValueEx::ValueEx(void)
StringUnorderedMap<CharStringHashTrait, std::string>::ValueEx::ValueEx()
    : key_(), value_() {}

// IDA_EA: 0x8080CE2
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsE7ValueExD2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::ValueEx::~ValueEx()
StringUnorderedMap<CharStringHashTrait, std::string>::ValueEx::~ValueEx() {}

// IDA_EA: 0x80817A8
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsE7ValueExC2ERKS2_
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::ValueEx::ValueEx(StringUnorderedMap<CharStringHashTrait,std::string>::ValueEx const&)
StringUnorderedMap<CharStringHashTrait, std::string>::ValueEx::ValueEx(
    const ValueEx& source)
    : key_(source.key_), value_(source.value_) {}

StringUnorderedMap<CharStringHashTrait, std::string>::ValueEx&
StringUnorderedMap<CharStringHashTrait, std::string>::ValueEx::operator=(
    const ValueEx& source) {
    if (this != &source) {
        key_ = source.key_;
        value_ = source.value_;
    }
    return *this;
}

// IDA_EA: 0x8080494
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsEC2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::StringUnorderedMap(void)
StringUnorderedMap<CharStringHashTrait, std::string>::StringUnorderedMap()
    : values_() {
    values_.rehash(10);
}

// IDA_EA: 0x80804FE
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsED2Ev
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::~StringUnorderedMap()
StringUnorderedMap<CharStringHashTrait, std::string>::~StringUnorderedMap() {}

// IDA_EA: 0x808098E
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsE5clearEv
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::clear(void)
int StringUnorderedMap<CharStringHashTrait, std::string>::clear() {
    values_.clear();
    return 0;
}

// IDA_EA: 0x8080D3E
// IDA_NAME: _ZN18StringUnorderedMapI19CharStringHashTraitSsE6insertEPKcRKSs
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::insert(char const*,std::string const&)
void StringUnorderedMap<CharStringHashTrait, std::string>::insert(
    const char* key,
    const std::string& value) {
    if (values_.find(key) != values_.end()) {
        return;
    }
    ValueEx entry;
    entry.key_ = key;
    entry.value_ = value;
    values_.insert(std::make_pair(entry.key_.c_str(), entry));
}

// IDA_EA: 0x8080F42
// IDA_NAME: _ZNK18StringUnorderedMapI19CharStringHashTraitSsE3getEPKc
// DEMANGLED: StringUnorderedMap<CharStringHashTrait,std::string>::get(char const*)const
const std::string* StringUnorderedMap<CharStringHashTrait, std::string>::get(
    const char* key) const {
    Map::const_iterator found = values_.find(key);
    return found == values_.end() ? 0 : &found->second.value_;
}
