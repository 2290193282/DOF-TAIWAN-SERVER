#include "common/manager/game_manager.h"
#include "common/config.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/pvp_party/db_guild_exp_book_delete.h"
#include "old_game/activity/schedule/event_character_handler.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/activity/schedule/exp_keeping_event.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/activity/schedule/collect_box_dispatcher.h"
#include "old_game/activity/schedule/packet_dispatcher_event_all_user_gift.h"
#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"
#include "old_game/activity/schedule/glass_bottle_event.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/sea_bottom_event.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/social/guild/inter_guild_member_return_dispatchers.h"
#include "old_game/pvp/tournament/pvp_tournament.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/achievement_runtime.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_game/world/region/world_area_allocators.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>
#include <stdint.h>

namespace {

uint8_t HasUserEventCharacterParty(CParty* party) {
    if (!party) return 0u;
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member && member->getUserEventCharacterFlag() != 0) return 1u;
    }
    return 0u;
}

}  // namespace
#include <utility>
#include <vector>

// IDA_EA: 0x8296D30
// IDA_NAME: _ZN12CGameManager17insert_game_worldEP5CUser
// DEMANGLED: CGameManager::insert_game_world(CUser *)
int CGameManager::insert_game_world(CUser* user) {
    if (!user) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf20,
            "insert_waiting_room pUser parameter is null.");
        return 0;
    }
    if (!user->getCurCharacR()) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf27,
            "Not selected character(a reason is fast returning to character screen on client)");
        return 0;
    }

    const int current_time = GlobalData::s_systemTime_.getCurSec();
    std::vector<std::pair<int, int> > reset_items;
    std::vector<std::pair<int, int> > changed_slots;
    if (Inter_MonitorGuildPointDel::getLastDeleteTime() <= current_time &&
        user->getCurCharacLastPlayTick() <
            Inter_MonitorGuildPointDel::getLastDeleteTime()) {
        reset_items.push_back(std::make_pair(3204, 0));
    }
    if (user->getCurCharacGuildSecede()) {
        reset_items.push_back(std::make_pair(1200, 0));
        user->setCurCharacGuildSecede(0);
        const ENUM_SERVER_GROUP server_group =
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
        const unsigned int charac_no =
            static_cast<unsigned int>(user->get_charac_no(-1));
        DB_GuildExpBookDelete::makeRequest(
            charac_no, 0, server_group);
    }
    user->ResetItemByScript(reset_items);

    const int daily_check_time = GlobalData::s_systemTime_.getCurSec();
    if (CheckDailyScheduleTime(
            G_CEnvironment()->get_fatigue_time(),
            static_cast<long>(user->getCurCharacLastPlayTick()),
            static_cast<long>(daily_check_time))) {
        std::vector<std::pair<int, int> > daily_changed_slots;
        user->deleteDailyStackableItem(
            daily_changed_slots,
            oldsrv::config::kMainInventoryQuickSlotFirst,
            oldsrv::config::kMainInventoryQuickSlotLast);
        user->deleteDailyStackableItem(
            daily_changed_slots,
            oldsrv::config::kMainInventoryConsumableSlotFirst,
            oldsrv::config::kMainInventoryConsumableSlotLast);
    }
    user->deleteSpecificItem(reset_items, changed_slots);

    if (user->send_itemspace(12) != 1) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf58,
            "CGameManager::insert_game_world, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_ACCOUNT_CARGO ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        return 0;
    }
    if (user->send_itemspace(0) != 1) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf5f,
            "CGameManager::insert_game_world, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_INVENTORY ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        return 0;
    }
    if (user->send_itemspace(1) != 1) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf64,
            "CGameManager::insert_game_world, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_AVATAR ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        return 0;
    }
    if (user->send_itemspace(2) != 1) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf69,
            "CGameManager::insert_game_world, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_CARGO ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        return 0;
    }
    if (user->send_itemspace(7) != 1) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "bool CGameManager::insert_game_world(CUser*)",
            0xf7a,
            "CGameManager::insert_game_world, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_CREATURE ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        return 0;
    }

    if (G_GameWorld()->GetChannelType() == 14) {
        user->set_charac_guildkey(0);
    }
    if (G_GameWorld()->reach_game_world(user) != 1) {
        const char* const account_id =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
        const int uid = user->get_unique_id();
        cMyTrace reach_trace(
            "bool CGameManager::insert_game_world(CUser*)", 4002, 0);
        reach_trace(
            "CGameManager::insert_game_world , !G_GameWorld()->reach_game_world( pUser ) , USER : %d(%s)",
            uid,
            account_id);
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 4);
        cMyTrace source_trace(
            "bool CGameManager::insert_game_world(CUser*)", 4011, 0);
        source_trace("from (%d) m_id[%s] uid[%d]", 1, account_id, uid);
        return 0;
    }

    if (user->getCurCharacEventCharacterGrowtype() > 0) {
        CEventCharacterHandlerInstance()->_makeEventCharacter(user, 40);
    }

    CExpandEquipslot* const expand_equip = static_cast<CExpandEquipslot*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
    if (G_GameWorld()->IsEquipSlotSwitchChannel()) {
        if (user->getCurChannelEquipslotSwitch()) {
            expand_equip->EquipslotSwitch(
                user,
                0,
                static_cast<char>(user->getCurChannelEquipslotSwitch()),
                static_cast<char>(user->getCurExpandEquipslotSwitch()));
        }
    } else if (user->getCurChannelEquipslotSwitch() == 1) {
        expand_equip->EquipslotSwitch(
            user,
            0,
            static_cast<char>(user->getCurChannelEquipslotSwitch()),
            static_cast<char>(user->getCurExpandEquipslotSwitch()));
    }

    if (G_GameWorld()->GetChannelType() == 15 ||
        G_GameWorld()->GetChannelType() == 16) {
        CMissionList_Charac* const mission_list =
            static_cast<CMissionList_Charac*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        mission_list->Update_EnterWorld_event(*user);
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    user->send_skill_info();

    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(1);
    packet.put_short(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);

    expand_equip->Send_Expand_Equip_Info(user);
    user->processReturnUserQuestAutoClear();
    user->send_clear_quest_list();
    packet.clear();
    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&packet));
    user->Send(packet);

    static_cast<CTitleBook*>(user->GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)))->sendList();
    static_cast<CAchievement*>(user->GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(15)))->sendList();
    static_cast<item_lock::CItemLock*>(user->GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)))->SendItemLockList(user);

    if (G_GameWorld()->GetChannelType() != 15) {
        SendPvpList(user);
    }
    SendWarRoomList(user);
    user->send_pvp_record();
    SendPartyList(user);

    if (static_cast<unsigned short>(user->getPunishTradeAlert()) == 231u ||
        static_cast<unsigned short>(user->getPunishTradeAlert()) == 232u) {
        packet.clear();
        packet.put_header(0, 255);
        packet.put_int(user->getPunishTradeAlert());
        packet.finalize(true);
        user->Send(packet);
    }

    packet.clear();
    packet.put_header(0, 124);
    packet.finalize(true);
    user->Send(packet);

    // The 0828 client treats NOTI124 as the end of the base world-entry
    // transaction.  Native feature packets sent before it can reach handlers
    // whose town-side objects do not exist yet (the observed first offenders
    // were NOTI518 and NOTI487), causing the client to reset the connection
    // while the Loading overlay is still active.  Queue every native session
    // only after the canonical completion packet has entered this user's TCP
    // stream so asynchronous DB replies cannot overtake the base handshake.
    upgrade_room::OnEnterWorld(user);
    event_attendance::OnWorldReady(user);
    event_all_user_gift::QueueEligibilityCheck(user);
    facebook_like::BeginSession(user);
    exp_keeping::BeginSession(user);
    pvp_tournament::BeginSession(user);
    zombie_mode::BeginUserSession(user);
    glass_bottle_event::BeginSession(user);
    blue_horse_event::BeginSession(user);
    sea_bottom_event::BeginSession(user);
    fatigue_acceleration::SendTownState(user);
    collect_box::BeginSession(user);

    user->GetPICSMap().clear();
    user->ReCalcChattingEmoticon();
    user->SendChattingEmoticon();

    Packet_Monitor_Char_Info char_info;
    char_info.account_id = static_cast<uint32_t>(user->get_acc_id());
    char_info.channel_no = static_cast<int8_t>(G_CEnvironment()->get_channel_no());
    char_info.channel_type = static_cast<uint8_t>(G_GameWorld()->GetChannelType());
    char_info.charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    char_info.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    char_info.charac_job = static_cast<int8_t>(user->get_charac_job());
    char_info.charac_grow_type =
        static_cast<int8_t>(user->getCurCharacGrowType());
    char_info.charac_level = static_cast<int16_t>(user->get_charac_level());
    char_info.charac_memberkey =
        static_cast<uint32_t>(user->get_charac_memberkey());
    const size_t charac_name_length = std::strlen(user->getCurCharacName());
    std::memcpy(char_info.charac_name,
                user->getCurCharacName(),
                charac_name_length);
    char_info.return_user_first_login =
        static_cast<uint8_t>(user->IsReturnUserFirstLogin());

    GlobalData::s_monitor_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendTcpPacket(reinterpret_cast<char*>(&char_info), sizeof(char_info));
    GlobalData::s_guild_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendTcpPacket(reinterpret_cast<char*>(&char_info), sizeof(char_info));

    g_schoolMgr.AddUser(user);
    if (user->get_charac_guildkey()) {
        g_guildAgitMgr.AllocGuildAgitArea(
            static_cast<uint32_t>(user->get_charac_guildkey()));
    }

    const int charac_no_timer_2 = user->getCurCharacNo();
    const int login_tick_timer_2 = user->GetLoginTick();
    const int uid_timer_2 = user->GetUID();
    G_TimerQueue()->InsertTimer(0,
                                uid_timer_2,
                                2,
                                60,
                                static_cast<unsigned int>(login_tick_timer_2),
                                charac_no_timer_2);

    const int charac_no_timer_1 = user->getCurCharacNo();
    const int login_tick_timer_1 = user->GetLoginTick();
    const int timer_1_delay = static_cast<int>(get_rand_int(300)) + 600;
    const int uid_timer_1 = user->GetUID();
    G_TimerQueue()->InsertTimer(0,
                                uid_timer_1,
                                1,
                                timer_1_delay,
                                static_cast<unsigned int>(login_tick_timer_1),
                                charac_no_timer_1);

    const int charac_no_timer_95 = user->getCurCharacNo();
    const int login_tick_timer_95 = user->GetLoginTick();
    const int uid_timer_95 = user->GetUID();
    G_TimerQueue()->InsertTimer(0,
                                uid_timer_95,
                                95,
                                300,
                                static_cast<unsigned int>(login_tick_timer_95),
                                charac_no_timer_95);

    const int charac_no_timer_96 = user->getCurCharacNo();
    const int login_tick_timer_96 = user->GetLoginTick();
    const int uid_timer_96 = user->GetUID();
    G_TimerQueue()->InsertTimer(0,
                                uid_timer_96,
                                96,
                                600,
                                static_cast<unsigned int>(login_tick_timer_96),
                                charac_no_timer_96);
    user->send_ontime_reward_start_notify();
    return 1;
}

// IDA_EA: 0x8296642
// IDA_NAME: _ZN12CGameManager11SendPvpListEP5CUser
// DEMANGLED: CGameManager::SendPvpList(CUser *)
void CGameManager::SendPvpList(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 41);
    int count_index = packet.get_index();
    packet.put_short(0);
    int sent_room_count = 0;

    for (PvpMap::iterator it = pvp_rooms_.begin(); it != pvp_rooms_.end(); ++it) {
        PvP_Room* const room = it->second;
        if (room && room->get_waiter_count() > 0) {
            ++sent_room_count;
            room->make_room_info(reinterpret_cast<char*>(&packet));
        }
    }

    packet.put_short(count_index, sent_room_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8296792
// IDA_NAME: _ZN12CGameManager13SendPartyListEP5CUser
// DEMANGLED: CGameManager::SendPartyList(CUser *)
void CGameManager::SendPartyList(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PARTY_INFO);

    int count_index = packet.get_index();
    packet.put_short(static_cast<int>(parties_.size()));

    // This local is initialized to zero and never changed in the original
    // body, so it emits the complete (rather than delta) party record form.
    const uint8_t list_kind = 0;
    int sent_party_count = 0;

    for (PartyMap::iterator it = parties_.begin(); it != parties_.end(); ++it) {
        CParty* const party = it->second;
        if (!party || static_cast<uint8_t>(party->IsAutoCreated()) == 1u) {
            continue;
        }

        ++sent_party_count;
        packet.put_short(party->GetPartyIndex());
        packet.put_byte(list_kind);

        if (list_kind <= 1u) {
            const uint8_t title_index =
                static_cast<uint8_t>(party->getTitleIndex());
            packet.put_byte(title_index);
            if (!title_index) {
                char* const title = party->getTitle();
                const int title_length = static_cast<int>(std::strlen(title));
                packet.put_int(title_length);
                packet.put_str(title, title_length);
            }
            packet.put_byte(
                static_cast<uint8_t>(party->IsReturnUserParty()) ? 1 : 0);
            packet.put_byte(HasUserEventCharacterParty(party));
            packet.put_byte(party->getUserMax());
            packet.put_short(party->getDungIndex());
            packet.put_byte(party->getDungDiffi());
            packet.put_byte(
                static_cast<uint8_t>(party->IsEventCharacParty()) ? 1 : 0);
        }

        if (!list_kind || list_kind == 2u) {
            int manager_slot = 0;
            for (int slot = 0; slot < 8; ++slot) {
                // CLIENT_IDA_EVIDENCE: sub_597E20 reads and installs all eight
                // wire slots (0x5980F1..0x598221).  Ordinary CParty still owns
                // only four live members; the generic client-slot width is not
                // an ordinary-party capacity increase.
                CUser* const member = slot < 4 ? party->get_user(slot) : 0;
                if (member) {
                    packet.put_short(member->get_unique_id());
                    if (party->getManager() == member) {
                        manager_slot = slot;
                    }
                    int sex = member->getSex();
                    if (sex == -1) {
                        sex = 1;
                    }
                    packet.put_byte(sex);
                } else {
                    packet.put_short(0xFFFF);
                    packet.put_byte(0xFF);
                }
            }

            packet.put_byte(
                static_cast<uint8_t>(party->IsReturnUserParty()) ? 1 : 0);
            packet.put_byte(HasUserEventCharacterParty(party));
            packet.put_byte(manager_slot);
            packet.put_byte(party->is_quick_party());
            packet.put_byte(
                static_cast<uint8_t>(party->IsEventCharacParty()) ? 1 : 0);
        }

        if (list_kind <= 2u) {
            // 0627's four-slot tag pairs do not match the 0828 tag-record layout.
            packet.put_byte(0);
        }
    }

    packet.put_short(count_index, sent_party_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82A2F94
// IDA_NAME: _ZN12CGameManager15SendWarRoomListEP5CUser
// DEMANGLED: CGameManager::SendWarRoomList(CUser *)
void CGameManager::SendWarRoomList(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 87);

    int count_index = packet.get_index();
    packet.put_short(0);
    int sent_room_count = 0;

    for (WarRoomMap::iterator it = war_rooms_.begin(); it != war_rooms_.end(); ++it) {
        WarRoom* const room = it->second;
        if (room) {
            room->MakeRoomInfo(reinterpret_cast<char*>(&packet));
            ++sent_room_count;
        }
    }

    packet.put_short(count_index, sent_room_count);
    packet.finalize(true);
    user->Send(packet);
}
