#ifndef OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_DATA_MGR_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_DATA_MGR_H_

#include "old_game/player/core/charac_expand_cdata.h"

// IDA owns this enum at global scope. Its underlying type is recovered in
// cuser.h; repeating the forward declaration keeps this runtime header usable
// without pulling the complete CUser state layout into every caller.
enum ENUM_CHARAC_EXPAND_TYPE : int;

namespace charac_expand {

// IDA unit: 04_character_select_create/charac_expand/charac_expand_CDataMgr.
// Canonical C runtime declaration. Lifecycle and update bodies live only in
// old_game/userinfo/charac_expand_cdata_mgr_lifecycle.cpp and
// old_game/userinfo/charac_expand_cdata_mgr_update.cpp.
class CDataMgr {
public:
    // IDA_EA: 0x832A778
    // IDA_NAME: _ZN13charac_expand8CDataMgrC2Ev
    // DEMANGLED: charac_expand::CDataMgr::CDataMgr(void)
    CDataMgr();

    // IDA_EA: 0x832AC48
    // IDA_NAME: _ZN13charac_expand8CDataMgrD2Ev
    // DEMANGLED: charac_expand::CDataMgr::~CDataMgr()
    ~CDataMgr();

    // IDA_EA: 0x832AC94
    // IDA_NAME: _ZNK13charac_expand8CDataMgr7GetDataE23ENUM_CHARAC_EXPAND_TYPE
    // DEMANGLED: charac_expand::CDataMgr::GetData(ENUM_CHARAC_EXPAND_TYPE)const
    CData* GetData(ENUM_CHARAC_EXPAND_TYPE expand_type) const;

    // IDA_EA: 0x832ACB2
    // IDA_NAME: _ZNK13charac_expand8CDataMgr8GetDataRE23ENUM_CHARAC_EXPAND_TYPE
    // DEMANGLED: charac_expand::CDataMgr::GetDataR(ENUM_CHARAC_EXPAND_TYPE)const
    CData* GetDataR(ENUM_CHARAC_EXPAND_TYPE expand_type) const;

    // IDA_EA: 0x832ACD0
    // IDA_NAME: _ZN13charac_expand8CDataMgr6updateEP5CUser
    // DEMANGLED: charac_expand::CDataMgr::update(CUser *)
    int update(CUser* user);

    // IDA_EA: 0x832AD30
    // IDA_NAME: _ZN13charac_expand8CDataMgr6updateEP5CUser23ENUM_CHARAC_EXPAND_TYPE
    // DEMANGLED: charac_expand::CDataMgr::update(CUser *,ENUM_CHARAC_EXPAND_TYPE)
    int update(CUser* user, ENUM_CHARAC_EXPAND_TYPE expand_type);

    // IDA_EA: 0x832AD78
    // IDA_NAME: _ZN13charac_expand8CDataMgr5resetEv
    // DEMANGLED: charac_expand::CDataMgr::reset(void)
    bool reset();

    // IDA_EA: 0x832ADAA
    // IDA_NAME: _ZN13charac_expand8CDataMgr10ResetDailyEv
    // DEMANGLED: charac_expand::CDataMgr::ResetDaily(void)
    bool ResetDaily();

    // IDA_EA: 0x832ADE8
    // IDA_NAME: _ZN13charac_expand8CDataMgr18ResetDailyMidnightEv
    // DEMANGLED: charac_expand::CDataMgr::ResetDailyMidnight(void)
    bool ResetDailyMidnight();

private:
    CData* slots_[20];  // IDA +0x00..+0x4f
};

}  // namespace charac_expand

#endif  // OLDSRV_OLD_GAME_USERINFO_CHARAC_EXPAND_DATA_MGR_H_
