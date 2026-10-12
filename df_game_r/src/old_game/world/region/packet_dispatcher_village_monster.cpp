#include "old_game/world/region/packet_dispatcher_village_monster.h"
#include "old_game/world/region/inter_village_attacked_runtime.h"

#include <algorithm>
#include <cstring>
#include <stddef.h>
#include <stdint.h>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/db_types.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8596E1C
// IDA_NAME: _ZN30Dispatcher_FightVillageMonsterC2Ev
// DEMANGLED: Dispatcher_FightVillageMonster::Dispatcher_FightVillageMonster(void)
Dispatcher_FightVillageMonster::Dispatcher_FightVillageMonster() {}

// IDA_EA: 0x8240E04
// IDA_NAME: _ZN30Dispatcher_FightVillageMonsterD2Ev
// DEMANGLED: Dispatcher_FightVillageMonster::~Dispatcher_FightVillageMonster()
// IDA_EA: 0x8240E34
// IDA_NAME: _ZN30Dispatcher_FightVillageMonsterD0Ev
// DEMANGLED: Dispatcher_FightVillageMonster::~Dispatcher_FightVillageMonster()
Dispatcher_FightVillageMonster::~Dispatcher_FightVillageMonster() {}

// IDA_EA: 0x82226C0
// IDA_NAME: _ZN30Dispatcher_FightVillageMonster12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_FightVillageMonster::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_FightVillageMonster::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }
    // Old IDA reads CEnvironment+0x1A8 twice.  That is the proven
    // Server_Envir::server_type field exposed by this typed accessor.
    if (G_CEnvironment()->get_server_type() == 1 ||
        G_CEnvironment()->get_server_type() == 2) {
        return 0;
    }
    if (!user) {
        return LineFunc(
            56344,
            "virtual int Dispatcher_FightVillageMonster::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->get_state() != 3) {
        return 0;
    }

    uint16_t monster_uid = 0;
    if (packet.get_short(monster_uid) != 1) {
        return LineFunc(
            56358,
            "virtual int Dispatcher_FightVillageMonster::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CParty* party = user->GetParty();
    if (!party) {
        party = G_CGameManager()->GetParty();
        if (party) {
            party->set_single_play(user);
        }
    }
    if (party) {
        const int result =
            party->OnFightVillageMonster(user, static_cast<int>(monster_uid));
        if (result) {
            user->SendCmdErrorPacket(
                ENUM_CMDPACKET_FIGHT_VILLAGE_MONSTER,
                static_cast<unsigned char>(result));
        }
    }
    return 0;
}

// IDA_EA: 0x8233FBA
// IDA_NAME: _ZN30Dispatcher_FightVillageMonster4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_FightVillageMonster::read(PacketBuf &,MSG_BASE &)
int Dispatcher_FightVillageMonster::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233FC4
// IDA_NAME: _ZN30Dispatcher_FightVillageMonster7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_FightVillageMonster::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_FightVillageMonster::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596E38
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFightingC2Ev
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::Dispatcher_FinishVillageMonsterFighting(void)
Dispatcher_FinishVillageMonsterFighting::Dispatcher_FinishVillageMonsterFighting() {}

// IDA_EA: 0x8240DB6
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFightingD2Ev
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::~Dispatcher_FinishVillageMonsterFighting()
// IDA_EA: 0x8240DE6
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFightingD0Ev
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::~Dispatcher_FinishVillageMonsterFighting()
Dispatcher_FinishVillageMonsterFighting::~Dispatcher_FinishVillageMonsterFighting() {}

// IDA_EA: 0x822284A
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFighting12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_FinishVillageMonsterFighting::dispatch_sig(CUser* user,
                                                          PacketBuf& packet) {
    (void)packet;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }
    // The fight-village-monster path above uses the same original +0x1A8
    // server_type gate.
    if (G_CEnvironment()->get_server_type() == 1 ||
        G_CEnvironment()->get_server_type() == 2) {
        return 0;
    }
    if (!user) {
        return LineFunc(
            56422,
            "virtual int Dispatcher_FinishVillageMonsterFighting::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->get_state() != 5) {
        return 0;
    }

    CParty* const party = user->GetParty();
    if (party) {
        const int result = party->OnFinishVillageMonsterFighting(user, false);
        if (result) {
            user->SendCmdErrorPacket(
                ENUM_CMDPACKET_FINISH_VILLAGE_MONSTER_FIGHTING,
                static_cast<unsigned char>(result));
        }
    }
    return 0;
}

// IDA_EA: 0x8233FCE
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFighting4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::read(PacketBuf &,MSG_BASE &)
int Dispatcher_FinishVillageMonsterFighting::read(PacketBuf& packet,
                                                   MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233FD8
// IDA_NAME: _ZN39Dispatcher_FinishVillageMonsterFighting7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_FinishVillageMonsterFighting::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_FinishVillageMonsterFighting::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

#pragma pack(push, 1)
struct FullLevelBroadcastPayload {
    uint8_t routing_envelope[10]; // +0x00
    char text[1];                 // +0x0a, NUL-terminated
};
#pragma pack(pop)

static_assert(offsetof(FullLevelBroadcastPayload, text) == 0x0au,
              "full-level broadcast text moved from the 0627 ABI");

// IDA data: _ZN10GlobalData19s_villageMonsterMgrE.  GlobalData's public
// header intentionally does not expose this C-owned concrete type, so keep
// the original process-global landing typed at this gameplay boundary.

int ReadVillageAttackedReplyInt(const char* payload, unsigned int offset) {
    int value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t ReadVillageAttackedReplyUInt(const char* payload,
                                      unsigned int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

void WriteVillageAttackedPacketInt(char* body,
                                   unsigned int offset,
                                   int value) {
    std::memcpy(body + offset, &value, sizeof(value));
}

bool IsVillageAttackedChannelEnabled() {
    return !G_GameWorld()->IsIntegratedPvPBaseChannel() &&
           G_CEnvironment()->get_server_type() != 1 &&
           G_CEnvironment()->get_server_type() != 2;
}

}  // namespace

// IDA_EA: 0x84EE500
// IDA_NAME: _ZN20Inter_TakeScreenShotC2Ev
// DEMANGLED: Inter_TakeScreenShot::Inter_TakeScreenShot(void)
Inter_TakeScreenShot::Inter_TakeScreenShot() : ISigDispatcher() {}

// IDA_EA: 0x84F7482
// IDA_NAME: _ZN20Inter_TakeScreenShotD2Ev
// DEMANGLED: Inter_TakeScreenShot::~Inter_TakeScreenShot()
// IDA_EA: 0x84F74B2
// IDA_NAME: _ZN20Inter_TakeScreenShotD0Ev
Inter_TakeScreenShot::~Inter_TakeScreenShot() {}

// IDA_EA: 0x84DF3D6
// IDA_NAME: _ZN20Inter_TakeScreenShot12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TakeScreenShot::dispatch_sig(CUser *,char *,int)
int Inter_TakeScreenShot::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)user;
    (void)payload_size;

    PacketGuard packet;
    packet.put_header(0, 244);
    packet.put_int(ReadVillageAttackedReplyInt(payload, 11));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EE51C
// IDA_NAME: _ZN26Inter_VillageAttackedStartC2Ev
// DEMANGLED: Inter_VillageAttackedStart::Inter_VillageAttackedStart(void)
Inter_VillageAttackedStart::Inter_VillageAttackedStart()
    : ISigDispatcher() {}

// IDA_EA: 0x84F7434
// IDA_NAME: _ZN26Inter_VillageAttackedStartD2Ev
// DEMANGLED: Inter_VillageAttackedStart::~Inter_VillageAttackedStart()
// IDA_EA: 0x84F7464
// IDA_NAME: _ZN26Inter_VillageAttackedStartD0Ev
Inter_VillageAttackedStart::~Inter_VillageAttackedStart() {}

// IDA_EA: 0x84DF47A
// IDA_NAME: _ZN26Inter_VillageAttackedStart12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedStart::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedStart::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)user;
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled()) {
        return 0;
    }

    ServerParameterScript* const server_parameter =
        G_CDataManager()->get_server_parameter_script();
    if (server_parameter->isDungeonOpen(255)) {
        server_parameter->setDungeonOpen(255, false);
        G_GameWorld()->send_all_dungeon_inout_message(11007, false);
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 241);
    packet.put_int(0);
    packet.put_int(ReadVillageAttackedReplyInt(payload, 10));
    packet.put_int(ReadVillageAttackedReplyInt(payload, 14));
    packet.put_int(ReadVillageAttackedReplyInt(payload, 18));
    packet.put_int(0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    GlobalData::s_villageMonsterMgr->OnStartVillageAttacked();
    return 0;
}

// IDA_EA: 0x84EE538
// IDA_NAME: _ZN24Inter_VillageAttackedEndC2Ev
// DEMANGLED: Inter_VillageAttackedEnd::Inter_VillageAttackedEnd(void)
Inter_VillageAttackedEnd::Inter_VillageAttackedEnd() : ISigDispatcher() {}

// IDA_EA: 0x84F73E6
// IDA_NAME: _ZN24Inter_VillageAttackedEndD2Ev
// DEMANGLED: Inter_VillageAttackedEnd::~Inter_VillageAttackedEnd()
// IDA_EA: 0x84F7416
// IDA_NAME: _ZN24Inter_VillageAttackedEndD0Ev
Inter_VillageAttackedEnd::~Inter_VillageAttackedEnd() {}

// IDA_EA: 0x84DF6A4
// IDA_NAME: _ZN24Inter_VillageAttackedEnd12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedEnd::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedEnd::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    (void)user;
    (void)payload_size;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }

    const uint32_t current_seconds =
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec());
    const uint32_t close_time = ReadVillageAttackedReplyUInt(payload, 10);
    const int remaining_seconds = close_time < current_seconds
        ? 0
        : static_cast<int>(close_time - current_seconds);

    const uint32_t result_value = ReadVillageAttackedReplyUInt(payload, 18);
    const uint32_t compare_value = ReadVillageAttackedReplyUInt(payload, 14);
    const int reward_type = result_value > compare_value ? 2 : 1;
    if (reward_type == 2) {
        GlobalData::s_revengeDungeonMgr->CloseRevengeDungeon();
    } else {
        GlobalData::s_revengeDungeonMgr->OpenRevengeDungeon();
    }
    GlobalData::s_revengeDungeonMgr->SetCloseTime(close_time);

    G_GameWorld()->ProcessAllUsers(
        [remaining_seconds, close_time, reward_type](CUser* target) {
            if (!target) {
                return;
            }

            char response[10];
            std::memset(response, 0, sizeof(response));
            PacketGuard packet;
            packet.clear();
            packet.put_header(0, ENUM_NOTIPACKET_VILLAGE_ATTACKED_COMBINE);
            target->VillageAttack_NotDBUpdate();

            if (reward_type == 1) {
                // The original call site has no decoded AP arguments.  The
                // restored target body is itself empty, so no action-group
                // values are invented here.
                response[0] = 0;
                response[1] = 0;
                WriteVillageAttackedPacketInt(response, 2, remaining_seconds);

                const int attack_count = target->GetCurVillageAttackCount();
                const int state = target->get_state();
                if (state > 2) {
                    target->setCurCharacFatigue(
                        std::max(0, target->getCurCharacFatigue() - 5));
                    target->SendFatigue();
                }
                if (state != 3) {
                    return;
                }

                if (attack_count > 2) {
                    WriteVillageAttackedPacketInt(response, 6, 7745);
                }

                const ConditionEnterRevengeVector& conditions =
                    G_CDataManager()->get_server_parameter_script()
                        ->get_condition_enter_revenge();
                for (ConditionEnterRevengeVector::const_iterator it =
                         conditions.begin();
                     it != conditions.end();
                     ++it) {
                    if (attack_count >= it->minimum_attack_count &&
                        attack_count <= it->maximum_attack_count) {
                        target->SetCurRevengeDungeonCount(
                            static_cast<uint8_t>(it->revenge_dungeon_count));
                        target->VillageAttack_DBUpdate();
                        break;
                    }
                }
            } else {
                // The old loop closes this process-global state again for
                // each non-null world user before emitting the result packet.
                GlobalData::s_revengeDungeonMgr->CloseRevengeDungeon();
                GlobalData::s_revengeDungeonMgr->SetCloseTime(close_time);
                response[0] = 1;
                response[1] = 0;
                WriteVillageAttackedPacketInt(response, 2, remaining_seconds);
                WriteVillageAttackedPacketInt(response, 6, 0);
            }

            packet.put_binary(response, sizeof(response));
            packet.finalize(true);
            target->SetVillageAttackedReward(false);
            target->Send(packet);
        });

    GlobalData::s_villageMonsterMgr->OnDestroyVillageMonster(
        static_cast<ENUM_VILLAGE_ATTACKED_REWARD>(reward_type));
    return 0;
}

// IDA_EA: 0x84EE554
// IDA_NAME: _ZN30Inter_VillageAttackedCountdownC2Ev
// DEMANGLED: Inter_VillageAttackedCountdown::Inter_VillageAttackedCountdown(void)
Inter_VillageAttackedCountdown::Inter_VillageAttackedCountdown()
    : ISigDispatcher() {}

// IDA_EA: 0x84F7398
// IDA_NAME: _ZN30Inter_VillageAttackedCountdownD2Ev
// DEMANGLED: Inter_VillageAttackedCountdown::~Inter_VillageAttackedCountdown()
// IDA_EA: 0x84F73C8
// IDA_NAME: _ZN30Inter_VillageAttackedCountdownD0Ev
Inter_VillageAttackedCountdown::~Inter_VillageAttackedCountdown() {}

// IDA_EA: 0x84DFAE0
// IDA_NAME: _ZN30Inter_VillageAttackedCountdown12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedCountdown::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedCountdown::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int payload_size) {
    (void)user;
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled()) {
        return 0;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 241);
    packet.put_int(ReadVillageAttackedReplyInt(payload, 10));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EE570
// IDA_NAME: _ZN27Inter_VillageAttackedUpdateC2Ev
// DEMANGLED: Inter_VillageAttackedUpdate::Inter_VillageAttackedUpdate(void)
Inter_VillageAttackedUpdate::Inter_VillageAttackedUpdate()
    : ISigDispatcher() {}

// IDA_EA: 0x84F734A
// IDA_NAME: _ZN27Inter_VillageAttackedUpdateD2Ev
// DEMANGLED: Inter_VillageAttackedUpdate::~Inter_VillageAttackedUpdate()
// IDA_EA: 0x84F737A
// IDA_NAME: _ZN27Inter_VillageAttackedUpdateD0Ev
Inter_VillageAttackedUpdate::~Inter_VillageAttackedUpdate() {}

// IDA_EA: 0x84DFBE6
// IDA_NAME: _ZN27Inter_VillageAttackedUpdate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedUpdate::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedUpdate::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)user;
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled()) {
        return 0;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 247);
    packet.put_int(ReadVillageAttackedReplyInt(payload, 10));
    packet.put_int(ReadVillageAttackedReplyInt(payload, 14));
    packet.put_int(ReadVillageAttackedReplyInt(payload, 18));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EE58C
// IDA_NAME: _ZN26Inter_VillageAttackedScoreC2Ev
// DEMANGLED: Inter_VillageAttackedScore::Inter_VillageAttackedScore(void)
Inter_VillageAttackedScore::Inter_VillageAttackedScore()
    : ISigDispatcher() {}

// IDA_EA: 0x84F72FC
// IDA_NAME: _ZN26Inter_VillageAttackedScoreD2Ev
// DEMANGLED: Inter_VillageAttackedScore::~Inter_VillageAttackedScore()
// IDA_EA: 0x84F732C
// IDA_NAME: _ZN26Inter_VillageAttackedScoreD0Ev
Inter_VillageAttackedScore::~Inter_VillageAttackedScore() {}

// IDA_EA: 0x84DFD16
// IDA_NAME: _ZN26Inter_VillageAttackedScore12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedScore::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedScore::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled()) {
        return 0;
    }
    if (user->getCurCharacR() &&
        user->get_charac_no(-1) == ReadVillageAttackedReplyInt(payload, 14)) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 248);
        packet.put_int(ReadVillageAttackedReplyInt(payload, 18));
        packet.put_int(ReadVillageAttackedReplyInt(payload, 22));
        packet.put_int(ReadVillageAttackedReplyInt(payload, 26));
        packet.put_int(ReadVillageAttackedReplyInt(payload, 30));
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE5A8
// IDA_NAME: _ZN27Inter_VillageAttackedRewardC2Ev
// DEMANGLED: Inter_VillageAttackedReward::Inter_VillageAttackedReward(void)
Inter_VillageAttackedReward::Inter_VillageAttackedReward()
    : ISigDispatcher() {}

// IDA_EA: 0x84F72AE
// IDA_NAME: _ZN27Inter_VillageAttackedRewardD2Ev
// DEMANGLED: Inter_VillageAttackedReward::~Inter_VillageAttackedReward()
// IDA_EA: 0x84F72DE
// IDA_NAME: _ZN27Inter_VillageAttackedRewardD0Ev
Inter_VillageAttackedReward::~Inter_VillageAttackedReward() {}

// IDA_EA: 0x84E0390
// IDA_NAME: _ZN27Inter_VillageAttackedReward17_send_reward_itemEP5CUsermii
// DEMANGLED: Inter_VillageAttackedReward::_send_reward_item(CUser *,unsigned long,int,int)
int Inter_VillageAttackedReward::_send_reward_item(CUser* user,
                                                   unsigned long item_index,
                                                   int item_count,
                                                   int reward_value) {
    (void)reward_value;

    Inven_Item item;
    item.item_id_ = static_cast<int32_t>(item_index);
    G_CDataManager()->get_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, item, item_count);
    if (item.item_id_ == -1) {
        return 0;
    }
    if (item.item_id_ == 1) {
        item.avatar_select_ = 1;
    }

    char title[30] = {};
    char text[256] = {};
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_124", 0),
                 29);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_125", 0),
                 255);
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        12,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

// IDA_EA: 0x84DFEA6
// IDA_NAME: _ZN27Inter_VillageAttackedReward12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedReward::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedReward::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled() || !user ||
        !user->getCurCharacR() ||
        user->get_charac_no(-1) != ReadVillageAttackedReplyInt(payload, 14)) {
        return 0;
    }

    const int reward_value = ReadVillageAttackedReplyInt(payload, 18);
    switch (ReadVillageAttackedReplyInt(payload, 22)) {
    case 5: {
        Arad_VillageAttackRewardItem::Data reward(reward_value, 0, 0);
        Arad_VillageAttackRewardItem* const table =
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()
                ->getAradVillageAttackReward();
        if (table->find_reward_item(reward_value, reward)) {
            user->SetVillageAttackedReward(true);
            _send_reward_item(user,
                              reward.item_index,
                              reward.item_count,
                              reward.win_point);
        }
        break;
    }
    case 6: {
        Arad_VillageAttackRewardItem::RankData reward;
        Arad_VillageAttackRewardItem* const table =
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()
                ->getAradVillageAttackReward();
        if (!table->find_ranking_reward_item(
                static_cast<short>(reward_value), reward)) {
            return 0;
        }
        const unsigned long item_indexes[2] = {
            reward.item_index, reward.item_index2
        };
        const int item_counts[2] = {reward.item_count, reward.item_count2};
        for (int index = 0; index <= 1; ++index) {
            if (item_indexes[index]) {
                _send_reward_item(user,
                                  item_indexes[index],
                                  item_counts[index],
                                  reward_value);
            }
        }
        break;
    }
    case 3: {
        user->SetVillageAttackedReward(true);
        Inven_Item item;
        item.item_id_ = 1;
        G_CDataManager()->get_item_list()->create_item(
            static_cast<ENUM_ITEM_CREATE_TYPE>(3), item, 1);
        item.avatar_select_ = 1;
        if (item.item_id_ == -1) {
            return 0;
        }

        char title[21] = {};
        char text[256] = {};
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_124", 0),
                     20);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_125", 0),
                     255);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            text,
            static_cast<int>(std::strlen(text)),
            30,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
            false,
            false);
        break;
    }
    }
    return 0;
}

// IDA_EA: 0x84EE5C4
// IDA_NAME: _ZN33Inter_VillageAttackedRewardServerC2Ev
// DEMANGLED: Inter_VillageAttackedRewardServer::Inter_VillageAttackedRewardServer(void)
Inter_VillageAttackedRewardServer::Inter_VillageAttackedRewardServer()
    : ISigDispatcher() {}

// IDA_EA: 0x84F7260
// IDA_NAME: _ZN33Inter_VillageAttackedRewardServerD2Ev
// DEMANGLED: Inter_VillageAttackedRewardServer::~Inter_VillageAttackedRewardServer()
// IDA_EA: 0x84F7290
// IDA_NAME: _ZN33Inter_VillageAttackedRewardServerD0Ev
Inter_VillageAttackedRewardServer::~Inter_VillageAttackedRewardServer() {}

// IDA_EA: 0x84E05C2
// IDA_NAME: _ZN33Inter_VillageAttackedRewardServer12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_VillageAttackedRewardServer::dispatch_sig(CUser *,char *,int)
int Inter_VillageAttackedRewardServer::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    if (!IsVillageAttackedChannelEnabled()) {
        return 0;
    }

    char response[10];
    std::memset(response, 0, sizeof(response));
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_VILLAGE_ATTACKED_COMBINE);
    const int reward_type = GlobalData::s_villageMonsterMgr->GetRewardType();
    if (reward_type == 1) {
        response[0] = 0;
    } else if (reward_type == 2) {
        response[0] = 1;
    }
    response[1] = 2;
    WriteVillageAttackedPacketInt(response, 2, 0);
    WriteVillageAttackedPacketInt(response, 6, 0);
    packet.put_binary(response, sizeof(response));
    packet.finalize(true);

    GlobalData::s_revengeDungeonMgr->CloseRevengeDungeon();
    GlobalData::s_revengeDungeonMgr->SetCloseTime(0);
    G_GameWorld()->ProcessAllUsers([&packet](CUser* target) {
        if (target && target->get_state() > 2) {
            target->SetCurCharacRevengeData(0, 0);
            target->VillageAttack_NotDBUpdate();
            target->Send(packet);
        }
    });
    GlobalData::s_villageMonsterMgr->OnEndRewardVillageMonster();
    return 0;
}

// IDA_EA: 0x84EE5E0
// IDA_NAME: _ZN24Inter_FullLevelBroadCastC2Ev
// DEMANGLED: Inter_FullLevelBroadCast::Inter_FullLevelBroadCast(void)
Inter_FullLevelBroadCast::Inter_FullLevelBroadCast() : ISigDispatcher() {}

// IDA_EA: 0x84F7212
// IDA_NAME: _ZN24Inter_FullLevelBroadCastD2Ev
// DEMANGLED: Inter_FullLevelBroadCast::~Inter_FullLevelBroadCast()
// IDA_EA: 0x84F7242
// IDA_NAME: _ZN24Inter_FullLevelBroadCastD0Ev
Inter_FullLevelBroadCast::~Inter_FullLevelBroadCast() {}

// IDA_EA: 0x84E0816
// IDA_NAME: _ZN24Inter_FullLevelBroadCast12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_FullLevelBroadCast::dispatch_sig(CUser *,char *,int)
int Inter_FullLevelBroadCast::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    (void)user;
    (void)payload_size;
    const FullLevelBroadcastPayload& input =
        *reinterpret_cast<const FullLevelBroadcastPayload*>(payload);

    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(6);
    packet.put_int(static_cast<int>(std::strlen(input.text)));
    packet.put_str(input.text,
                   static_cast<int>(std::strlen(input.text)));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}
