#include "old_game/social/party/party.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "common/config.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/play_info_public_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/dungeon/progress/best_clear_time_runtime.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/dungeon/instance/tod_layer.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

extern unsigned int get_rand_int(int limit) __asm__("_Z12get_rand_inti");

// IDA_EA: 0x85A7B8A
// IDA_NAME: _ZN6CParty26CheckHaveHellPartyPassItemEPK8CDungeon17ENUM_DUNGEON_TYPE
// DEMANGLED: CParty::CheckHaveHellPartyPassItem(CDungeon const*,ENUM_DUNGEON_TYPE)
int CParty::CheckHaveHellPartyPassItem(const CDungeon* dungeon,
                                       ENUM_DUNGEON_TYPE dungeon_mode) {
    if (!world_map_ || world_map_->IsInHellDungeon() != 1 ||
        static_cast<int>(dungeon_mode) != 1) {
        return 1;
    }

    std::vector<int> failed_members;
    if (CheckDestroyConditionSealDoor(
            dungeon, failed_members,
            world_map_->freepass_items_) == 1) {
        hell_dungeon_seal_door_ready_ = 1;
    } else {
        PacketGuard packet;
        packet.put_header(1, 16);
        packet.put_byte(0);
        packet.put_byte(27);
        packet.put_byte(static_cast<int>(failed_members.size()));
        for (std::vector<int>::const_iterator current = failed_members.begin();
             current != failed_members.end(); ++current) {
            packet.put_short(*current);
        }
        packet.finalize(true);
        send_to_party(packet);
        hell_dungeon_seal_door_ready_ = 0;
    }
    return hell_dungeon_seal_door_ready_;
}

// IDA_EA: 0x85BAC26
// IDA_NAME: _ZN6CParty17_Send_TOD_APCInfoEi
// DEMANGLED: CParty::_Send_TOD_APCInfo(int)
int CParty::_Send_TOD_APCInfo(int apc_index) {
    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (dungeon && dungeon->isTowerOfDespairDungeon()) {
        TowerOfDespairMgr* const manager = GlobalData::s_pTowerOfDespairMgr;
        if (manager) {
            manager->SendAPCInfo(TOD_Layer(0), getManager());
            if (apc_index != -1) {
                manager->SendAPCInfo(TOD_Layer(
                    static_cast<uint16_t>(apc_index)), getManager());
            }
        }
    }
    return 1;
}

// IDA_EA: 0x85BACC0
// IDA_NAME: _ZN6CParty21GetDungeonIndex_isTODERiS0_
// DEMANGLED: CParty::GetDungeonIndex_isTOD(int &,int &)
int CParty::GetDungeonIndex_isTOD(int& dungeon_index, int& apc_index) {
    TowerOfDespairMgr* const manager = GlobalData::s_pTowerOfDespairMgr;
    if (!manager) {
        return 19;
    }
    if (manager->GetBaseLayerIndex() != dungeon_index) {
        return 0;
    }

    CUser* const user = getManager();
    if (!user) {
        return 19;
    }
    TOD_UserState* const state = static_cast<TOD_UserState*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
    if (!state) {
        return 19;
    }

    const TOD_Layer layer = state->getTodayEnterLayer();
    dungeon_index = manager->Get_DungeonIndexByLayer(layer.GetLayer());
    const CDungeon* const dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    const int enter_error = manager->CheckEnterable(user, dungeon);
    if (enter_error) {
        return enter_error;
    }

    apc_index = manager->Get_APCIndexByLayer(layer.GetLayer());
    if (apc_index != -1) {
        const char* const apc_name = manager->Get_APCNameByLayer(layer);
        if (!apc_name || !*apc_name) {
            // Dynamic-APC floors must fail before START_MAP when the reload
            // table is empty.  Layer 0 remains a separate zero-record clear
            // projection and is sent by _Send_TOD_APCInfo.
            return 19;
        }
    }
    char message[256];
    std::snprintf(message, sizeof(message), "Enter Layer : %d", layer.GetLayer());
    TowerOfDespairMgr::SendMessageToCUser(user, message);
    return 0;
}

// IDA_EA: 0x85BB62A
// IDA_NAME: _ZN6CParty21SetProperDungeonEventEP5CUseri
// DEMANGLED: CParty::SetProperDungeonEvent(CUser *,int)
int CParty::SetProperDungeonEvent(CUser* user, int dungeon_index) {
    // The i386 body keeps this third ABI argument unused; retain it instead
    // of collapsing the member to a different symbol name.
    (void)dungeon_index;
    user->ClearProperDungeonUser();
    CDungeon* const dungeon = battle_field_.GetDungeon();
    ConditionLevelChkDungeon* const condition =
        G_CDataManager()->get_condition_level_chk_dungeon();
    int result = condition->isEventDungeon(dungeon->get_index()) ^ 1;
    if (!result) {
        result = condition->isApplyLevel(user->get_charac_level()) ^ 1;
        if (!result) {
            const int minimum_level = std::max(dungeon->get_min_level(), 0);
            const int maximum_level =
                std::min<int>(dungeon->get_standard_level() + 1,
                              oldsrv::config::kCharacterLevelMaximum);
            if (user->get_charac_level() >= minimum_level &&
                user->get_charac_level() <= maximum_level) {
                return user->SetProperDungeonUser();
            }
            result = maximum_level;
        }
    }
    return result;
}

// IDA_EA: 0x85BB746
// IDA_NAME: _ZN6CParty21SetProperLevelDungeonEP5CUser
// DEMANGLED: CParty::SetProperLevelDungeon(CUser *)
int CParty::SetProperLevelDungeon(CUser* user) {
    user->ClearProperLevelDungeonUser();
    CDungeon* const dungeon = battle_field_.GetDungeon();
    ConditionLevelChkDungeon* const condition =
        G_CDataManager()->get_condition_level_chk_dungeon();
    int result = condition->isEventDungeon(dungeon->get_index()) ^ 1;
    if (!result) {
        const int minimum_level = std::max(dungeon->get_min_level(), 0);
        const int maximum_level =
            std::min<int>(dungeon->get_standard_level() + 1,
                          oldsrv::config::kCharacterLevelMaximum);
        if (user->get_charac_level() >= minimum_level &&
            user->get_charac_level() <= maximum_level) {
            // The original i386 body returns CUser::SetProperLevelDungeonUser's
            // pointer result through this int-returning legacy boundary.
            return static_cast<int>(reinterpret_cast<uintptr_t>(
                user->SetProperLevelDungeonUser()));
        }
        result = maximum_level;
    }
    return result;
}

// IDA_EA: 0x85BE05A
// IDA_NAME: _ZN6CParty27SendPartyMemberTagChracInfoEv
// DEMANGLED: CParty::SendPartyMemberTagChracInfo(void)
void CParty::SendPartyMemberTagChracInfo() {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 414);
    int count_index = packet.get_index();
    packet.put_short(0);

    int member_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = get_user(seat);
        if (user && user->MakeTagCharacInfo(packet) == 1) {
            ++member_count;
        }
    }
    packet.put_short(count_index, member_count);
    packet.finalize(true);
    const zombie_map_runtime::HookResult zombie_route =
        zombie_map_runtime::RouteBootstrapTagCharacterPacket(this, packet);
    if (zombie_route == zombie_map_runtime::kHookNotManaged)
        send_to_party(packet);
}

// IDA_EA: 0x85BE932
// IDA_NAME: _ZN6CParty22checkDungeonDifficultyEcPK8CDungeon
// DEMANGLED: CParty::checkDungeonDifficulty(char,CDungeon const*)
bool CParty::checkDungeonDifficulty(char dungeon_difficulty,
                                    const CDungeon* dungeon) {
    int designated_difficulty = dungeon
        ? time_spiral::GetDesignatedDifficulty(dungeon->get_index())
        : -1;
    if (dungeon && designated_difficulty < 0) {
        int fixed_difficulty = -1;
        if (dungeon_runtime::GetFixedDifficulty(
                dungeon->get_index(), &fixed_difficulty)) {
            designated_difficulty = fixed_difficulty;
        }
    }
    if (!dungeon || !dungeon_difficulty ||
        designated_difficulty == dungeon_difficulty ||
        getManager()->IsGameMasterMode() == 1 ||
        dungeon->isTournamentDungeon()) {
        return false;
    }
    return getManager()->getMoveSpace() != 1 &&
           dungeon->isRiskDungeon() == 0;
}

// IDA_EA: 0x85B11EE
// IDA_NAME: _ZN6CParty23isPremiumGoldCardRewardEPK8CDungeonRj
// DEMANGLED: CParty::isPremiumGoldCardReward(CDungeon const*,unsigned int &)
int CParty::isPremiumGoldCardReward(const CDungeon* dungeon,
                                    unsigned int& item_id) {
    if (!dungeon) {
        return 0;
    }

    const ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    if (!parameters) {
        return 0;
    }

    const int standard_level = dungeon->get_standard_level();
    for (int index = 0; index < 10; ++index) {
        int minimum_level = 0;
        int maximum_level = 0;
        int probability = 0;
        unsigned int candidate_item_id = 0;
        if (!parameters->getPremiumGoldCardDropRule(
                index,
                minimum_level,
                maximum_level,
                probability,
                candidate_item_id) ||
            minimum_level <= 0 || maximum_level <= 0 || probability <= 0) {
            break;
        }

        if (standard_level >= minimum_level && standard_level <= maximum_level &&
            static_cast<int>(get_rand_int(1000)) <= probability) {
            item_id = candidate_item_id;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85A0954
// IDA_NAME: _ZN6CParty13dungeon_startEic17ENUM_DUNGEON_TYPE
// DEMANGLED: CParty::dungeon_start(int,char,ENUM_DUNGEON_TYPE)
int CParty::dungeon_start(int dungeon_index,
                          char dungeon_difficulty,
                          ENUM_DUNGEON_TYPE dungeon_mode) {
    resetMemberUseCoin();
    first_map_clear_ = 0;
    ClearMapHitCount();
    secu_.startDungeon();

    int apc_index = -1;
    const int tod_error = GetDungeonIndex_isTOD(dungeon_index, apc_index);
    if (tod_error != 0) {
        return tod_error;
    }

    CDungeon* const dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon) {
        return 21;
    }

    const int instance_mode = battle_field_.instance_dungeon_mode();
    SetPartyMemberCoinLimit(dungeon->GetPartyMemberCoinLimit() *
                            get_member_count());
    trace_mob_die_hack_.setDungeonDiff(dungeon_difficulty);

    if (GetEPLPState()) {
        // The old body records hack 0x195 for every valid party member before
        // rejecting a repeated start transition.
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const user = get_user(seat);
            if (user && user->getHackAnalyzer()) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x195u),
                    1u,
                    0u,
                    0u);
            }
        }
        return 8;
    }

    CUser* const manager = getManager();
    if (!manager) {
        return 19;
    }
    if (manager->IsGameMasterMode() != 1 &&
        manager->get_charac_level() < dungeon->get_min_level()) {
        return 14;
    }

    // 140828 fixed-difficulty DGN tags are first-entry contracts, not requests
    // to prove a historical clear at that difficulty.  Reject any forged
    // alternate value before bypassing the legacy clear-difficulty gate.
    int designated_difficulty =
        time_spiral::GetDesignatedDifficulty(dungeon->get_index());
    if (designated_difficulty < 0) {
        int fixed_difficulty = -1;
        if (dungeon_runtime::GetFixedDifficulty(
                dungeon->get_index(), &fixed_difficulty)) {
            designated_difficulty = fixed_difficulty;
        }
    }
    if (designated_difficulty >= 0 &&
        dungeon_difficulty != designated_difficulty) {
        cMyTrace trace("CParty::dungeon_start", __LINE__, 5);
        trace("[CMD16_DUNGEON_START_FAIL] stage=fixed_difficulty_mismatch "
              "party_index(%d) dungeon(%d) requested(%d) designated(%d) "
              "required_item(%d) required_count(%d) delete_after_start(%u)",
              GetPartyIndex(), dungeon->get_index(),
              static_cast<int>(dungeon_difficulty), designated_difficulty,
              dungeon->required_item_.item_id,
              dungeon->required_item_.item_count,
              static_cast<unsigned int>(
                  dungeon->required_item_.delete_after_start));
        return 19;
    }

    if (checkDungeonDifficulty(dungeon_difficulty, dungeon)) {
        const WongWork::CDungeonClear* const cleared =
            reinterpret_cast<const WongWork::CDungeonClear*>(
                manager->getDungeonClearRefR());
        const int cleared_difficulty =
            cleared ? cleared->getClearedDungeonDiff(
                          static_cast<short>(dungeon->get_index()))
                    : 0;
        if (!cleared_difficulty ||
            (cleared_difficulty < dungeon_difficulty &&
             dungeon_difficulty != 4)) {
            cMyTrace trace("CParty::dungeon_start", __LINE__, 5);
            trace("[CMD16_DUNGEON_START_FAIL] stage=historical_difficulty "
                  "party_index(%d) dungeon(%d) requested(%d) "
                  "designated(%d) cleared(%d) required_item(%d) "
                  "required_count(%d) delete_after_start(%u)",
                  GetPartyIndex(), dungeon->get_index(),
                  static_cast<int>(dungeon_difficulty),
                  designated_difficulty, cleared_difficulty,
                  dungeon->required_item_.item_id,
                  dungeon->required_item_.item_count,
                  static_cast<unsigned int>(
                      dungeon->required_item_.delete_after_start));
            return 19;
        }
    }

    int quest_map = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 && instance_mode != 1) {
            CUser* const user = get_user(seat);
            if (user) {
                quest_map = user->CheckQuestMap(dungeon->get_index());
                if (quest_map > 0) {
                    break;
                }
            }
        }
    }

    SetUsedCoinCount(0);
    if (CheckHaveHellPartyPassItem(dungeon, dungeon_mode) != 1) {
        return 0;
    }
    if (checkInoutConditionDungeon(
            dungeon,
            static_cast<ENUM_DUNGEON_MODE>(instance_mode),
            static_cast<ENUM_CMDPACKET>(16))) {
        return 0;
    }

    bool random_hell_party = false;
    battle_field_.check_random_appear_hell_dungeon(
        dungeon,
        static_cast<ENUM_DUNGEON_MODE>(instance_mode),
        dungeon_mode,
        random_hell_party);
    if (!battle_field_.SelectDungeon(dungeon_index,
                                     dungeon,
                                     dungeon_difficulty,
                                     dungeon_mode,
                                     get_member_count(),
                                     world_map_,
                                     random_hell_party,
                                     quest_map)) {
        LogManager::logFormat(
            1,
            "party.cpp",
            "int CParty::dungeon_start(int, char, ENUM_DUNGEON_TYPE)",
            0x153,
            "[CMD16_DUNGEON_START_FAIL] stage=select-dungeon dungeon=%d difficulty=%d mode=%d members=%d instance_mode=%d\n",
            dungeon_index,
            static_cast<int>(dungeon_difficulty),
            static_cast<int>(dungeon_mode),
            get_member_count(),
            instance_mode);
        return 1;
    }

    choose_random_buff(static_cast<QuickParty::RandomBuffDungeon>(
        battle_field_.getRandomBuffDungeonType(0)));

    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    _getMemberNames(member_names);
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const user = get_user(seat);
        if (!user) {
            continue;
        }
        BestClearTime* const best_clear_time = static_cast<BestClearTime*>(
            user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(18)));
        if (best_clear_time) {
            best_clear_time->setNewRecord(false);
        }
        CUser_history_log(user)->EnterDungeon(dungeon->GetDungeonName(),
                                              dungeon_difficulty,
                                              member_names,
                                              instance_mode);
        user->resetDungeonClearHackCheck();

        if (user->getMoveSpace() == 0) {
            int empty_skill_slot_kind = 0;
            const int empty_skill_slot_count =
                user->getCurCharacSkillW()->checkskillSlotEmptyCount(
                    empty_skill_slot_kind);
            for (int index = 0; index < empty_skill_slot_count; ++index) {
                const unsigned int saved_kind = static_cast<unsigned int>(
                    user->SetETC(422, empty_skill_slot_kind));
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x1A6u),
                    1u,
                    saved_kind,
                    0u);
            }

            int empty_equipment_count = 0;
            int mastery_mismatch_count = 0;
            int mastery_mismatch_state = 0;
            int unselected_avatar_count = 0;
            int creature_artifact_count = 0;
            int mastery_equipment_count = 0;
            user->getCurCharacInvenW()->checkEquipmentState(
                user->get_charac_job(),
                user->getCurCharFirstGrowType(),
                empty_equipment_count,
                mastery_mismatch_count,
                mastery_mismatch_state,
                unselected_avatar_count,
                creature_artifact_count,
                mastery_equipment_count);

            if (user->get_charac_level() > 15) {
                for (int index = 0; index < empty_equipment_count; ++index) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(0x1A4u),
                        1u,
                        0u,
                        0u);
                }
            }
            if (user->get_charac_level() > 23) {
                for (int index = 0; index < mastery_mismatch_count; ++index) {
                    const unsigned int saved_state =
                        static_cast<unsigned int>(user->SetETC(
                            421, mastery_mismatch_state));
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(0x1A5u),
                        1u,
                        saved_state,
                        0u);
                }
            }
            for (int index = 0; index < unselected_avatar_count; ++index) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x1A7u),
                    1u,
                    0u,
                    0u);
            }
            for (int index = 0; index < creature_artifact_count; ++index) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x1A8u),
                    1u,
                    0u,
                    0u);
            }
            for (int index = 0; index < mastery_equipment_count; ++index) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x1A9u),
                    1u,
                    0u,
                    0u);
            }
        }
    }

    if (world_map_ == 0 || world_map_->IsInHellDungeon() != 1 ||
        static_cast<int>(dungeon_mode) != 1 ||
        hell_dungeon_seal_door_ready_ == 0 ||
        UseSealDoorItems(world_map_->freepass_items_) == 1) {
        if (!racing_dungeon::CommitDungeonStart(
                this, dungeon_index)) {
            LogManager::logFormat(
                1,
                "party.cpp",
                "int CParty::dungeon_start(int, char, ENUM_DUNGEON_TYPE)",
                0x1f5,
                "[CMD16_DUNGEON_START_FAIL] stage=racing-commit dungeon=%d difficulty=%d mode=%d members=%d\n",
                dungeon_index,
                static_cast<int>(dungeon_difficulty),
                static_cast<int>(dungeon_mode),
                get_member_count());
            // SelectDungeon may already have allocated a TimeSpiral run.
            // A later admission transaction failure must not leave that
            // generation attached to a party which never entered state 2.
            time_spiral::ResetParty(this);
            return 1;
        }

        champion_break::BeginDungeonRun(this, dungeon_index);

        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) == 1) {
                fatigue_acceleration::BeginDungeonRun(
                    get_user(seat), dungeon, dungeon_index);
            }
        }

        PacketGuard packet;
        const bool blood_dungeon = dungeon->get_blood_dungeon_type() > 0;
        if (!blood_dungeon) {
            const bool tournament = dungeon->isTournamentDungeon() != 0;
            packet.clear();
            packet.put_header(
                0,
                tournament ? 386 : ENUM_NOTIPACKET_DUNGEON_INFO);
            packet.put_short(dungeon_index);
            packet.put_byte(dungeon_difficulty);
            battle_field_.MakePacket(
                tournament ? 386 : ENUM_NOTIPACKET_DUNGEON_INFO,
                &packet);
            const zombie_map_runtime::HookResult zombie_dungeon_info =
                zombie_map_runtime::RouteDungeonInfoPacket(this, packet);
            if (zombie_dungeon_info == zombie_map_runtime::kHookRejected) {
                return 1;
            }
            if (zombie_dungeon_info ==
                zombie_map_runtime::kHookNotManaged) {
                send_to_party(packet);
            }

            // CLIENT_IDA_EVIDENCE: NOTI29 consumes the currently installed
            // acceleration mode while accounting the first map.  Publish
            // NOTI487 before START_MAP so an unsupported/no-fatigue dungeon
            // cannot inherit the town state for one room.
            for (int seat = 0; seat <= 3; ++seat) {
                if (_checkValidUser(seat) == 1) {
                    fatigue_acceleration::SendDungeonState(
                        get_user(seat), dungeon, dungeon_index);
                }
            }

            packet.clear();
            battle_field_.MakePacket(
                tournament ? 387 : ENUM_NOTIPACKET_START_MAP,
                &packet);
            const zombie_map_runtime::HookResult zombie_route =
                zombie_map_runtime::RouteStartMapPacket(this, packet);
            if (zombie_route == zombie_map_runtime::kHookRejected) {
                return 1;
            }
            if (zombie_route == zombie_map_runtime::kHookNotManaged) {
                send_to_party(packet);
            }
            if (!tournament) {
                _Send_TOD_APCInfo(apc_index);
            }
        } else {
            packet.clear();
            packet.put_header(0, 212);
            packet.put_short(dungeon_index);
            packet.put_byte(dungeon_difficulty);
            battle_field_.MakePacket(212, &packet);
            send_to_party(packet);

            for (int seat = 0; seat <= 3; ++seat) {
                if (_checkValidUser(seat) == 1) {
                    fatigue_acceleration::SendDungeonState(
                        get_user(seat), dungeon, dungeon_index);
                }
            }

            packet.clear();
            battle_field_.MakePacket(211, &packet);
            send_to_party(packet);
            if (!battle_field_.setBloodState(
                    static_cast<ENUM_BLOOD_STATE>(2))) {
                champion_break::AbortDungeonRun(this);
                for (int seat = 0; seat <= 3; ++seat) {
                    if (_checkValidUser(seat) == 1) {
                        fatigue_acceleration::AbortDungeonRun(
                            get_user(seat));
                    }
                }
                time_spiral::ResetParty(this);
                return 1;
            }
        }
    } else {
        LogManager::logFormat(
            1,
            "party.cpp",
            "int CParty::dungeon_start(int, char, ENUM_DUNGEON_TYPE)",
            0x248,
            "[CMD16_DUNGEON_START_FAIL] stage=hell-seal-door dungeon=%d difficulty=%d mode=%d members=%d\n",
            dungeon_index,
            static_cast<int>(dungeon_difficulty),
            static_cast<int>(dungeon_mode),
            get_member_count());
        time_spiral::ResetParty(this);
        return 1;
    }

    const uint32_t tick = static_cast<uint32_t>(
        GlobalData::system_time() ?
            GlobalData::system_time()->getCurTickCount() : 0);
    battle_data_.map_tick_211() = tick;
    battle_data_.map_tick_212() = 0;
    battle_data_.map_pause_time() = 0;
    battle_data_.map_start_time() = 0;
    battle_data_.map_tick_214() = 0;
    battle_data_.map_tick_215() = 0;
    battle_data_.map_tick_216() = battle_data_.map_tick_211();
    battle_data_.map_tick_217() = battle_data_.map_tick_212();
    set_state(2);
    SetEPLPState(1);
    gen_timer_key(static_cast<TIMER_MESSAGE>(45));
    SetSelectedEPLPCmd(-1);

    const int member_count = get_member_count();
    setStartGamePartyCount(member_count);
    int secret_shop_npc = 1000;
    if (dungeon->get_blood_dungeon_type() == 0 &&
        manager->get_state() != 10 &&
        !dungeon->isTowerOfDespairDungeon() &&
        !dungeon->isTournamentDungeon() &&
        GlobalData::s_secret_shop) {
        secret_shop_npc = GlobalData::s_secret_shop->LotteryNpc(
            dungeon->get_index(), dungeon->get_standard_level(), member_count);
    }
    // The dungeon-clear path owns the item lottery and keys it with the NPC
    // selected here.  Keep that state on the party exactly as the 0627 server
    // did (CParty + 0x2a4); sending NOTI 276 alone only draws the NPC and leaves
    // the later item-list path closed.
    secret_shop_data_.state = secret_shop_npc;

    unsigned int premium_item_id = 0;
    CEventBase* const premium_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(65)
        : 0;
    if (premium_event && premium_event->IsEventing(0) == 1 &&
        IsReturnUserParty() == 1 &&
        isPremiumGoldCardReward(dungeon, premium_item_id) == 1) {
        SetPremiumGoldCardParty();
        SetPremiumGoldCardDefaultItem(premium_item_id);
    }

    result_recv_flag_.Clear();
    dungeon_clear_state_ = 0;
    for (int group = 0; group <= 1; ++group) {
        for (int seat = 0; seat <= 3; ++seat) {
            selected_cards_[group][seat] = 0xffu;
        }
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const user = get_user(seat);
        if (!user) {
            continue;
        }
        if (GlobalData::s_secret_shop) {
            GlobalData::s_secret_shop->SendSecretShopNpc(user, secret_shop_npc);
        }
        UserQuest* const quests = user->getCurCharacQuestW();
        if (quests) {
            quests->set_authen_data(static_cast<QUEST_CONDITION>(0),
                                    dungeon_index,
                                    dungeon_difficulty);
        }
        SetProperLevelDungeon(user);

        if (instance_mode != 1 &&
            dungeon->village_attack_revenge_dungeon_ != 1 &&
            dungeon->get_blood_dungeon_type() == 0 &&
            dungeon_index != 10000 &&
            dungeon->isNoFatigueDungeon() != 1 &&
            !racing_dungeon::ShouldSkipOrdinaryFatigue(this)) {
            if (dungeon->isTournamentDungeon()) {
                user->FatigueUp(dungeon->getTournamentRoundFatigue());
                user->SendFatigue();
            } else {
                fatigue_acceleration::ConsumeFatigue(
                    user, dungeon, dungeon_index);
            }
        }
        user->setPlayingDungeonIndex(dungeon_index);
        user->character_.PlayInfo::Reset();
        user->character_.party_member_count_ =
            static_cast<uint32_t>(member_count);
        user->character_statistic_.statistic_received_ = 0;

        // CParty +0x354..+0x357 are the embedded BattleData +0x28..+0x2b
        // bytes. Preserve the old per-member ordering: the standard-level
        // and dimension conditions only set the first pair, while the party
        // level-gap condition refreshes the second pair for each member.
        const int charac_level = user->get_charac_level();
        if (dungeon->get_standard_level() - 5 <= charac_level &&
            charac_level <= dungeon->get_standard_level() + 3) {
            battle_data_.dungeon_start_statistic_flag(0) = 1;
            battle_data_.dungeon_start_statistic_flag(1) = 1;
        }
        if (getMemberMaxLevelBeforeDungeon() -
                user->get_charac_level_before_dungeon() >
                6 &&
            member_count != 1) {
            battle_data_.dungeon_start_statistic_flag(2) = 1;
            battle_data_.dungeon_start_statistic_flag(3) =
                isGuildParty() == 0 &&
                FindUserByCharId(
                    static_cast<unsigned int>(
                        user->get_connect_upper_member())) == 0;
        } else {
            battle_data_.dungeon_start_statistic_flag(2) = 0;
            battle_data_.dungeon_start_statistic_flag(3) = 0;
        }
        if (dungeon->get_dimension_possible() > 0) {
            battle_data_.dungeon_start_statistic_flag(0) = 1;
            battle_data_.dungeon_start_statistic_flag(1) = 1;
        }

        if (manager->getMoveSpace() != 1) {
            GetInstanceDungeonEntranceLog()->IncrementDungeonEntrance(
                dungeon_index, instance_mode == 1);
        }
        if (instance_mode != 1) {
            user->IncCurCharacDungeonPlayCount();
        } else {
            CInventory* const inventory = user->getCurCharacInvenR();
            if (inventory && inventory->GetCreatureMgrW()) {
                inventory->GetCreatureMgrW()->TurnStomach(false);
            }
        }
        user->set_level_before_dungeon(
            static_cast<int16_t>(user->get_charac_level()));
        SetProperDungeonEvent(user, dungeon_index);
        SetEndRouting();
        if (user->getHades()) {
            user->getHades()->StdCateUpdate(2u, 0u);
        }
    }

    for (int group = 0; group <= 1; ++group) {
        for (int seat = 0; seat <= 3; ++seat) {
            play_result_values_[group][seat] = 0;
        }
    }
    Reset_party_overlapped_drop_ratio();
    check_passive_skill();
    send_loadingcheck_msg();
    CEventBase* const weekend_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(87)
        : 0;
    SetWeekendEvent(weekend_event && weekend_event->IsEventing(0) == 1);

    QuickParty::RewardData_Param reward_parameter(this);
    QuickParty::RewardData_Result reward_result;
    set_random_reward(static_cast<QuickParty::RandomBuffType>(9),
                      reward_parameter,
                      reward_result);
    SendPartyMemberTagChracInfo();
    return 0;
}

// IDA_EA: 0x85A2246
// IDA_NAME: _ZN6CParty14RestartDungeonEv
// DEMANGLED: CParty::RestartDungeon(void)
int CParty::RestartDungeon() {
    const int dungeon_index = battle_field_.get_dungeon_index();
    const char dungeon_difficulty =
        static_cast<char>(battle_field_.get_dungeon_diff());
    battle_field_.reset_field();
    return dungeon_start(dungeon_index,
                         dungeon_difficulty,
                         static_cast<ENUM_DUNGEON_TYPE>(0));
}
