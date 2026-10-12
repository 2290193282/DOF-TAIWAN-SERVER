#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstring>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace WongWork {

// IDA_EA: 0x814926E
// IDA_NAME: _ZN8WongWork9CBossPlay12InitPlayUserEii
// DEMANGLED: WongWork::CBossPlay::InitPlayUser(int,int)
void CBossPlay::InitPlayUser(int dungeon_index, int tower_index) {
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }
        user->setBossTowerIndex(static_cast<short>(tower_index));
        user->set_state(12);
        setMemberAlive(seat, true);

        UserQuest* const quest = user->getCurCharacQuestW();
        quest->authen_data.reset();
        quest->set_authen_data(static_cast<QUEST_CONDITION>(0),
                               dungeon_index,
                               -1);
    }
}

// IDA_EA: 0x814F426
// IDA_NAME: _ZN8WongWork10CBossStage16RandomConsistMapER5CUser
// DEMANGLED: WongWork::CBossStage::RandomConsistMap(CUser &)
int CBossStage::RandomConsistMap(CUser& manager) {
    int map_index = 0;
    if (randomMapIndex(map_index) != 1) {
        return 0;
    }

    // The map override is a one-shot GM setting.  The stage-transition path
    // consumes the same CUser member at 0x814F8A3; initial composition uses
    // the identical selected-map contract.
    if (manager.isGMUser() && manager.gm_boss_tower_map_index_override_) {
        map_index = static_cast<int>(manager.gm_boss_tower_map_index_override_);
        manager.gm_boss_tower_map_index_override_ = 0;
    }

    CMap* const map = G_CDataManager()->find_map(map_index);
    return map ? consistMap(map) : 0;
}

// IDA_EA: 0x8142F32
// IDA_NAME: _ZN8WongWork10CBossTower13InitBossTowerEP6CPartyi
// DEMANGLED: WongWork::CBossTower::InitBossTower(CParty *,int)
int CBossTower::InitBossTower(CParty* party, int dungeon_index) {
    dungeon_ = G_CDataManager()->find_dungeon(dungeon_index);
    if (!party || !dungeon_) {
        return 0;
    }
    party_ = party;
    maximum_stage_ = static_cast<uint32_t>(dungeon_->getRandomTowerMaxStage());
    prepare_finish_ = 0;
    return 1;
}

// IDA_EA: 0x8142B72
// IDA_NAME: _ZN8WongWork10CBossTower17handleEPLPCommandEi
// DEMANGLED: WongWork::CBossTower::handleEPLPCommand(int)
int CBossTower::handleEPLPCommand(int command) {
    if (!party_ || party_->GetEPLPState() != 2) {
        return 0;
    }

    if (command <= 2) {
        char member_names[252] = {};
        party_->getMemberNames(member_names);

        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party_->get_user(seat);
            if (!user || party_->checkValidUser(seat) != 1) {
                continue;
            }

            user->history_log().LeaveDungeon(
                dungeon_->GetDungeonName(),
                0,
                member_names,
                party_->battle_field().instance_dungeon_mode());
        }
        party_->ReturnToVillage();
    }

    _destroy();
    return 1;
}

// IDA_EA: 0x8142FA6
// IDA_NAME: _ZN8WongWork10CBossTower23checkStartGameConditionERhb
// DEMANGLED: WongWork::CBossTower::checkStartGameCondition(unsigned char &,bool)
int CBossTower::checkStartGameCondition(unsigned char& failed_member,
                                        bool consume_ancient_items) {
    Inven_Item inventory_snapshots[4];
    int inventory_slots[4];
    std::memset(inventory_snapshots, 0, sizeof(inventory_snapshots));
    std::memset(inventory_slots, 0, sizeof(inventory_slots));

    if (!dungeon_ || dungeon_->IsOpenScheduleDungeon() != 1) {
        cMyTrace trace(
            "bool WongWork::CBossTower::checkStartGameCondition(unsigned char&, bool)",
            219, 5);
        trace("boss Tower dungeon is not open");
        PacketGuard packet;
        packet.put_header(1, 16);
        packet.put_byte(0);
        packet.put_int(21);
        packet.finalize(true);
        party_->send_to_party(packet);
        return 0;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }
        if (user->get_charac_level() < dungeon_->get_min_level()) {
            cMyTrace trace(
                "bool WongWork::CBossTower::checkStartGameCondition(unsigned char&, bool)",
                240, 5);
            trace("dungeon level check charac_level : %d, dungeon_min_level : %d",
                  user->get_charac_level(), dungeon_->get_min_level());
            PacketGuard packet;
            packet.put_header(1, 16);
            packet.put_byte(0);
            packet.put_int(14);
            packet.finalize(true);
            party_->send_to_party(packet);
            return 0;
        }
    }

    if (party_->battle_field().instance_dungeon_mode() == 1) {
        return 1;
    }
    if (!party_->CheckEnterAncientDungeon(dungeon_,
                                          failed_member,
                                          inventory_snapshots,
                                          inventory_slots)) {
        PacketGuard packet;
        packet.put_header(1, 16);
        packet.put_byte(0);
        packet.put_int(17);
        packet.put_byte(failed_member);
        packet.finalize(true);
        party_->send_to_party(packet);
        return 0;
    }
    if (!consume_ancient_items ||
        party_->UseAncientDungeonItems(dungeon_,
                                       inventory_snapshots,
                                       inventory_slots) == 1) {
        return 1;
    }

    PacketGuard packet;
    packet.put_header(1, 16);
    packet.put_byte(0);
    packet.put_int(1);
    packet.finalize(true);
    party_->send_to_party(packet);
    return 0;
}

// IDA_EA: 0x81434F4
// IDA_NAME: _ZN8WongWork10CBossTower15handleStartGameEP6CPartyic17ENUM_DUNGEON_TYPE
// DEMANGLED: WongWork::CBossTower::handleStartGame(CParty *,int,char,ENUM_DUNGEON_TYPE)
int CBossTower::handleStartGame(CParty* party,
                                int dungeon_index,
                                char member_count_class,
                                ENUM_DUNGEON_TYPE dungeon_mode) {
    if (static_cast<int>(dungeon_mode) == 1 ||
        party->battle_field().instance_dungeon_mode() == 1) {
        return 0;
    }

    stage_value_aec_ = static_cast<uint32_t>(dungeon_mode);
    member_count_class_ = static_cast<uint32_t>(member_count_class);
    CBossPlay::reset();
    CBossStage::reset();
    entrance_log_.reset();
    current_stage_ = 1;

    if (InitBossTower(party, dungeon_index) != 1) {
        return 0;
    }
    unsigned char failed_member = 0;
    if (checkStartGameCondition(failed_member, true) != 1) {
        return 0;
    }

    CUser* const manager = party->getManager();
    char packet_member_count_class = member_count_class;
    if (manager->isGMUser() && manager->gm_boss_tower_stage_override_) {
        packet_member_count_class = static_cast<char>(
            manager->gm_boss_tower_stage_override_);
        manager->gm_boss_tower_stage_override_ = 0;
    }
    if (RandomConsistMap(*manager) != 1) {
        return 0;
    }

    InitPlayUser(dungeon_index, static_cast<int>(pool_index_));
    // handleStartGame @ 0x814365b records the clear-process timer separately
    // from CBossStage's per-stage duration pair.  _onPrepareFinishTower uses
    // this +0xaf4/+0xaf8 pair for its DungeonClearInfo elapsed time.
    stage_prepare_tick_ = GlobalData::s_systemTime_.getCurTickCount();
    stage_finish_tick_ = 0;

    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    party_->getMemberNames(member_names);
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }
        entrance_log_.IncrementBossStageStart(
            *user, static_cast<uint8_t>(current_stage_),
            static_cast<uint8_t>(member_count_class_));
        GetInstanceDungeonEntranceLog()->IncrementDungeonEntrance(
            dungeon_index, party_->battle_field().instance_dungeon_mode() == 1);
        user->history_log().EnterDungeon(
            dungeon_->GetDungeonName(), static_cast<int>(member_count_class_),
            member_names, party_->battle_field().instance_dungeon_mode());
    }

    party_->SetEPLPState(1);
    party_->gen_timer_key(static_cast<TIMER_MESSAGE>(45));
    party_->SetSelectedEPLPCmd(-1);

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_BOSS_TOWER_INFO);
    packet.put_int(dungeon_index);
    packet.put_short(static_cast<int>(maximum_stage_));
    packet.put_byte(party_->battle_field().instance_dungeon_mode());
    packet.put_byte(packet_member_count_class);
    packet.finalize(true);
    party_->send_to_party(packet);
    SendMapInfoPacket();

    const int timeout = party_->get_member_count() == 1 ? 90 : 120;
    G_TimerQueue()->InsertTimer(7, static_cast<int>(pool_index_), 165,
                                timeout, 165, 165);
    return 1;
}

}  // namespace WongWork

// IDA_EA: 0x8146722
// IDA_NAME: _ZN6Global14CBossTowerFunc13SelectDungeonEP6CPartys17ENUM_DUNGEON_TYPE
// DEMANGLED: Global::CBossTowerFunc::SelectDungeon(CParty *,short,ENUM_DUNGEON_TYPE)
int Global::CBossTowerFunc::SelectDungeon(CParty* party,
                                          short dungeon_index,
                                          ENUM_DUNGEON_TYPE dungeon_mode) {
    if (!party) {
        return 0;
    }

    WongWork::CBossTower* const boss_tower =
        G_CGameManager()->getBossTower();
    if (!boss_tower) {
        cMyTrace trace(
            "static bool Global::CBossTowerFunc::SelectDungeon(CParty*, short int, ENUM_DUNGEON_TYPE)",
            52, 5);
        trace("StaticPool BossTower NULL");
        return 0;
    }

    const int member_count = party->get_member_count();
    char member_count_class = 0;
    if (member_count == 2) {
        member_count_class = 1;
    } else if (member_count == 3) {
        member_count_class = 2;
    } else if (member_count == 4) {
        member_count_class = 3;
    }

    const int started = boss_tower->handleStartGame(
        party, dungeon_index, member_count_class, dungeon_mode);
    if (started != 1) {
        boss_tower->_destroy();
    }
    return started;
}


// IDA_EA: 0x8146824
// IDA_NAME: _ZN6Global14CBossTowerFunc18DieCharacInDungeonEP5CUser
// DEMANGLED: Global::CBossTowerFunc::DieCharacInDungeon(CUser *)
int Global::CBossTowerFunc::DieCharacInDungeon(CUser* user) {
    const int state = user->get_state();
    if (state == 10) {
        WongWork::CDeathTower* const death_tower = user->getDeathTower();
        if (death_tower) {
            death_tower->handleDieUser(user);
        }
        return 1;
    }

    if (state == 12) {
        WongWork::CBossTower* const boss_tower = user->getBossTower();
        if (boss_tower) {
            boss_tower->handleDieUser(user);
        }
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8146AD8
// IDA_NAME: _ZN6Global14CBossTowerFunc10GiveUpGameEP5CUser
// DEMANGLED: Global::CBossTowerFunc::GiveUpGame(CUser *)
int Global::CBossTowerFunc::GiveUpGame(CUser* user) {
    switch (user->get_state()) {
    case 5: {
        CParty* const party = user->GetParty();
        if (party) {
            party->giveup_game(user, true, false, false);
        }
        break;
    }
    case 10: {
        WongWork::CDeathTower* const death_tower = user->getDeathTower();
        if (death_tower) {
            death_tower->handleLeaveUser(user);
        }
        break;
    }
    case 12: {
        WongWork::CBossTower* const boss_tower = user->getBossTower();
        if (boss_tower) {
            boss_tower->handleLeaveUser(user);
        }
        break;
    }
    }
    return 1;
}

// IDA_EA: 0x8146916
// IDA_NAME: _ZN6Global14CBossTowerFunc10FinishLoadEP5CUser
// DEMANGLED: Global::CBossTowerFunc::FinishLoad(CUser *)
int Global::CBossTowerFunc::FinishLoad(CUser* user) {
    const int state = user->get_state();
    if (state == 10) {
        WongWork::CDeathTower* const death_tower = user->getDeathTower();
        if (death_tower) {
            death_tower->handleFinishLoading(user);
        }
        return 1;
    }
    if (state == 12) {
        WongWork::CBossTower* const boss_tower = user->getBossTower();
        if (boss_tower) {
            boss_tower->handleFinishLoading(user);
        }
        return 1;
    }
    return 0;
}


// IDA_EA: 0x8146BA2
// IDA_NAME: _ZN6Global14CBossTowerFunc7MoveMapEP5CUserPt
// DEMANGLED: Global::CBossTowerFunc::MoveMap(CUser *,unsigned short *)
int Global::CBossTowerFunc::MoveMap(CUser* user,
                                    unsigned short* map_path) {
    const int state = user->get_state();
    if (state == 10) {
        WongWork::CDeathTower* death_tower = user->getDeathTower();
        if (death_tower) {
            death_tower->handleMoveMap();
        }
        return 1;
    }

    if (state == 12) {
        WongWork::CBossTower* boss_tower = user->getBossTower();
        if (boss_tower) {
            boss_tower->handleMoveMap(map_path);
        }
        return 1;
    }

    return 0;
}

// IDA_EA: 0x8146C20
// IDA_NAME: _ZN6Global14CBossTowerFunc7UseItemEP5CUserRK10Inven_Item
// DEMANGLED: Global::CBossTowerFunc::UseItem(CUser *,Inven_Item const&)
int Global::CBossTowerFunc::UseItem(CUser* user, const Inven_Item& item) {
    const int state = user->get_state();
    if (state == 5) {
        CParty* const party = user->GetParty();
        if (party) {
            party->useItem(user, item);
        }
        return 1;
    }

    if (state == 12) {
        WongWork::CBossTower* const boss_tower = user->getBossTower();
        if (boss_tower) {
            boss_tower->handleUseItem(user, item.item_id_, item.item_category_);
        }
        return 1;
    }

    return 0;
}

namespace WongWork {

// IDA_EA: 0x81441D4
// IDA_NAME: _ZN8WongWork10CBossTower14SendELPLPacketEv
// DEMANGLED: WongWork::CBossTower::SendELPLPacket(void)
int CBossTower::SendELPLPacket() {
    bool can_start = true;
    if (party_->battle_field().instance_dungeon_mode() != 1) {
        for (int member_slot = 0; member_slot <= 3; ++member_slot) {
            CUser* const user = party_->get_user(member_slot);
            if (!user || party_->checkValidUser(member_slot) != 1) {
                continue;
            }

            unsigned char failed_member = 0;
            if (checkStartGameCondition(failed_member, false) != 1) {
                can_start = false;
                break;
            }
        }
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_BOSS_TOWER_STATE_EPLP);
    packet.put_byte(can_start ? 1 : 0);
    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

// IDA_EA: 0x814432A
// IDA_NAME: _ZN8WongWork10CBossTower16SendRewardPacketEv
// DEMANGLED: WongWork::CBossTower::SendRewardPacket(void)
int CBossTower::SendRewardPacket() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || party_->checkValidUser(member_slot) != 1) {
            continue;
        }

        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_BOSS_TOWER_STATE_REWARD);
        packet.put_int(static_cast<int>(values_[member_slot]));
        const stRewardItem_t& reward = rewards_[member_slot];
        packet.put_byte(static_cast<int>(reward.reward_value_));
        for (int item_slot = 0;
             item_slot < static_cast<int>(reward.reward_value_);
             ++item_slot) {
            const Inven_Item& item = reward.items_[item_slot];
            packet.put_int(item.item_id_);
            packet.put_int(item.get_add_info());
        }
        packet.finalize(true);
        user->Send(packet);
    }
    return 1;
}

// IDA_EA: 0x81446A2
// IDA_NAME: _ZN8WongWork10CBossTower15InsertRewardExpEv
// DEMANGLED: WongWork::CBossTower::InsertRewardExp(void)
int CBossTower::InsertRewardExp() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || party_->checkValidUser(member_slot) != 1) {
            continue;
        }

        int gained_sp = 0;
        int gained_sfp = 0;
        if (user->gain_exp_sp(static_cast<int>(values_[member_slot]),
                              gained_sp,
                              gained_sfp,
                              static_cast<eExpAddReason>(2),
                              0,
                              false)) {
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                                 static_cast<ENUM_NOTIPACKET>(2),
                                 1);
        }
    }
    return 1;
}

// IDA_EA: 0x8144D54
// IDA_NAME: _ZN8WongWork10CBossTower17generateRewardExpEv
// DEMANGLED: WongWork::CBossTower::generateRewardExp(void)
int CBossTower::generateRewardExp() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || party_->checkValidUser(member_slot) != 1) {
            continue;
        }

        unsigned int reward_multiplier = 0;
        G_CDataManager()->get_mob_reward(user->get_charac_level(),
                                         reward_multiplier);
        CBossPlay::generateRewardExp(member_slot,
                                     current_stage_,
                                     reward_multiplier,
                                     dungeon_);
    }
    return 1;
}

// IDA_EA: 0x8144E36
// IDA_NAME: _ZN8WongWork10CBossTower18generateRewardItemEv
// DEMANGLED: WongWork::CBossTower::generateRewardItem(void)
int CBossTower::generateRewardItem() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || party_->checkValidUser(member_slot) != 1) {
            continue;
        }

        const stDeathTower_t* const tower_script =
            G_CDataManager()->GetTowerScript(dungeon_->get_index());
        const int script_limit =
            tower_script->reward_card_num[current_stage_];
        const int dungeon_limit = dungeon_->getTowerMaxClearRewardItemNum();
        const int item_count = std::min(10, std::min(dungeon_limit, script_limit));
        const unsigned int reward_context = std::min(10u, current_stage_);
        CBossPlay::generateRewardItem(
            member_slot,
            item_count,
            static_cast<unsigned int>(user->get_charac_level()),
            static_cast<ENUM_DUNGEON_DIFFICULTY>(member_count_class_),
            reward_context,
            dungeon_);
    }
    return 1;
}

// IDA_EA: 0x8144FCA
// IDA_NAME: _ZN8WongWork10CBossTower15onReturnVillageEv
// DEMANGLED: WongWork::CBossTower::onReturnVillage(void)
int CBossTower::onReturnVillage() {
    if (party_ && party_->GetEPLPState() == 2) {
        // IDA 0x8145018 clears the four-byte tail after the 252-byte name
        // buffer, stores the instance mode in its final byte, and then
        // calls CParty::getMemberNames.  That helper owns the historical
        // `buffer[255] = 0` terminator, so the value observed by each
        // LeaveDungeon call is the resulting signed final byte, not a
        // separately retained dungeon-mode local.
        char member_names[256] = {};
        member_names[255] = static_cast<char>(
            party_->battle_field().instance_dungeon_mode());
        party_->getMemberNames(member_names);
        const int history_instance_dungeon_mode =
            static_cast<int>(static_cast<signed char>(member_names[255]));
        for (int member_slot = 0; member_slot <= 3; ++member_slot) {
            CUser* const user = party_->get_user(member_slot);
            if (!user || party_->checkValidUser(member_slot) != 1) {
                continue;
            }
            user->history_log().LeaveDungeon(
                dungeon_->GetDungeonName(),
                static_cast<int>(member_count_class_),
                member_names,
                history_instance_dungeon_mode);
        }
        party_->ReturnToVillage();
        _destroy();
    }
    return 1;
}

}  // namespace WongWork
