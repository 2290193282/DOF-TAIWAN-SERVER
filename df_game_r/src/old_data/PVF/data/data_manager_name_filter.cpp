#include "old_data/PVF/data/data_manager.h"

#include "common/manager/invalid_char_data.h"
#include "common/manager/slang_filter.h"
#include "common/manager/slang_name_data.h"

#include <algorithm>
#include <stdint.h>
#include <string>
#include <string.h>

namespace {

template <typename RangeVector, typename Value>
bool ValueInRanges(const RangeVector& ranges, Value value, bool inclusive_end) {
    for (typename RangeVector::const_iterator it = ranges.begin();
         it != ranges.end(); ++it) {
        if (it->begin <= value &&
            (inclusive_end ? it->end >= value : it->end > value)) {
            return true;
        }
    }
    return false;
}

template <typename CharVector, typename Value>
bool ValueInChars(const CharVector& chars, Value value) {
    for (typename CharVector::const_iterator it = chars.begin();
         it != chars.end(); ++it) {
        if (*it == value) {
            return true;
        }
    }
    return false;
}

uint16_t GetTwoByteCharacter(const char* text, std::size_t index) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(static_cast<unsigned char>(text[index])) << 8) |
        static_cast<unsigned char>(text[index + 1]));
}

}  // namespace

// IDA_EA: 0x8362C0E
// IDA_NAME: _ZN12CDataManager19_isOneByteCharacterEh
// DEMANGLED: CDataManager::_isOneByteCharacter(unsigned char)
int CDataManager::_isOneByteCharacter(unsigned char character) {
    (void)character;
    return 1;
}

// IDA_EA: 0x8362C22
// IDA_NAME: _ZN12CDataManager19_subPunctuationMarkEPKcPc
// DEMANGLED: CDataManager::_subPunctuationMark(char const*,char *)
char* CDataManager::_subPunctuationMark(const char* source,
                                        char* destination) {
    const SlangNameData* const slang_name_data = &slang_name_data_;
    std::size_t write_index = 0;

    for (std::size_t read_index = 0; ::strlen(source) > read_index;
         ++read_index) {
        const unsigned char one_byte =
            static_cast<unsigned char>(source[read_index]);
        if (_isOneByteCharacter(one_byte)) {
            if (std::find(slang_name_data->one_byte_skips.begin(),
                          slang_name_data->one_byte_skips.end(),
                          one_byte) == slang_name_data->one_byte_skips.end()) {
                destination[write_index++] = source[read_index];
            }
        } else {
            const uint16_t two_byte = GetTwoByteCharacter(source, read_index);
            if (std::find(slang_name_data->two_byte_skips.begin(),
                          slang_name_data->two_byte_skips.end(),
                          two_byte) == slang_name_data->two_byte_skips.end()) {
                destination[write_index++] = source[read_index];
                destination[write_index++] = source[read_index + 1];
            }
            ++read_index;
        }
    }

    destination[write_index] = '\0';
    return destination;
}

// IDA_EA: 0x822B668
// IDA_NAME: _ZN12CDataManager23setCharacNameFilterPassEb
// DEMANGLED: CDataManager::setCharacNameFilterPass(bool)
CDataManager* CDataManager::setCharacNameFilterPass(bool pass) {
    charac_name_filter_pass_ = static_cast<unsigned char>(pass);
    return this;
}

// IDA_EA: 0x82A3F42
// IDA_NAME: _ZNK12CDataManager23getCharacNameFilterPassEv
// DEMANGLED: CDataManager::getCharacNameFilterPass(void)const
int CDataManager::getCharacNameFilterPass() const {
    return charac_name_filter_pass_;
}

// IDA_EA: 0x8362E0E
// IDA_NAME: _ZN12CDataManager30hasSpecialCharacterExceptSpaceEPKc
// DEMANGLED: CDataManager::hasSpecialCharacterExceptSpace(char const*)
int CDataManager::hasSpecialCharacterExceptSpace(const char* text) {
    const InvalidCharData* invalid_chars = &invalid_char_data_;

    for (std::size_t index = 0; ::strlen(text) > index; ++index) {
        const unsigned char one_byte =
            static_cast<unsigned char>(text[index]);
        if (_isOneByteCharacter(one_byte)) {
            if (ValueInRanges(invalid_chars->one_byte_ranges, one_byte, false) ||
                ValueInChars(invalid_chars->one_byte_chars, one_byte)) {
                return 1;
            }
        } else {
            const uint16_t two_byte = GetTwoByteCharacter(text, index);
            if (ValueInRanges(invalid_chars->two_byte_ranges, two_byte, true) ||
                ValueInChars(invalid_chars->two_byte_chars, two_byte)) {
                return 1;
            }
            ++index;
        }
    }
    return 0;
}

// IDA_EA: 0x8363140
// IDA_NAME: _ZN12CDataManager19hasSpecialCharacterEPKc
// DEMANGLED: CDataManager::hasSpecialCharacter(char const*)
int CDataManager::hasSpecialCharacter(const char* text) {
    const InvalidCharData* invalid_chars = &invalid_char_data_;

    for (std::size_t index = 0; ::strlen(text) > index; ++index) {
        const unsigned char one_byte =
            static_cast<unsigned char>(text[index]);
        if (_isOneByteCharacter(one_byte)) {
            if (ValueInRanges(invalid_chars->one_byte_ranges, one_byte, true) ||
                ValueInChars(invalid_chars->one_byte_chars, one_byte)) {
                return 1;
            }
        } else {
            const uint16_t two_byte = GetTwoByteCharacter(text, index);
            if (ValueInRanges(invalid_chars->two_byte_ranges, two_byte, true) ||
                ValueInChars(invalid_chars->two_byte_chars, two_byte)) {
                return 1;
            }
            ++index;
        }
    }
    return 0;
}

// IDA_EA: 0x8363472
// IDA_NAME: _ZN12CDataManager16hasPreventStringEPKc
// DEMANGLED: CDataManager::hasPreventString(char const*)
int CDataManager::hasPreventString(const char* text) {
    if (getCharacNameFilterPass()) {
        setCharacNameFilterPass(false);
        return 0;
    }
    return CSyncSlangFilter::GetInstance()->HasSlang(std::string(text));
}

// IDA_EA: 0x836353A
// IDA_NAME: _ZN12CDataManager20hasPreventStringNameEPKc
// DEMANGLED: CDataManager::hasPreventStringName(char const*)
int CDataManager::hasPreventStringName(const char* text) {
    return CSyncSlangFilter::GetInstance()->HasSlangName(std::string(text));
}

// IDA_EA: 0x836360C
// IDA_NAME: _ZN12CDataManager26hasPreventPrefixCharacNameEPKci
// DEMANGLED: CDataManager::hasPreventPrefixCharacName(char const*, int)
int CDataManager::hasPreventPrefixCharacName(const char* text, int length) {
    (void)length;
    const int count = server_parameter_script_.get_prevent_prefix_count();
    for (int index = 0; index < count; ++index) {
        const char* prefix = server_parameter_script_.get_prevent_prefix(index);
        char destination[21];
        ::memset(destination, 0, sizeof(destination));
        ::memcpy(destination, text, ::strlen(prefix));
        if (::strcmp(prefix, destination) == 0) {
            return 1;
        }
    }
    return 0;
}
