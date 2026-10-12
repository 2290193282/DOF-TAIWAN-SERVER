#include "infra/security/policy/restrict_inputting_name.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

namespace restrict_inputting_name {

bool RestrictInputtingNameData::isLoaded_ = false;
std::vector<std::pair<int, int> >
    RestrictInputtingNameData::unicodeRangeDataList_;

// IDA_EA: 0x8AC2ECA
// IDA_NAME: _ZN23restrict_inputting_name25RestrictInputtingNameData22importUnicodeRangeDataEPKcPSt6vectorISt4pairIiiESaIS5_EE
// DEMANGLED: restrict_inputting_name::RestrictInputtingNameData::importUnicodeRangeData(char const*,std::vector<std::pair<int,int>>*)
int RestrictInputtingNameData::importUnicodeRangeData(
    const char* filename,
    std::vector<std::pair<int, int> >* range_data) {
    if (!loadRDARScriptFile("etc\\", filename)) {
        return 0;
    }

    std::string token;
    while (ScanType(token, true) == 1) {
        if (token == "[allow unicode range]") {
            std::vector<std::pair<int, int> > parsed;
            while (true) {
                if (ScanType(token, true) != 1) {
                    return 0;
                }
                if (token == "[/allow unicode range]") {
                    break;
                }
                if (token == "[range]") {
                    int begin = 0;
                    int end = 0;
                    if (!ScanInt(&begin) || !ScanInt(&end)) {
                        return 0;
                    }
                    parsed.push_back(std::make_pair(begin, end));
                }
            }
            *range_data = parsed;
        }
    }

    return 0;
}

// IDA_EA: 0x8AC3168
// IDA_NAME: _ZN23restrict_inputting_name25RestrictInputtingNameDataC2Ev
// DEMANGLED: restrict_inputting_name::RestrictInputtingNameData::RestrictInputtingNameData(void)
RestrictInputtingNameData::RestrictInputtingNameData() {
    if (isLoaded_ != true) {
        isLoaded_ = importUnicodeRangeData(
            "RestrictNameUnicodeRangeTable.etc",
            &unicodeRangeDataList_) != 0;
    }
}

// IDA_EA: 0x8AC31A0
// IDA_NAME: _ZNK23restrict_inputting_name25RestrictInputtingNameData9isInRangeEw
// DEMANGLED: restrict_inputting_name::RestrictInputtingNameData::isInRange(wchar_t)const
int RestrictInputtingNameData::isInRange(wchar_t value) const {
    for (std::vector<std::pair<int, int> >::size_type index = 0;
         index < unicodeRangeDataList_.size(); ++index) {
        if (unicodeRangeDataList_[index].first <= value &&
            unicodeRangeDataList_[index].second >= value) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8AC321C
// IDA_NAME: _ZNK23restrict_inputting_name18IsNoneUnicodeRangeclEw
// DEMANGLED: restrict_inputting_name::IsNoneUnicodeRange::operator()(wchar_t)const
int IsNoneUnicodeRange::operator()(wchar_t value) const {
    return isInRange(value) ^ 1;
}

// IDA_EA: 0x8AC3240
// IDA_NAME: _ZN23restrict_inputting_name18IsNoneUnicodeRangeC2Ev
// DEMANGLED: restrict_inputting_name::IsNoneUnicodeRange::IsNoneUnicodeRange(void)
IsNoneUnicodeRange::IsNoneUnicodeRange() : RestrictInputtingNameData() {}

// IDA_EA: 0x8AC2D6C
// IDA_NAME: _ZN23restrict_inputting_name23isUtf8StrInUnicodeRangeEPKc
// DEMANGLED: restrict_inputting_name::isUtf8StrInUnicodeRange(char const*)
int isUtf8StrInUnicodeRange(const char* text) {
    std::wstring wide;
    const std::size_t length = std::mbstowcs(0, text, 0);
    if (length != static_cast<std::size_t>(-1)) {
        std::vector<wchar_t> buffer(length + 1);
        std::mbstowcs(&buffer[0], text, length + 1);
        wide.assign(&buffer[0]);
    }
    return isWideStrInUnicodeRange(wide.c_str());
}

// IDA_EA: 0x8AC2DE4
// IDA_NAME: _ZN23restrict_inputting_name23isWideStrInUnicodeRangeEPKw
// DEMANGLED: restrict_inputting_name::isWideStrInUnicodeRange(wchar_t const*)
int isWideStrInUnicodeRange(const wchar_t* text) {
    IsNoneUnicodeRange is_none_unicode_range;
    const std::size_t length = ::wcslen(text);
    return std::any_of(text, text + length, is_none_unicode_range) == false;
}

}  // namespace restrict_inputting_name
