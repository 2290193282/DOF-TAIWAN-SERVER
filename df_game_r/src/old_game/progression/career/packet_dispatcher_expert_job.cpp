#include "old_game/progression/career/packet_dispatcher_expert_job.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/progression/career/packet_dispatcher_expert_job_abi.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8596BB4
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJobC2Ev
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::Dispatcher_CompoundItemByExpertJob(void)
Dispatcher_CompoundItemByExpertJob::Dispatcher_CompoundItemByExpertJob() {}

// IDA_EA: 0x82411AC
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJobD2Ev
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::~Dispatcher_CompoundItemByExpertJob()
// IDA_EA: 0x82411DC
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJobD0Ev
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::~Dispatcher_CompoundItemByExpertJob()
Dispatcher_CompoundItemByExpertJob::~Dispatcher_CompoundItemByExpertJob() {}

// IDA_EA: 0x821EB66
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJob12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    if (!user->getCurCharacR()) {
        return LineFunc(
            55100,
            "virtual int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->GetCurCharacExpertJobType()) {
        return LineFunc(
            55103,
            "virtual int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->get_state() != 3) {
        return 0;
    }
    if (user->CheckInTrade()) {
        return 0;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(
            user, RestrictCategory::AuthMobile, 19)) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB, 209));
        return 0;
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(28));
    if (security_result) {
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB,
            static_cast<unsigned char>(security_result)));
        return 0;
    }

    unsigned long recipe_slot = 0;
    if (packet.get_int(recipe_slot) != 1) {
        return LineFunc(
            55138,
            "virtual int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    unsigned short compound_count = 0;
    if (packet.get_short(compound_count) != 1) {
        return LineFunc(
            55140,
            "virtual int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!compound_count) {
        return LineFunc(
            55146,
            "virtual int Dispatcher_CompoundItemByExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int result = user->compound_item(
        static_cast<int>(recipe_slot),
        true,
        compound_count,
        static_cast<int>(ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB));
    if (result <= 0) {
        if (user && result == 0) {
            const int expert_job_type = user->GetCurCharacExpertJobType();
            if (expert_job_type == 2) {
                APSystem::CUserProc::ClearActionAndSendtoUser(
                    user,
                    static_cast<APSystem::_ActionGroupIndex>(29),
                    0,
                    ENUM_PACKETCLASS_NOTIPACKET);
            } else if (expert_job_type == 4) {
                APSystem::CUserProc::ClearActionAndSendtoUser(
                    user,
                    static_cast<APSystem::_ActionGroupIndex>(30),
                    0,
                    ENUM_PACKETCLASS_NOTIPACKET);
            }
        }
        return 0;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB);
    response.put_byte(0);
    response.put_byte(result);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233ECA
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJob4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompoundItemByExpertJob::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233ED4
// IDA_NAME: _ZN34Dispatcher_CompoundItemByExpertJob7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompoundItemByExpertJob::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompoundItemByExpertJob::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596BD0
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJobC2Ev
// DEMANGLED: Dispatcher_GiveupExpertJob::Dispatcher_GiveupExpertJob(void)
Dispatcher_GiveupExpertJob::Dispatcher_GiveupExpertJob() {}

// IDA_EA: 0x824115E
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJobD2Ev
// DEMANGLED: Dispatcher_GiveupExpertJob::~Dispatcher_GiveupExpertJob()
// IDA_EA: 0x824118E
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJobD0Ev
// DEMANGLED: Dispatcher_GiveupExpertJob::~Dispatcher_GiveupExpertJob()
Dispatcher_GiveupExpertJob::~Dispatcher_GiveupExpertJob() {}

// IDA_EA: 0x821EED2
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJob12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GiveupExpertJob::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GiveupExpertJob::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            55191,
            "virtual int Dispatcher_GiveupExpertJob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    // The old body neither checks this global pointer nor catches ENUM_ERROR.
    // Manager construction/lifetime remains explicit link debt, so preserve
    // that direct call and leave this class unregistered.
    GlobalData::s_ExpertJobMgr->OnGiveup(user);
    return 0;
}

// IDA_EA: 0x8233EDE
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJob4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GiveupExpertJob::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GiveupExpertJob::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233EE8
// IDA_NAME: _ZN26Dispatcher_GiveupExpertJob7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GiveupExpertJob::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GiveupExpertJob::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
