#include "old_game/progression/career/packet_dispatcher_condition_event_reward.h"

#include <map>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/condition/condition_event_manager_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x82223A6
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventReward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestConditionEventReward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestConditionEventReward::dispatch_sig(CUser* user,
                                                          PacketBuf& packet) {
    if (!user) {
        return LineFunc(
            56282,
            "virtual int Dispatcher_RequestConditionEventReward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->get_state() != 3) {
        return 0;
    }

    short event_index = 0;
    char reward_step = 0;
    if (packet.get_short(event_index) != 1) {
        return LineFunc(
            56289,
            "virtual int Dispatcher_RequestConditionEventReward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_byte(reward_step) != 1) {
        return LineFunc(
            56290,
            "virtual int Dispatcher_RequestConditionEventReward::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    std::map<int, int> rewards;
    int result = 0;
    CConditionEventManager* const manager =
        G_CGameManager()->GetConditionEventManager();
    if (manager) {
        result = manager->RequestConditionEventReward(user,
                                                       event_index,
                                                       reward_step,
                                                       rewards);
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_REQUEST_CONDITION_EVENT_REWARD);
    if (result) {
        response.put_byte(0);
        response.put_byte(static_cast<char>(result));
    } else {
        response.put_byte(1);
        response.put_byte(static_cast<char>(event_index));
        response.put_byte(reward_step);
        response.put_byte(static_cast<char>(rewards.size()));
        for (std::map<int, int>::const_iterator it = rewards.begin();
             it != rewards.end();
             ++it) {
            response.put_int(it->first);
            response.put_int(it->second);
        }
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233FA6
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestConditionEventReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestConditionEventReward::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233FB0
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestConditionEventReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestConditionEventReward::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8240E52
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventRewardD2Ev
// DEMANGLED: Dispatcher_RequestConditionEventReward::~Dispatcher_RequestConditionEventReward()
// IDA_EA: 0x8240E82
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventRewardD0Ev
// DEMANGLED: Dispatcher_RequestConditionEventReward::~Dispatcher_RequestConditionEventReward()
Dispatcher_RequestConditionEventReward::~Dispatcher_RequestConditionEventReward() {}

// IDA_EA: 0x8596DE4
// IDA_NAME: _ZN38Dispatcher_RequestConditionEventRewardC2Ev
// DEMANGLED: Dispatcher_RequestConditionEventReward::Dispatcher_RequestConditionEventReward(void)
Dispatcher_RequestConditionEventReward::Dispatcher_RequestConditionEventReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
