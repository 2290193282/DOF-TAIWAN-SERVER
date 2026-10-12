#ifndef OLDSRV_GLOBAL_RESTRICT_INPUTTING_NAME_H_
#define OLDSRV_GLOBAL_RESTRICT_INPUTTING_NAME_H_

#include <utility>
#include <vector>
#include <wchar.h>

namespace restrict_inputting_name {

// IDA_EA: 0x8AC2ECA
// IDA_NAME: _ZN23restrict_inputting_name25RestrictInputtingNameData22importUnicodeRangeDataEPKcPSt6vectorISt4pairIiiESaIS5_EE
// IDA_EA: 0x8AC3168
// IDA_NAME: _ZN23restrict_inputting_name25RestrictInputtingNameDataC2Ev
// IDA_EA: 0x8AC31A0
// IDA_NAME: _ZNK23restrict_inputting_name25RestrictInputtingNameData9isInRangeEw
// IDA_EA: 0x8AC321C
// IDA_NAME: _ZNK23restrict_inputting_name18IsNoneUnicodeRangeclEw
// IDA_EA: 0x8AC3240
// IDA_NAME: _ZN23restrict_inputting_name18IsNoneUnicodeRangeC2Ev
// IDA_EA: 0x8AC2D6C
// IDA_NAME: _ZN23restrict_inputting_name23isUtf8StrInUnicodeRangeEPKc
// IDA_EA: 0x8AC2DE4
// IDA_NAME: _ZN23restrict_inputting_name23isWideStrInUnicodeRangeEPKw
class RestrictInputtingNameData {
public:
    RestrictInputtingNameData();

    static int importUnicodeRangeData(
        const char* filename,
        std::vector<std::pair<int, int> >* range_data);
    int isInRange(wchar_t value) const;

    static bool isLoaded_;
    static std::vector<std::pair<int, int> > unicodeRangeDataList_;
};

class IsNoneUnicodeRange : public RestrictInputtingNameData {
public:
    IsNoneUnicodeRange();
    int operator()(wchar_t value) const;
};

int isUtf8StrInUnicodeRange(const char* text);
int isWideStrInUnicodeRange(const wchar_t* text);

}  // namespace restrict_inputting_name

#endif  // OLDSRV_GLOBAL_RESTRICT_INPUTTING_NAME_H_
