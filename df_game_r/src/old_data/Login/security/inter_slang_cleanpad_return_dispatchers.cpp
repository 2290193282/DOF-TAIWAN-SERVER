#include "old_data/Login/security/inter_slang_cleanpad_return_dispatchers.h"

#include <string>

#include "common/manager/slang_filter.h"
#include "infra/log/format/trace.h"
#include "old_data/db/anti_cheat/db_load_clean_pad_point.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84D5498
// IDA_NAME: _ZN15Inter_Add_Slang12dispatch_sigEP5CUserPci
int Inter_Add_Slang::dispatch_sig(CUser*, char* payload, int) {
    // The legacy sender prefixes this notification with ten bytes of control
    // data; the remaining NUL-terminated string is copied before AddSlang.
    const std::string slang(payload + 10);
    CSyncSlangFilter::GetInstance()->AddSlang(slang);
    return 0;
}

// IDA_EA: 0x84ED930 / 0x84F8B0E / 0x84F8B3E
// IDA_NAME: _ZN15Inter_Add_SlangC2Ev / D2Ev / D0Ev
Inter_Add_Slang::Inter_Add_Slang() : ISigDispatcher() {}
Inter_Add_Slang::~Inter_Add_Slang() {}

// IDA_EA: 0x84D553E
// IDA_NAME: _ZN26Inter_LoadCleanPadPointReq12dispatch_sigEP5CUserPci
int Inter_LoadCleanPadPointReq::dispatch_sig(CUser*, char*, int) {
    DB_LoadCleanPadPoint::makeRequest();
    cMyTrace trace(
        "virtual int Inter_LoadCleanPadPointReq::dispatch_sig(CUser*, char*, int)",
        14109, 0);
    trace("Get CleanPad point Request");
    return 0;
}

// IDA_EA: 0x84EDCE8 / 0x84F8AC0 / 0x84F8AF0
// IDA_NAME: _ZN26Inter_LoadCleanPadPointReqC2Ev / D2Ev / D0Ev
Inter_LoadCleanPadPointReq::Inter_LoadCleanPadPointReq() : ISigDispatcher() {}
Inter_LoadCleanPadPointReq::~Inter_LoadCleanPadPointReq() {}

// IDA_EA: 0x84D5586
// IDA_NAME: _ZN23Inter_LoadCleanPadPoint12dispatch_sigEP5CUserPci
int Inter_LoadCleanPadPoint::dispatch_sig(CUser*, char* payload, int) {
    WongWork::CMCAPManager::setPointPerHackType(
        reinterpret_cast<int*>(payload));
    return 0;
}

// IDA_EA: 0x84EDD04 / 0x84F8A72 / 0x84F8AA2
// IDA_NAME: _ZN23Inter_LoadCleanPadPointC2Ev / D2Ev / D0Ev
Inter_LoadCleanPadPoint::Inter_LoadCleanPadPoint() : ISigDispatcher() {}
Inter_LoadCleanPadPoint::~Inter_LoadCleanPadPoint() {}
