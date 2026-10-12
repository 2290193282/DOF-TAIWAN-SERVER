#include "old_game/player/core/charac_expand_data_mgr.h"

namespace charac_expand {

typedef char CDataMgr_Size_Must_Be_0x50[
    (sizeof(CDataMgr) == 0x50) ? 1 : -1];

// IDA_EA: 0x832AC94
// IDA_NAME: _ZNK13charac_expand8CDataMgr7GetDataE23ENUM_CHARAC_EXPAND_TYPE
// DEMANGLED: charac_expand::CDataMgr::GetData(ENUM_CHARAC_EXPAND_TYPE)const
CData* CDataMgr::GetData(ENUM_CHARAC_EXPAND_TYPE expand_type) const {
    if (static_cast<unsigned int>(expand_type) <= 0x13u) {
        return slots_[expand_type];
    }
    return 0;
}

// IDA_EA: 0x832ACB2
// IDA_NAME: _ZNK13charac_expand8CDataMgr8GetDataRE23ENUM_CHARAC_EXPAND_TYPE
// DEMANGLED: charac_expand::CDataMgr::GetDataR(ENUM_CHARAC_EXPAND_TYPE)const
CData* CDataMgr::GetDataR(ENUM_CHARAC_EXPAND_TYPE expand_type) const {
    if (static_cast<unsigned int>(expand_type) <= 0x13u) {
        return slots_[expand_type];
    }
    return 0;
}

// IDA_EA: 0x832ACD0
// IDA_NAME: _ZN13charac_expand8CDataMgr6updateEP5CUser
// DEMANGLED: charac_expand::CDataMgr::update(CUser *)
int CDataMgr::update(CUser* user) {
    unsigned char result = 1;
    for (int index = 0; index <= 19; ++index) {
        if ((slots_[index]->isSave() & 0xFF) &&
            ((slots_[index]->saveData(user) & 0xFF) != 1)) {
            result = 0;
        }
    }
    return result;
}

// IDA_EA: 0x832AD30
// IDA_NAME: _ZN13charac_expand8CDataMgr6updateEP5CUser23ENUM_CHARAC_EXPAND_TYPE
// DEMANGLED: charac_expand::CDataMgr::update(CUser *,ENUM_CHARAC_EXPAND_TYPE)
int CDataMgr::update(CUser* user, ENUM_CHARAC_EXPAND_TYPE expand_type) {
    unsigned char result = 1;
    if (slots_[expand_type]->isSave() & 0xFF) {
        return ((slots_[expand_type]->saveData(user) & 0xFF) == 1);
    }
    return result;
}

// IDA_EA: 0x832AD78
// IDA_NAME: _ZN13charac_expand8CDataMgr5resetEv
// DEMANGLED: charac_expand::CDataMgr::reset(void)
bool CDataMgr::reset() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 19;
        if (index > 19) {
            break;
        }
        slots_[index]->reset();
    }
    return result;
}

// IDA_EA: 0x832ADAA
// IDA_NAME: _ZN13charac_expand8CDataMgr10ResetDailyEv
// DEMANGLED: charac_expand::CDataMgr::ResetDaily(void)
bool CDataMgr::ResetDaily() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 19;
        if (index > 19) {
            break;
        }
        slots_[index]->ResetDaily();
    }
    return result;
}

// IDA_EA: 0x832ADE8
// IDA_NAME: _ZN13charac_expand8CDataMgr18ResetDailyMidnightEv
// DEMANGLED: charac_expand::CDataMgr::ResetDailyMidnight(void)
bool CDataMgr::ResetDailyMidnight() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 19;
        if (index > 19) {
            break;
        }
        slots_[index]->ResetDailyMidnight();
    }
    return result;
}

}  // namespace charac_expand