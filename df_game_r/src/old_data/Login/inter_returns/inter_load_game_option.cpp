#include "old_data/Login/inter_returns/inter_load_game_option.h"

#include "common/manager/common_time_game_option_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/game_option/db_load_game_option.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

// IDA_EA: 0x84EDEA8
// IDA_NAME: _ZN20Inter_LoadGameOptionC2Ev
// DEMANGLED: Inter_LoadGameOption::Inter_LoadGameOption(void)
Inter_LoadGameOption::Inter_LoadGameOption() {}

// IDA_EA: 0x84F85E0 / 0x84F8610
// IDA_NAME: _ZN20Inter_LoadGameOptionD2Ev / D0Ev
// DEMANGLED: Inter_LoadGameOption::~Inter_LoadGameOption()
Inter_LoadGameOption::~Inter_LoadGameOption() {}

// IDA_EA: 0x84D7E84
// IDA_NAME: _ZN20Inter_LoadGameOption12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadGameOption::dispatch_sig(CUser *,char *,int)
int Inter_LoadGameOption::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(SIG_LOAD_GAME_OPTION))) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "int Inter_LoadGameOption::dispatch_sig(CUser*, char*, int)", 0,
            "[LOGIN_FLOW][GAME_OPTION_NOTI173_INVALID_PAYLOAD] "
            "uid=%d payload=%p payload_size=%d expected=%u",
            user ? user->GetUID() : 0,
            payload,
            payload_size,
            static_cast<unsigned int>(sizeof(SIG_LOAD_GAME_OPTION)));
        return 0;
    }

    const int state = user->get_state();
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "int Inter_LoadGameOption::dispatch_sig(CUser*, char*, int)", 0,
        "[LOGIN_FLOW][GAME_OPTION_NOTI173_ENTER] uid=%d state=%d payload_size=%d",
        user->GetUID(), state, payload_size);

    if (state <= 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "int Inter_LoadGameOption::dispatch_sig(CUser*, char*, int)", 0,
            "[LOGIN_FLOW][GAME_OPTION_NOTI173_SKIP] uid=%d state=%d",
            user->GetUID(), state);
        return 0;
    }

    SIG_LOAD_GAME_OPTION& reply =
        *reinterpret_cast<SIG_LOAD_GAME_OPTION*>(payload);
    CGameOption* const game_option = user->GetGameOptionRef();
    static_assert(sizeof(CETCOption) == sizeof(CGameEtcOptionPayload),
                  "runtime and DB ETC-option layouts must match");
    CETCOption options;
    std::memcpy(&options,
                &reply.etc_option,
                sizeof(reply.etc_option));
    game_option->SetEtcOption(options);
    game_option->SetQuickChatOption(reply.quick_chat[0], 0);
    game_option->SetQuickChatOption(reply.quick_chat[1], 1);
    game_option->SetEmoticonInfo(reply.emoticon_config);

    PacketGuard packet;
    packet.put_header(0, 173);
    packet.put_int(sizeof(reply.etc_option));
    if (G_GameWorld()->IsSchoolPvPChannel()) {
        options.setETCOption(10, 1);
        options.setETCOption(11, 0);
        std::memcpy(&reply.etc_option,
                    &options,
                    sizeof(reply.etc_option));
    }
    packet.put_binary(reinterpret_cast<char*>(&reply.etc_option),
                      static_cast<int>(sizeof(reply.etc_option)));
    packet.put_int(sizeof(reply.quick_chat[0]));
    packet.put_binary(reinterpret_cast<char*>(&reply.quick_chat[0]),
                      static_cast<int>(sizeof(reply.quick_chat[0])));
    packet.put_int(sizeof(reply.quick_chat[1]));
    packet.put_binary(reinterpret_cast<char*>(&reply.quick_chat[1]),
                      static_cast<int>(sizeof(reply.quick_chat[1])));
    const int finalize_result = packet.finalize(true);
    const int send_result = user->Send(packet);
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "int Inter_LoadGameOption::dispatch_sig(CUser*, char*, int)", 0,
        "[LOGIN_FLOW][GAME_OPTION_NOTI173_SEND] uid=%d finalize=%d len=%d send=%d",
        user->GetUID(), finalize_result, packet.get_len(), send_result);
    return 0;
}
