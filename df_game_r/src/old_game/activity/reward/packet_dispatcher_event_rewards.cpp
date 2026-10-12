#include "old_game/activity/reward/packet_dispatcher_event_rewards.h"

#include <cstdio>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "common/manager/system_time.h"

namespace {

const char kOneDayLetheDispatch[] =
    "virtual int DIspatcher_OneDayLetheTicket::dispatch_sig(CUser*, PacketBuf&)";
const char kLotteryDispatch[] =
    "virtual int DisPatcher_Lottery::dispatch_sig(CUser*, PacketBuf&)";
const char kMultiBoxLotteryDispatch[] =
    "virtual int DisPatcher_MultiBoxLottery::dispatch_sig(CUser*, PacketBuf&)";
const char kOnTimeRewardDispatch[] =
    "virtual int Dispatcher_RequestOnTimeEventReward::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8596984
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketC2Ev
// DEMANGLED: DIspatcher_OneDayLetheTicket::DIspatcher_OneDayLetheTicket(void)
DIspatcher_OneDayLetheTicket::DIspatcher_OneDayLetheTicket() {}

// IDA_EA: 0x82416DA
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketD2Ev
// DEMANGLED: DIspatcher_OneDayLetheTicket::~DIspatcher_OneDayLetheTicket()
// The non-deleting destructor is emitted from the definition below.
// IDA_EA: 0x824170A
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketD0Ev
// DEMANGLED: DIspatcher_OneDayLetheTicket::~DIspatcher_OneDayLetheTicket()
// The deleting destructor is the compiler-emitted companion of that same body.
DIspatcher_OneDayLetheTicket::~DIspatcher_OneDayLetheTicket() {}

// IDA_EA: 0x821B106
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DIspatcher_OneDayLetheTicket::dispatch_sig(CUser *,PacketBuf &)
int DIspatcher_OneDayLetheTicket::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    // 140828 sub_116A660 writes the fixed request flag 1 and finalizes.
    uint8_t native_request_flag = 0;
    if (packet.get_byte(native_request_flag) != 1 ||
        native_request_flag != 1u) {
        return LineFunc(52905, kOneDayLetheDispatch, 0, 0);
    }

    cMyTrace trace(kOneDayLetheDispatch, 52906, 0);
    trace("ONE_DAY_LETHE : SKILL_INIT_PACKET");

    if (!user) {
        return 0;
    }

    if (user->get_state() == 3) {
        GameWorld* const game_world = G_GameWorld();
        const bool unavailable_pvp_skill_tree =
            game_world->IsPVPChannel() && !game_world->IsPvPSkilTreeChannel();
        if (!unavailable_pvp_skill_tree) {
            CPremiumLetheManager* const manager =
                G_CGameManager()->GetPremiumLetheManager();
            if (manager) {
                if (manager->ResetSkillReq(user) != 1) {
                    user->SendCmdErrorPacket(ENUM_CMDPACKET_ONEDAY_LETHE, 1);
                }
                return 0;
            }
        }
    }

    user->SendCmdErrorPacket(ENUM_CMDPACKET_ONEDAY_LETHE, 1);
    return 0;
}

// IDA_EA: 0x8233D76
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket4readER9PacketBufR8MSG_BASE
// DEMANGLED: DIspatcher_OneDayLetheTicket::read(PacketBuf &,MSG_BASE &)
int DIspatcher_OneDayLetheTicket::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D80
// IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DIspatcher_OneDayLetheTicket::process(CUser *,MSG_BASE &,ParamBase &)
int DIspatcher_OneDayLetheTicket::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595628
// IDA_NAME: _ZN18DisPatcher_LotteryC2Ev
// DEMANGLED: DisPatcher_Lottery::DisPatcher_Lottery(void)
DisPatcher_Lottery::DisPatcher_Lottery()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823328A
// IDA_NAME: _ZN18DisPatcher_LotteryD2Ev
// DEMANGLED: DisPatcher_Lottery::~DisPatcher_Lottery()
// The non-deleting destructor is emitted from the definition below.
// IDA_EA: 0x82332BA
// IDA_NAME: _ZN18DisPatcher_LotteryD0Ev
// DEMANGLED: DisPatcher_Lottery::~DisPatcher_Lottery()
// The deleting destructor is the compiler-emitted companion of that same body.
DisPatcher_Lottery::~DisPatcher_Lottery() {}

// IDA_EA: 0x81F8380
// IDA_NAME: _ZN18DisPatcher_Lottery12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_Lottery::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_Lottery::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    if (user->CheckInTrade()) {
        PacketGuard reply;
        reply.put_header(1, ENUM_CMDPACKET_USE_LOTTERY_ITEM);
        reply.put_byte(0);
        reply.put_byte(218);
        reply.finalize(true);
        user->Send(reply);
        return 0;
    }

    short item_id = 0;
    if (packet.get_short(item_id) != 1) {
        return LineFunc(34332, kLotteryDispatch, 0, 0);
    }

    const int result = user->lottery_item(static_cast<int>(item_id));
    if (result <= 0) {
        const int last_lottery_time = user->getLastLotteryTime();
        if (last_lottery_time) {
            const unsigned int elapsed = static_cast<unsigned int>(
                GlobalData::system_time()->getCurSec() - last_lottery_time);
            if (elapsed <= 2u) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x334u),
                    1u,
                    0u,
                    0u);
            }
        }
        user->setLastLotteryTime(
            static_cast<unsigned long>(GlobalData::system_time()->getCurSec()));
        return 0;
    }

    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_USE_LOTTERY_ITEM);
    reply.put_byte(0);
    reply.put_byte(result);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x8233276
// IDA_NAME: _ZN18DisPatcher_Lottery4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_Lottery::read(PacketBuf &,MSG_BASE &)
int DisPatcher_Lottery::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233280
// IDA_NAME: _ZN18DisPatcher_Lottery7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_Lottery::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_Lottery::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859694C
// IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryC2Ev
// DEMANGLED: DisPatcher_MultiBoxLottery::DisPatcher_MultiBoxLottery(void)
DisPatcher_MultiBoxLottery::DisPatcher_MultiBoxLottery()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241F14
// IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryD2Ev
// DEMANGLED: DisPatcher_MultiBoxLottery::~DisPatcher_MultiBoxLottery()
// The non-deleting destructor is emitted from the definition below.
// IDA_EA: 0x8241F44
// IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryD0Ev
// DEMANGLED: DisPatcher_MultiBoxLottery::~DisPatcher_MultiBoxLottery()
// The deleting destructor is the compiler-emitted companion of that same body.
DisPatcher_MultiBoxLottery::~DisPatcher_MultiBoxLottery() {}

// IDA_EA: 0x820B802
// IDA_NAME: _ZN26DisPatcher_MultiBoxLottery12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_MultiBoxLottery::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_MultiBoxLottery::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || user->CheckInTrade()) {
        return 0;
    }

    ServiceRestrictManager* const restrictions =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrictions->isRestricted(user, RestrictCategory::AuthMobile, 18)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_RANDOMBOX_ITEM, 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(24));
    if (security_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_RANDOMBOX_ITEM,
            static_cast<unsigned char>(security_error));
        return 0;
    }

    unsigned short item_id = 0;
    if (packet.get_short(item_id) != 1) {
        return LineFunc(43064, kMultiBoxLotteryDispatch, 0, 0);
    }

    unsigned short count = 0;
    if (packet.get_short(count) != 1) {
        return LineFunc(43065, kMultiBoxLotteryDispatch, 0, 0);
    }
    const int result = user->multibox_lottery_item(
        static_cast<int>(item_id), static_cast<int>(count));
    if (result > 0) {
        PacketGuard reply;
        reply.put_header(1, ENUM_CMDPACKET_USE_RANDOMBOX_ITEM);
        reply.put_byte(0);
        reply.put_byte(result);
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x8233BD2
// IDA_NAME: _ZN26DisPatcher_MultiBoxLottery4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_MultiBoxLottery::read(PacketBuf &,MSG_BASE &)
int DisPatcher_MultiBoxLottery::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233BDC
// IDA_NAME: _ZN26DisPatcher_MultiBoxLottery7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_MultiBoxLottery::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_MultiBoxLottery::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596FA4
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardC2Ev
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::Dispatcher_RequestOnTimeEventReward(void)
Dispatcher_RequestOnTimeEventReward::Dispatcher_RequestOnTimeEventReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240A0E
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardD2Ev
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::~Dispatcher_RequestOnTimeEventReward()
// The non-deleting destructor is emitted from the definition below.
// IDA_EA: 0x8240A3E
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardD0Ev
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::~Dispatcher_RequestOnTimeEventReward()
// The deleting destructor is the compiler-emitted companion of that same body.
Dispatcher_RequestOnTimeEventReward::~Dispatcher_RequestOnTimeEventReward() {}

// IDA_EA: 0x822458C
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestOnTimeEventReward::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;

    if (G_GameWorld()->IsPVPChannel()) {
        return 0;
    }
    if (user->get_state() != 3) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                                 216);
        return 0;
    }

    COnTimeEvent* const repeat_event = static_cast<COnTimeEvent*>(
        GlobalData::s_event_manager->GetRepeatEvent(51));
    if (!repeat_event) {
        return 0;
    }

    if (repeat_event->IsEventing(0) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                                 19);

        char log_prefix[256] = {};
        std::sprintf(log_prefix,
                     "./log/%s/OnTimeEvent",
                     G_CEnvironment()->get_file_name());
        const int charac_no = user->get_charac_no(-1);
        const char* const account_id =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
        CMyFileLog log(kOnTimeRewardDispatch, 57089);
        log(log_prefix,
            "On Time Event:Request Reward IsEventing error m_id(%s) charac_no(%u)",
            account_id,
            static_cast<unsigned int>(charac_no));
        return 0;
    }

    if (repeat_event->get_reward_state()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                                 19);

        char log_prefix[256] = {};
        std::sprintf(log_prefix,
                     "./log/%s/OnTimeEvent",
                     G_CEnvironment()->get_file_name());
        const int charac_no = user->get_charac_no(-1);
        const char* const account_id =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
        CMyFileLog log(kOnTimeRewardDispatch, 57108);
        log(log_prefix,
            "On Time Event:Request Reward ENUM_ONTIME_EVENT_ON_REWARD_ON error m_id(%s) charac_no(%u)",
            account_id,
            static_cast<unsigned int>(charac_no));
        return 0;
    }

    if (user->get_ontime_last_recv_idx()) {
        repeat_event->insert_reward_user(user);
        return 0;
    }

    user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD, 2);

    char log_prefix[256] = {};
    std::sprintf(log_prefix,
                 "./log/%s/OnTimeEvent",
                 G_CEnvironment()->get_file_name());
    const int charac_no = user->get_charac_no(-1);
    const char* const account_id =
        NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
    CMyFileLog log(kOnTimeRewardDispatch, 57120);
    log(log_prefix,
        "On Time Event:Request Reward user db error m_id(%s) charac_no(%u)",
        account_id,
        static_cast<unsigned int>(charac_no));
    return 0;
}

// IDA_EA: 0x82340BE
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestOnTimeEventReward::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82340C8
// IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestOnTimeEventReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestOnTimeEventReward::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597A46
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardC2Ev
// DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::Dispatcher_Exchange_Random_Item_Reward(void)
Dispatcher_Exchange_Random_Item_Reward::
    Dispatcher_Exchange_Random_Item_Reward()
    : IPacketDispatcher<MSG_EXCHANGE_RANDOM_ITEM_REWARD,
                        stExchangeRandomItemReward,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244578
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardD2Ev
// IDA_EA: 0x82445A8
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardD0Ev
// DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::~Dispatcher_Exchange_Random_Item_Reward()
Dispatcher_Exchange_Random_Item_Reward::
    ~Dispatcher_Exchange_Random_Item_Reward() {}

// IDA_EA: 0x81DE406
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    MSG_EXCHANGE_RANDOM_ITEM_REWARD& request =
        static_cast<MSG_EXCHANGE_RANDOM_ITEM_REWARD&>(message);
    if (packet.get_short(request.event_id_0e) != 1) {
        return LineFunc(20424,
                        "virtual int Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }

    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(20428,
                        "virtual int Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    for (uint8_t index = 0; index < count; ++index) {
        uint16_t item_slot = 0;
        if (packet.get_short(item_slot) != 1) {
            return LineFunc(20434,
                            "virtual int Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf&, MSG_BASE&)",
                            0,
                            0);
        }
        request.item_indices_10.push_back(static_cast<int>(item_slot));
    }
    return 0;
}

// IDA_EA: 0x81DE52C
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Exchange_Random_Item_Reward::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& param) {
    const MSG_EXCHANGE_RANDOM_ITEM_REWARD& request =
        static_cast<const MSG_EXCHANGE_RANDOM_ITEM_REWARD&>(message);
    stExchangeRandomItemReward& result =
        static_cast<stExchangeRandomItemReward&>(param);

    if (!user || user->get_state() != 3) {
        result.dword_04 = 7;
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* const event_manager =
        data_manager ? data_manager->get_event_script_mng() : 0;
    EventClassify::CExchangeRandomItemReward* const event =
        event_manager
            ? static_cast<EventClassify::CExchangeRandomItemReward*>(
                  event_manager->get_event_entity(request.event_id_0e))
            : 0;
    if (!event || event->is_eventing() != 1) {
        result.dword_04 = 178;
        return 0;
    }

    std::vector<std::pair<int, int> > material_slots;
    result.dword_04 = event->verifyMaterialItemByRule(
        user, request.item_indices_10, material_slots);
    if (result.dword_04) {
        return 0;
    }

    std::vector<std::pair<int, int> > rewards;
    result.dword_04 = event->getRewardItem(rewards);
    if (result.dword_04) {
        return 0;
    }
    result.dword_04 = event->deleteUsedMaterialItem(
        user, material_slots, result.reward_pairs_0c);
    if (result.dword_04) {
        return 0;
    }
    result.dword_04 = event->sendRandomItemReward(
        user, rewards, result.item_indices_18, result.byte_09);
    return 0;
}

// IDA_EA: 0x81DE72C
// IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::send(CUser *,ParamBase &)
void Dispatcher_Exchange_Random_Item_Reward::send(CUser* user,
                                                   ParamBase& param) {
    const stExchangeRandomItemReward& result =
        static_cast<const stExchangeRandomItemReward&>(param);
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_EXCHANGE_RANDOM_ITEM_REWARD);
    if (result.dword_04) {
        reply.put_byte(0);
        reply.put_byte(static_cast<uint8_t>(result.dword_04));
    } else {
        reply.put_byte(1);
        reply.put_byte(static_cast<uint8_t>(result.reward_pairs_0c.size()));
        for (std::vector<std::pair<int, int> >::const_iterator reward =
                 result.reward_pairs_0c.begin();
             reward != result.reward_pairs_0c.end();
             ++reward) {
            reply.put_int(reward->first);
            reply.put_int(reward->second);
        }
        reply.put_byte(result.byte_09);
        if (result.byte_09) {
            // The source protocol stores four values per successful mail:
            // slot, item id, add_info, and avatar selector.
            for (std::vector<int>::const_iterator it =
                     result.item_indices_18.begin();
                 it != result.item_indices_18.end();) {
                if (static_cast<size_t>(result.item_indices_18.end() - it) <
                    4u) {
                    break;
                }
                reply.put_short(static_cast<uint16_t>(*it++));
                reply.put_int(*it++);
                reply.put_int(*it++);
                reply.put_short(static_cast<uint16_t>(*it++));
            }
        }
    }
    reply.finalize(true);
    user->Send(reply);
}

// IDA_EA: 0x8597A62
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardC2Ev
// DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::Dispatcher_Avatar_Disjoint_Random_Reward(void)
Dispatcher_Avatar_Disjoint_Random_Reward::
    Dispatcher_Avatar_Disjoint_Random_Reward()
    : IPacketDispatcher<MSG_AVATAR_DISJOINT_RANDOM_REWARD,
                        stAvatarDisjointRandomReward,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824452A
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardD2Ev
// IDA_EA: 0x824455A
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardD0Ev
// DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::~Dispatcher_Avatar_Disjoint_Random_Reward()
Dispatcher_Avatar_Disjoint_Random_Reward::
    ~Dispatcher_Avatar_Disjoint_Random_Reward() {}

// IDA_EA: 0x81DE9DA
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Avatar_Disjoint_Random_Reward::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    MSG_AVATAR_DISJOINT_RANDOM_REWARD& request =
        static_cast<MSG_AVATAR_DISJOINT_RANDOM_REWARD&>(message);
    // 140828 sub_8E7D20 writes exactly {u16 avatar_slot,u32 item_id}.
    if (packet.get_short(request.avatar_slot_0e) != 1 ||
        packet.get_int(request.avatar_item_id_10) != 1) {
        return LineFunc(
            20545,
            "virtual int Dispatcher_Avatar_Disjoint_Random_Reward::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    return 0;
}

// IDA_EA: 0x81DEA06
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Avatar_Disjoint_Random_Reward::process(CUser* user,
                                                       MSG_BASE& message,
                                                       ParamBase& param) {
    const MSG_AVATAR_DISJOINT_RANDOM_REWARD& request =
        static_cast<const MSG_AVATAR_DISJOINT_RANDOM_REWARD&>(message);
    stAvatarDisjointRandomReward& result =
        static_cast<stAvatarDisjointRandomReward&>(param);
    if (!user || user->get_state() != 3) {
        result.error_code = 7;
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* const event_manager =
        data_manager ? data_manager->get_event_script_mng() : 0;
    EventClassify::CAvatarDisjointRandomReward* const event =
        event_manager
            ? static_cast<EventClassify::CAvatarDisjointRandomReward*>(
                  event_manager->get_event_entity(407))
            : 0;
    if (!event || event->is_eventing() != 1) {
        result.error_code = 178;
        return 0;
    }

    const Inven_Item avatar_item = user->getCurCharacInvenR()->GetInvenSlot(
        2, static_cast<int>(request.avatar_slot_0e));
    if (avatar_item.item_id_ != static_cast<int>(request.avatar_item_id_10)) {
        result.error_code = 17;
        return 0;
    }
    CItem* const item_proto = data_manager->find_item(avatar_item.item_id_);
    if (!item_proto) {
        result.error_code = 17;
        return 0;
    }

    result.error_code = event->checkAvatarDisjointError(
        user, request.avatar_slot_0e, avatar_item, item_proto);
    if (result.error_code) {
        return 0;
    }
    result.error_code = event->deleteAvatarItem(
        user, request.avatar_slot_0e, avatar_item, item_proto);
    if (result.error_code) {
        return 0;
    }
    result.error_code = event->getRewardItem(item_proto, result.reward_pairs_0c);
    if (result.error_code) {
        return 0;
    }
    (void)event->SendCreateEmblemStatistic(result.reward_pairs_0c);
    result.error_code = event->sendRandomItemReward(user, result.reward_pairs_0c);
    if (!result.error_code) {
        result.avatar_slot = request.avatar_slot_0e;
    }
    return 0;
}

// IDA_EA: 0x81DEC56
// IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::send(CUser *,ParamBase &)
void Dispatcher_Avatar_Disjoint_Random_Reward::send(CUser* user,
                                                     ParamBase& param) {
    const stAvatarDisjointRandomReward& result =
        static_cast<const stAvatarDisjointRandomReward&>(param);
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_AVATAR_DISJOINT_RANDOM_REWARD);
    if (result.error_code) {
        reply.put_byte(0);
        reply.put_byte(static_cast<uint8_t>(result.error_code));
    } else {
        reply.put_byte(1);
        reply.put_short(result.avatar_slot);
        reply.put_byte(static_cast<uint8_t>(result.reward_pairs_0c.size()));
        for (std::vector<std::pair<int, int> >::const_iterator reward =
                 result.reward_pairs_0c.begin();
             reward != result.reward_pairs_0c.end();
             ++reward) {
            reply.put_int(reward->first);
            reply.put_int(reward->second);
        }
    }
    reply.finalize(true);
    user->Send(reply);
}

// One explicit owner for each recovered dispatcher template/vtable.  These
// do not register either handler with the unavailable 606-slot root.
template class IPacketDispatcher<MSG_EXCHANGE_RANDOM_ITEM_REWARD,
                                 stExchangeRandomItemReward,
                                 static_cast<ch_state>(0)>;
template class IPacketDispatcher<MSG_AVATAR_DISJOINT_RANDOM_REWARD,
                                 stAvatarDisjointRandomReward,
                                 static_cast<ch_state>(0)>;
