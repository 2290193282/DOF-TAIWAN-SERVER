#include "old_game/activity/reward/packet_dispatcher_hero_mission_event_state_reward.h"

#include <stdint.h>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8260094
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser* user,
                                                           PacketBuf& packet) {
    int16_t mode = 0;
    if (packet.get_short(mode) != 1) {
        return LineFunc(
            2921,
            "virtual int Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    int16_t mission = 0;
    if (packet.get_short(mission) != 1) {
        return LineFunc(
            2925,
            "virtual int Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    // This contradictory old validation is intentionally retained verbatim.
    if (mode == 1 && mission < 0 && mission > 6) {
        return LineFunc(
            2929,
            "virtual int Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    HeroMissionEvent* const event = ARAD::Singleton<HeroMissionEvent>::Get();
    if (mode == 0) {
        event->sendMissionData(user);
        return 0;
    }

    const bool rewarded = event->rewardMission(
        user,
        static_cast<HeroMissionCondition::MissionNo::T>(mission));
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_HERO_MISSION_DATA_REWARD);
    if (rewarded) {
        response.put_byte(1);
    } else {
        response.put_byte(0);
        response.put_byte(0);
    }
    response.put_short(mode);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x826060E
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_HeroMissionEventState_Reward::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8260618
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::process(CUser*, MSG_BASE&, ParamBase&)
int Dispatcher_HeroMissionEventState_Reward::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x8260622
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::send(CUser*, ParamBase&)
void Dispatcher_HeroMissionEventState_Reward::send(CUser*, ParamBase&) {}

// IDA_EA: 0x82608BA
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_RewardD2Ev
// IDA_EA: 0x82608EA
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_RewardD0Ev
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::~Dispatcher_HeroMissionEventState_Reward()
Dispatcher_HeroMissionEventState_Reward::~Dispatcher_HeroMissionEventState_Reward() {}

// IDA_EA: 0x8597F86
// IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_RewardC2Ev
// DEMANGLED: Dispatcher_HeroMissionEventState_Reward::Dispatcher_HeroMissionEventState_Reward(void)
Dispatcher_HeroMissionEventState_Reward::Dispatcher_HeroMissionEventState_Reward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
