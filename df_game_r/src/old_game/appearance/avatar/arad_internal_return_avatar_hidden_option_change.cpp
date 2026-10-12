#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include "infra/log/format/trace.h"
#include "old_game/appearance/avatar/arad_avatar_packet_signals.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A6A8
// IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarHiddenOptionChange::Arad_INTER_kAvatarHiddenOptionChange(void)
Arad_INTER_kAvatarHiddenOptionChange::
    Arad_INTER_kAvatarHiddenOptionChange()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(46)) {}

// IDA_EA: 0x819A6CC
// IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeD2Ev
// IDA_EA: 0x819A6FC
// IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarHiddenOptionChange::~Arad_INTER_kAvatarHiddenOptionChange()
Arad_INTER_kAvatarHiddenOptionChange::~Arad_INTER_kAvatarHiddenOptionChange() {}

// IDA_EA: 0x8199D6E
// IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChange12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarHiddenOptionChange::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kAvatarHiddenOptionChange::dispatch_sig(CUser* user,
                                                        char* payload,
                                                        int length) {
    // The old code only tests the pointer; it does not inspect the supplied
    // inner-stream length before reading this already-typed queue payload.
    (void)length;

    if (user == 0) {
        return 1636;
    }

    arad::SigAvatarHiddenOptionChange* const signal =
        reinterpret_cast<arad::SigAvatarHiddenOptionChange*>(payload);
    if (signal == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kAvatarHiddenOptionChange::dispatch_sig(CUser*, char*, int)",
            0x669,
            "AVATAR HIDDENOPTION CHANGE INTER_DSP(SigAvatarHiddenOptionChange) null.");
        return 1642;
    }

    if (user->getCurCharacR() == 0) {
        return 1648;
    }

    // These are intentionally separate Get() calls: the old leaf performs
    // validation and mutation through the singleton twice, in that order.
    AvatarFixedHiddenOptionServer* const condition_server =
        ARAD::Singleton<AvatarFixedHiddenOptionServer>::Get();
    if (condition_server->CheckCondition(user, signal) != 1) {
        return 0;
    }

    AvatarFixedHiddenOptionServer* const change_server =
        ARAD::Singleton<AvatarFixedHiddenOptionServer>::Get();
    if (change_server->ChangeHiddenOption(user, signal) != 1) {
        return 0;
    }

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE);
    response.put_byte(1);
    response.put_short(static_cast<int>(signal->avatar_slot));
    response.put_short(static_cast<int>(signal->hidden_option));
    response.finalize(true);
    user->Send(response);
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
