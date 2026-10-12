#include "old_game/player/core/cuser_lifecycle_support_types.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8278A98
// IDA_NAME: _ZN19Secu_AccountHacking4initEP5CUser
// DEMANGLED: Secu_AccountHacking::init(CUser *)
Secu_AccountHacking* Secu_AccountHacking::init(CUser* user) {
    owner_ = user;
    return resetInfo();
}

// IDA_EA: 0x8278AB4
// IDA_NAME: _ZN19Secu_AccountHacking9resetInfoEv
// DEMANGLED: Secu_AccountHacking * Secu_AccountHacking::resetInfo(void)
Secu_AccountHacking* Secu_AccountHacking::resetInfo() {
    std::memset(detection_flags_, 0, sizeof(detection_flags_));
    avatar_count_ = 0;
    login_time_ = 0;
    return this;
}

// IDA_EA: 0x8279020
// IDA_NAME: _ZN19Secu_AccountHacking12setLoginTimeEv
// DEMANGLED: Secu_AccountHacking::setLoginTime(void)
Secu_AccountHacking* Secu_AccountHacking::setLoginTime() {
    login_time_ = static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    return this;
}
