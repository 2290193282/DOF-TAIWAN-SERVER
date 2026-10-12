#ifndef OLDSRV_COMMON_MANAGER_GAME_ETC_OPTION_PAYLOAD_H_
#define OLDSRV_COMMON_MANAGER_GAME_ETC_OPTION_PAYLOAD_H_

#include "common/config.h"

#include <stdint.h>


// Client-generation payload shared by CMD197, member_game_option.option_1 and
// NOTI173. Its extent is derived from common/config.h.
#pragma pack(push, 1)
struct CGameEtcOptionPayload {
    int16_t values[oldsrv::config::kGameEtcOptionCount];
};
#pragma pack(pop)

static_assert(sizeof(CGameEtcOptionPayload) ==
                  oldsrv::config::kGameEtcOptionBytes,
              "game ETC payload extent must follow common/config.h");

#endif  // OLDSRV_COMMON_MANAGER_GAME_ETC_OPTION_PAYLOAD_H_
