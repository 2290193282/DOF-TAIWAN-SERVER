#include "old_game/player/core/charac_expand_cdata.h"

namespace charac_expand {

typedef char CData_Size_Must_Be_0x08[(sizeof(CData) == 0x08) ? 1 : -1];

// IDA_EA: 0x80C8BC4
// IDA_NAME: _ZN13charac_expand5CData5alterEv
// DEMANGLED: charac_expand::CData::alter(void)
CData* CData::alter() {
    save_flag_ = 1;
    return this;
}

// IDA_EA: 0x80C8BD0
// IDA_NAME: _ZN13charac_expand5CData18ResetDailyMidnightEv
// DEMANGLED: charac_expand::CData::ResetDailyMidnight(void)
void CData::ResetDailyMidnight() {}

// IDA_EA: 0x80C8BD6
// IDA_NAME: _ZNK13charac_expand5CData7getDataEPc
// DEMANGLED: charac_expand::CData::getData(char *)const
void CData::getData(char* data) const {
    (void)data;
}

// IDA_EA: 0x80C8BDC
// IDA_NAME: _ZN13charac_expand5CData10ResetDailyEv
// DEMANGLED: charac_expand::CData::ResetDaily(void)
void CData::ResetDaily() {}

// IDA_EA: 0x80C8BE2
// IDA_NAME: _ZN13charac_expand5CDataD2Ev
// DEMANGLED: charac_expand::CData::~CData()
// IDA_EA: 0x80C8C08
// IDA_NAME: _ZN13charac_expand5CDataD0Ev
// DEMANGLED: charac_expand::CData::~CData() deleting destructor
CData::~CData() {}

// IDA_EA: 0x80C8CBA
// IDA_NAME: _ZN13charac_expand5CDataC2Ev
// DEMANGLED: charac_expand::CData::CData(void)
CData::CData() {}

// IDA_EA: 0x832AE80
// IDA_NAME: _ZN13charac_expand5CData6isSaveEv
// DEMANGLED: charac_expand::CData::isSave(void)
int CData::isSave() {
    return save_flag_;
}

// IDA_EA: 0x832AE8C
// IDA_NAME: _ZN13charac_expand5CData8saveDataEP5CUser
// DEMANGLED: charac_expand::CData::saveData(CUser *)
int CData::saveData(CUser* user) {
    const int result = _saveData(user) & 0xFF;
    save_flag_ = 0;
    return result;
}

// IDA_EA: 0x832AEBC
// IDA_NAME: _ZN13charac_expand5CData5resetEv
// DEMANGLED: charac_expand::CData::reset(void)
int CData::reset() {
    save_flag_ = 0;
    return _reset();
}

}  // namespace charac_expand
