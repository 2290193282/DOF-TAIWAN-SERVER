#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include "infra/log/format/trace.h"
#include "old_game/appearance/avatar/arad_avatar_packet_signals.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A71A
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kUseAvatarRecharge::Arad_INTER_kUseAvatarRecharge(void)
Arad_INTER_kUseAvatarRecharge::Arad_INTER_kUseAvatarRecharge()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(48)) {}

// IDA_EA: 0x819A73E
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeD2Ev
// IDA_EA: 0x819A76E
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kUseAvatarRecharge::~Arad_INTER_kUseAvatarRecharge()
Arad_INTER_kUseAvatarRecharge::~Arad_INTER_kUseAvatarRecharge() {}

// IDA_EA: 0x8199F0E
// IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRecharge12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kUseAvatarRecharge::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kUseAvatarRecharge::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int length) {
    // The original inner-dispatch leaf tests only the typed payload pointer;
    // stream framing was consumed by Inter_DispatcherAradOnly.
    (void)length;
    if (user == 0) {
        return 1678;
    }
    if (user->getCurCharacR() == 0) {
        return 1681;
    }

    arad::SigAradUseAvatarRecharge* const signal =
        reinterpret_cast<arad::SigAradUseAvatarRecharge*>(payload);
    if (signal == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kUseAvatarRecharge::dispatch_sig(CUser*, char*, int)",
            0x696,
            "AVATAR USE AVATAR RECHARGE INTER_DSP(SigAradUseAvatarRecharge) null.");
        return 1687;
    }

    AvatarRechargeServer* const condition_server =
        ARAD::Singleton<AvatarRechargeServer>::Get();
    if (condition_server->CheckConditionUseRecharge(user, signal) != 1) {
        return 0;
    }

    AvatarRechargeServer* const recharge_server =
        ARAD::Singleton<AvatarRechargeServer>::Get();
    if (recharge_server->Recharge(user, signal) != 1) {
        return 0;
    }

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN);
    response.put_byte(1);
    response.put_int(signal->current_endurance);
    response.put_int(signal->added_endurance);
    response.put_short(static_cast<int>(signal->avatar_slot));
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x819A78C
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kEmblemCompound::Arad_INTER_kEmblemCompound(void)
Arad_INTER_kEmblemCompound::Arad_INTER_kEmblemCompound()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(50)) {}

// IDA_EA: 0x819A7B0
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundD2Ev
// IDA_EA: 0x819A7E0
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kEmblemCompound::~Arad_INTER_kEmblemCompound()
Arad_INTER_kEmblemCompound::~Arad_INTER_kEmblemCompound() {}

// IDA_EA: 0x819A0C0
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompound12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kEmblemCompound::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kEmblemCompound::dispatch_sig(CUser* user,
                                              char* payload,
                                              int length) {
    (void)length;
    if (user == 0) {
        return 1718;
    }
    if (user->getCurCharacR() == 0) {
        return 1721;
    }

    arad::SigAradEmblemCompound* const signal =
        reinterpret_cast<arad::SigAradEmblemCompound*>(payload);
    if (signal == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kEmblemCompound::dispatch_sig(CUser*, char*, int)",
            0x6BE,
            "EMBLEM COMPOUND INTER_DSP(kEmblemCompound) null.");
        return 1727;
    }

    EmblemCompoundServer* const condition_server =
        ARAD::Singleton<EmblemCompoundServer>::Get();
    if (condition_server->CheckCondition(user, signal) != 1) {
        return 0;
    }

    EmblemCompoundServer* const process_server =
        ARAD::Singleton<EmblemCompoundServer>::Get();
    if (process_server->Process(user, signal) != 1) {
        return 0;
    }

    cMyTrace trace(
        "virtual int ARAD::DISPATCHER::Arad_INTER_kEmblemCompound::dispatch_sig(CUser*, char*, int)",
        1740,
        0);
    trace("EMBLEM COMPOUND : success [slot1:%d] [slot2:%d] [selectNo:%d] [resultIdx:%d] [charac_no:%d]",
          signal->materials[0].slot,
          signal->materials[1].slot,
          signal->select_no,
          signal->result_item_index,
          user->getCurCharacNo());
    return 0;
}

// IDA_EA: 0x819A7FE
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarConvert::Arad_INTER_kAvatarConvert(void)
Arad_INTER_kAvatarConvert::Arad_INTER_kAvatarConvert()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(51)) {}

// IDA_EA: 0x819A822
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertD2Ev
// IDA_EA: 0x819A852
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarConvert::~Arad_INTER_kAvatarConvert()
Arad_INTER_kAvatarConvert::~Arad_INTER_kAvatarConvert() {}

// IDA_EA: 0x819A20E
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvert12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarConvert::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kAvatarConvert::dispatch_sig(CUser* user,
                                             char* payload,
                                             int length) {
    (void)length;
    if (user == 0) {
        return 1752;
    }
    if (user->getCurCharacR() == 0) {
        return 1755;
    }

    arad::SigAradAvatarConvert* const signal =
        reinterpret_cast<arad::SigAradAvatarConvert*>(payload);
    if (signal == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kAvatarConvert::dispatch_sig(CUser*, char*, int)",
            0x6E0,
            "AVATAR CONVERT INTER_DSP(kAvatarConvert) null.");
        return 1761;
    }

    AvatarConvertServer* const condition_server =
        ARAD::Singleton<AvatarConvertServer>::Get();
    if (condition_server->CheckCondition(user, signal) == 1) {
        AvatarConvertServer* const convert_server =
            ARAD::Singleton<AvatarConvertServer>::Get();
        convert_server->Convert(user, signal);
    }
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
