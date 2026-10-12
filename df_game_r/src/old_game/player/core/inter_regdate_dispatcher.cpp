#include "old_game/player/core/inter_regdate_dispatcher.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_tools.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84E4FB6
// IDA_NAME: _ZN20Inter_GetUserRegdate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GetUserRegdate::dispatch_sig(CUser *,char *,int)
int Inter_GetUserRegdate::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (payload && payload_size >= 12) {
        checkNewMemberHacklog(ReadU32(payload, 4), user,
                              ReadU32(payload, 8));
    }
    return 0;
}

// IDA_EA: 0x84EEBE4 / 0x84F60B4 / 0x84F60E4
Inter_GetUserRegdate::Inter_GetUserRegdate() : ISigDispatcher() {}
Inter_GetUserRegdate::~Inter_GetUserRegdate() {}
