#include "old_game/pvp/battlefield/battle_field.h"

#include <cstring>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// The original start operation throws this fixed-underlying-type enum through
// the server's packet-dispatch boundary.  Its enumerator table is not part of
// the recovered unit, but the old values 1/8/14/17/21/22 are preserved below.
enum ENUM_ERROR : int {};

namespace WongWork {

// IDA_EA: 0x84605A2
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr14initDungeonMgrEi
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::initDungeonMgr(int)
int CDeathTower::CDungeonMgr::initDungeonMgr(int dungeon_index) {
    dungeon_ = G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon_) {
        return 0;
    }
    end_stage_ = dungeon_->getDeathTowerMaxStage();
    return 1;
}

// IDA_EA: 0x8462A26
// IDA_NAME: _ZN8WongWork11CDeathTower11attachPartyEP6CParty
// DEMANGLED: WongWork::CDeathTower::attachParty(CParty *)
int CDeathTower::attachParty(CParty* party) {
    party_ = party;
    for (int seat = 0; seat <= 3; ++seat) {
        if (party_->checkValidUser(seat) == 1) {
            CUser* const user = party_->get_user(seat);
            if (user) {
                user->setDeathTowerIndex(static_cast<short>(index_));
            }
        }
    }
    return setTowerState(static_cast<ENUM_TOWER_STATE>(1));
}

// IDA_EA: 0x84636F2
// IDA_NAME: _ZN8WongWork11CDeathTower11onLeaveUserEP5CUser
// DEMANGLED: WongWork::CDeathTower::onLeaveUser(CUser *)
int CDeathTower::onLeaveUser(CUser* user) {
    if (!user) {
        return 0;
    }

    user->setDeathTowerIndex(-1);
    user->getCurCharacInvenW()->RemoveFPItem();

    if (party_) {
        char member_names[256] = {};
        party_->getMemberNames(member_names);
        CDungeon* const dungeon = dungeon_manager_.getDungeon();
        if (dungeon) {
            user->history_log().LeaveDungeon(
                dungeon->GetDungeonName(), 0, member_names,
                party_->battle_field().instance_dungeon_mode());
            GetInstanceDungeonEntranceLog()->DecrementDungeonEntrance(
                dungeon->get_index(), false);
        }
    }

    if (!party_ || party_->get_member_count() <= 0) {
        _destroy();
    }
    return 1;
}

// IDA_EA: 0x8465AC2
// IDA_NAME: _ZN8WongWork11CDeathTower15handleLeaveUserEP5CUser
// DEMANGLED: WongWork::CDeathTower::handleLeaveUser(CUser *)
int CDeathTower::handleLeaveUser(CUser* user) {
    if (!user || !party_) {
        return 0;
    }

    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (party_->checkValidUser(member_slot) == 1 &&
            party_->get_user(member_slot) == user) {
            party_->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
            play_data_.setMemberAlive(member_slot, false);
        }
    }

    G_GameWorld()->out_from_dungeon(user);
    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);

    onLeaveUser(user);
    if (party_->get_member_count() > 0 && _checkMemberDie()) {
        _onFinishStage();
        _onPrepareFinishTower();
    }
    return 1;
}

// IDA_EA: 0x8463890
// IDA_NAME: _ZN8WongWork11CDeathTower24_checkStartGameConditionERh
// DEMANGLED: WongWork::CDeathTower::_checkStartGameCondition(unsigned char &)
int CDeathTower::_checkStartGameCondition(unsigned char& failed_member) {
    Inven_Item inventory_snapshots[4];
    int inventory_slots[4];
    std::memset(inventory_snapshots, 0, sizeof(inventory_snapshots));
    std::memset(inventory_slots, 0, sizeof(inventory_slots));

    const CDungeon* const dungeon = dungeon_manager_.getDungeon();
    if (party_->CheckEnterAncientDungeon(dungeon,
                                         failed_member,
                                         inventory_snapshots,
                                         inventory_slots) != 1) {
        return 17;
    }
    return party_->UseAncientDungeonItems(dungeon,
                                          inventory_snapshots,
                                          inventory_slots) != 1;
}

// IDA_EA: 0x846398E
// IDA_NAME: _ZN8WongWork11CDeathTower15handleStartGameEic17ENUM_DUNGEON_TYPE
// DEMANGLED: WongWork::CDeathTower::handleStartGame(int,char,ENUM_DUNGEON_TYPE)
int CDeathTower::handleStartGame(int dungeon_index,
                                 char dungeon_difficulty,
                                 ENUM_DUNGEON_TYPE dungeon_mode) {
    static_cast<void>(dungeon_difficulty);
    unsigned char failed_member = 0;
    if (static_cast<int>(dungeon_mode) == 1) {
        throw static_cast<ENUM_ERROR>(8);
    }

    dungeon_manager_.reset();
    if (dungeon_manager_.initDungeonMgr(dungeon_index) != 1) {
        throw static_cast<ENUM_ERROR>(22);
    }

    const int condition_error = _checkStartGameCondition(failed_member);
    if (condition_error != 0) {
        throw static_cast<ENUM_ERROR>(condition_error);
    }

    stage_.reset();
    stage_.resetCurrentStage();
    CMap* const stage_map = dungeon_manager_.getStageMap(
        static_cast<unsigned int>(stage_.getCurrentStage()));
    if (!stage_map) {
        throw static_cast<ENUM_ERROR>(21);
    }
    if (stage_.consistMap(dungeon_manager_, stage_map, this) != 1) {
        throw static_cast<ENUM_ERROR>(1);
    }

    play_data_.reset();
    play_data_.makeStartMemberInfo(party_);
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }
        const CDungeon* const dungeon = dungeon_manager_.getDungeon();
        if (user->get_charac_level() < dungeon->get_min_level()) {
            throw static_cast<ENUM_ERROR>(14);
        }
        user->set_state(10);
        play_data_.setMemberAlive(seat, true);
        GetInstanceDungeonEntranceLog()->IncrementDungeonEntrance(
            dungeon_index, party_->battle_field().instance_dungeon_mode() == 1);
    }

    party_->gen_timer_key(static_cast<TIMER_MESSAGE>(45));
    party_->SetSelectedEPLPCmd(-1);
    party_->choose_random_buff(static_cast<QuickParty::RandomBuffDungeon>(
        party_->battle_field().getRandomBuffDungeonType(0)));

    PacketGuard start_packet;
    start_packet.put_header(0, 142);
    start_packet.put_int(dungeon_index);
    start_packet.put_short(dungeon_manager_.getEndStage());
    start_packet.put_byte(party_->battle_field().instance_dungeon_mode());
    start_packet.put_byte(party_->get_random_buff_type());
    start_packet.finalize(true);
    party_->send_to_party(start_packet);

    PacketGuard stage_packet;
    stage_packet.put_header(0, 143);
    stage_.makeStagePacket(stage_packet);
    stage_packet.finalize(true);
    party_->send_to_party(stage_packet);

    _checkMemberReady();
    setTowerState(static_cast<ENUM_TOWER_STATE>(2));
    _onStartDeathTower();
    return 1;
}

// IDA_EA: 0x8467B50
// IDA_NAME: _ZN8WongWork11CDeathTower18_onStartDeathTowerEv
// DEMANGLED: WongWork::CDeathTower::_onStartDeathTower(void)
bool CDeathTower::_onStartDeathTower() {
    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    party_->getMemberNames(member_names);

    const CDungeon* const dungeon = dungeon_manager_.getDungeon();
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }
        user->history_log().EnterDungeon(
            dungeon->GetDungeonName(), 0, member_names,
            party_->battle_field().instance_dungeon_mode());
        UserQuest* const quest = user->getCurCharacQuestW();
        quest->authen_data.reset();
        quest->set_authen_data(static_cast<QUEST_CONDITION>(0),
                               dungeon->get_index(),
                               -1);
    }
    return true;
}

// IDA_EA: 0x8469C34
// IDA_NAME: _ZN8WongWork11CDeathTower13setTowerStateENS0_16ENUM_TOWER_STATEE
// DEMANGLED: WongWork::CDeathTower::setTowerState(WongWork::CDeathTower::ENUM_TOWER_STATE)
int CDeathTower::setTowerState(ENUM_TOWER_STATE state) {
    if (static_cast<int>(state) <= static_cast<int>(tower_state_)) {
        return 0;
    }
    tower_state_ = state;
    return 1;
}

// IDA_EA: 0x8467B20
// IDA_NAME: _ZN8WongWork11CDeathTower8_destroyEv
// DEMANGLED: WongWork::CDeathTower::_destroy(void)
void CDeathTower::_destroy() {
    setTowerState(static_cast<ENUM_TOWER_STATE>(0));
    CGameManager* const manager = G_CGameManager();
    if (manager) {
        manager->returnDeathTower(this);
    }
}

}  // namespace WongWork
