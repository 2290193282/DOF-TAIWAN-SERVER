#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <list>
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace advancealtar {

// IDA_EA: 0x812D378
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMsC2ERNS_10CharacDataERKi
ProcStage::ProcSummonObjectMs::ProcSummonObjectMs(
    CharacData& character_data_ref,
    const int& current_time_ms_ref)
    : next_index(0),
      summon_objects(),
      summon_by_id(),
      dead_count(0),
      created_count(0),
      character_data(&character_data_ref),
      current_time_ms(&current_time_ms_ref) {}

// IDA_EA: 0x812D3F0
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMsD2Ev
ProcStage::ProcSummonObjectMs::~ProcSummonObjectMs() {}

// IDA_EA: 0x812D43C
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs5resetEv
int ProcStage::ProcSummonObjectMs::reset() {
    next_index = 0;
    dead_count = 0;
    created_count = 0;
    for (std::vector<_SummonObjectMs*>::iterator it = summon_objects.begin();
         it != summon_objects.end();
         ++it) {
        // The original reset uses scalar operator delete directly; the
        // allocation site owns construction and has no separate dtor call.
        ::operator delete(*it);
    }
    summon_by_id.clear();
    summon_objects.clear();
    return 0;
}

// IDA_EA: 0x812D540
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs18insertSummonObjectERNS_15_SummonObjectMsE
void ProcStage::ProcSummonObjectMs::insertSummonObject(
    _SummonObjectMs& summon_object) {
    summon_objects.push_back(&summon_object);
    summon_by_id.insert(std::make_pair(
        static_cast<int16_t>(summon_object.monster_unique_id),
        &summon_object));
}

// IDA_EA: 0x812D5BA
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs22getLastInsertDataIndexEv
int ProcStage::ProcSummonObjectMs::getLastInsertDataIndex() {
    return static_cast<int>(summon_objects.size());
}

// IDA_EA: 0x812D5D0
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs24completePushSummonObjectEi
void ProcStage::ProcSummonObjectMs::completePushSummonObject(
    int last_insert_data_index) {
    if (last_insert_data_index <= 0 ||
        static_cast<int>(summon_objects.size()) <= last_insert_data_index) {
        return;
    }

    // This deliberately preserves the 0627 iterator boundary: IDA shows
    // operator[](last_insert_data_index) and operator[](size-1) passed to
    // std::sort, rather than a reconstructed begin()/end() range.
    std::sort(&summon_objects[last_insert_data_index],
              &summon_objects[summon_objects.size() - 1],
              ProcSummonObjectMs::compare);
}

// IDA_EA: 0x812D658
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs25summonObjectAndSendPacketEv
char ProcStage::ProcSummonObjectMs::summonObjectAndSendPacket() {
    const unsigned int record_count =
        static_cast<unsigned int>(summon_objects.size());
    while (static_cast<unsigned int>(next_index) < record_count) {
        _SummonObjectMs* const summon = summon_objects[next_index];
        if (summon) {
            if (summon->scheduled_time_ms > *current_time_ms) {
                return static_cast<char>(*current_time_ms);
            }
            if (!summon->spawned_) {
                ++created_count;
                summon->spawned_ = 1;

                NOTIPacketStruct::_NotiAdvanceAltarSpawnUnit notification;
                notification.monster_unique_id = summon->monster_unique_id;
                notification.summon_object = summon->summon_object;

                PacketGuard packet;
                packet.put_header(0, ENUM_NOTIPACKET_ADVANCE_ALTAR_SPAWN_UNIT);
                packet.put_binary(
                    reinterpret_cast<const char*>(&notification),
                    sizeof(notification));
                packet.finalize(true);
                character_data->getUser()->Send(packet);
            }
        }
        ++next_index;
    }
    return 0;
}

// IDA_EA: 0x812D804
// IDA_NAME: _ZNK12advancealtar9ProcStage18ProcSummonObjectMs8debugLogEv
void ProcStage::ProcSummonObjectMs::debugLog() const {
    for (std::vector<_SummonObjectMs*>::const_iterator it =
             summon_objects.begin();
         it != summon_objects.end(); ++it) {
        const _SummonObjectMs* const summon = *it;
        if (!summon) {
            continue;
        }
        cMyTrace trace(
            "void advancealtar::ProcStage::ProcSummonObjectMs::debugLog() const",
            320, 0);
        trace("\tVecotr Ms = %06d ActionDefineId=%d ObjectId=%d "
              "spawndId=%d isDead=%d ObjectType=%d ObjectIndex=%d\n",
              summon->scheduled_time_ms, summon->action_define_id,
              summon->summon_object.summon_object_id,
              summon->monster_unique_id, summon->killed_,
              summon->summon_object.summon_type,
              summon->summon_object.monster_id);
    }

    for (std::map<int16_t, _SummonObjectMs*>::const_iterator it =
             summon_by_id.begin();
         it != summon_by_id.end(); ++it) {
        const _SummonObjectMs* const summon = it->second;
        if (!summon) {
            continue;
        }
        cMyTrace trace(
            "void advancealtar::ProcStage::ProcSummonObjectMs::debugLog() const",
            333, 0);
        trace("\tMap Ms = %06d ActionDefineId=%d ObjectId=%d "
              "spawndId=%d isDead=%d ObjectType=%d ObjectIndex=%d\n",
              summon->scheduled_time_ms, summon->action_define_id,
              summon->summon_object.summon_object_id,
              summon->monster_unique_id, summon->killed_,
              summon->summon_object.summon_type,
              summon->summon_object.monster_id);
    }
}

// IDA_EA: 0x812DAEC
// IDA_NAME: _ZN12advancealtar9ProcStage18ProcSummonObjectMs7compareEPKNS_15_SummonObjectMsES4_
bool ProcStage::ProcSummonObjectMs::compare(
    const _SummonObjectMs* left,
    const _SummonObjectMs* right) {
    return left->scheduled_time_ms < right->scheduled_time_ms;
}

// IDA_EA: 0x812DC18
// IDA_NAME: _ZN12advancealtar9ProcStageC2ERNS_10CharacDataE
ProcStage::ProcStage(CharacData& character_data_ref)
    : character_data(&character_data_ref),
      state(7),
      current_time_ms(0),
      current_phase_index(0),
      next_phase_begin_time_ms(0),
      next_created_monster_id(0),
      summon_control(character_data_ref, current_time_ms),
      created_monsters(),
      stage_difficulty(-1),
      map_basis_level(0),
      dungeon_(0),
      map_(0),
      survival_mode_first_round(1),
      current_survival_round(0),
      timeline_actions(),
      start_stage_time(0),
      timeline(0) {}

// IDA_EA: 0x812DD26
// IDA_NAME: _ZN12advancealtar9ProcStageD2Ev
ProcStage::~ProcStage() {}

// IDA_EA: 0x812DD9A
// IDA_NAME: _ZN12advancealtar9ProcStage8resetAllEv
int ProcStage::resetAll() {
    dungeon_ = 0;
    start_stage_time = 0;
    reset(0);
    return reset(1);
}

// IDA_EA: 0x812DDE0
// IDA_NAME: _ZN12advancealtar9ProcStage5resetENS_9StageType1TE
int ProcStage::reset(int reset_type) {
    current_time_ms = 0;
    current_phase_index = 0;
    next_phase_begin_time_ms = 0;
    stage_difficulty = -1;
    map_ = 0;
    summon_control.reset();
    if (reset_type != 1) {
        state = 7;
        next_created_monster_id = 0;
        created_monsters.clear();
        survival_mode_first_round = 1;
        current_survival_round = 0;
        timeline_actions.clear();
    }
    return reset_type;
}

// IDA_EA: 0x812E0D6
// IDA_NAME: _ZN12advancealtar9ProcStage12procTimeLineEv
// DEMANGLED: advancealtar::ProcStage::procTimeLine(void)
char ProcStage::procTimeLine() {
    char result = static_cast<char>(state);
    if (state == 3) {
        if (summon_control.isEndSummon() &&
            makeCurrentPhaseSummonObject() != 1) {
            state = 4;
        }
        incCurrentMs();
        result = static_cast<char>(state);
        if (state == 3) {
            result = summon_control.summonObjectAndSendPacket();
        }
    }
    return result;
}

// IDA_EA: 0x812E146
// IDA_NAME: _ZN12advancealtar9ProcStage12incCurrentMsEv
// DEMANGLED: advancealtar::ProcStage::incCurrentMs(void)
ProcStage* ProcStage::incCurrentMs() {
    current_time_ms += 1000 * Manager::gmTimeSpeed_;
    return this;
}

// IDA_EA: 0x812EBA8
// IDA_NAME: _ZN12advancealtar9ProcStage28makeCurrentPhaseSummonObjectEv
// DEMANGLED: advancealtar::ProcStage::makeCurrentPhaseSummonObject(void)
int ProcStage::makeCurrentPhaseSummonObject() {
    if (!timeline || isEndAllPhase()) {
        return 0;
    }

    const Phase& phase = timeline->phases[current_phase_index];
    ++current_phase_index;

    const int alive_monster_count =
        summon_control.getAliveMonsterCount();
    const int phase_duration_ms = phase.duration_ms;
    const int phase_end_time_ms =
        next_phase_begin_time_ms + phase_duration_ms;

    const _ActionAssign* default_assign = 0;
    const _ActionAssign* selected_assign = 0;
    for (std::vector<_ActionAssign>::const_iterator it =
             phase.action_assigns.begin();
         it != phase.action_assigns.end(); ++it) {
        if (it->first_frame == static_cast<int16_t>(-1) &&
            it->last_frame == static_cast<int16_t>(-1)) {
            default_assign = &*it;
        } else if (it->first_frame <= alive_monster_count &&
                   it->last_frame >= alive_monster_count) {
            selected_assign = &*it;
        }
    }
    if (!selected_assign) {
        selected_assign = default_assign;
    }
    if (!selected_assign) {
        return 0;
    }

    std::map<int, int> repeated_monster_count;
    int last_insert_data_index = -1;
    const ActionDefine* const action_define =
        selected_assign->action_define;
    if (action_define) {
        for (std::vector<_Action>::const_iterator action_it =
                 action_define->actions.begin();
             action_it != action_define->actions.end(); ++action_it) {
            const _SummonObject* const summon_data =
                action_it->summon_data;
            if (!summon_data) {
                continue;
            }

            int summon_time_ms =
                next_phase_begin_time_ms + action_it->first_delay_ms;
            while (summon_time_ms < phase_end_time_ms) {
                bool should_create = true;
                if (action_it->has_repeat_count) {
                    std::map<int, int>::iterator repeated =
                        repeated_monster_count.find(summon_data->monster_id);
                    if (repeated == repeated_monster_count.end()) {
                        repeated_monster_count.insert(std::make_pair(
                            summon_data->monster_id, 1));
                    } else if (repeated->second >=
                               action_it->repeat_count) {
                        should_create = false;
                    } else {
                        ++repeated->second;
                    }
                }

                if (should_create) {
                    _SummonObjectMs* const summon =
                        new _SummonObjectMs();
                    summon->scheduled_time_ms = summon_time_ms;
                    summon->monster_unique_id =
                        static_cast<uint16_t>(next_created_monster_id);
                    ++next_created_monster_id;
                    summon->action_define_id =
                        action_define->action_define_id;
                    summon->killed_ = 0;
                    summon->summon_object = *summon_data;
                    summon_control.insertSummonObject(*summon);
                    summon_time_ms += action_it->next_delay_ms;
                    if (last_insert_data_index == -1) {
                        last_insert_data_index =
                            summon_control.getLastInsertDataIndex();
                    }
                } else {
                    summon_time_ms += action_it->next_delay_ms;
                }
            }
        }
    }

    summon_control.completePushSummonObject(last_insert_data_index);
    next_phase_begin_time_ms += phase_duration_ms;
    summon_control.debugLog();
    return 1;
}

// IDA_EA: 0x812EAA6
// IDA_NAME: _ZN12advancealtar9ProcStage5pauseEb
// DEMANGLED: advancealtar::ProcStage::pause(bool)
int ProcStage::pause(bool paused) {
    if (paused) {
        // Repeated pause packets are valid acknowledgements of the current
        // state.  Treating them as failures lets CMD647 echo a state which was
        // never applied and can leave the 0828 client input controller frozen.
        if (state == 2) {
            return 1;
        }
        if (state != 3) {
            return 0;
        }
        state = 2;
    } else {
        // Resume is idempotent for the same reason.  State 1 is the original
        // pre-timeline wait state and still performs the official 1 -> 3
        // transition on the first resume.
        if (state == 3) {
            return 1;
        }
        if (state != 2 && state != 1) {
            return 0;
        }
        state = 3;
    }
    return 1;
}

bool ProcStage::isPaused() const {
    return state == 2;
}

// IDA_EA: 0x812DE70
// IDA_NAME: _ZN12advancealtar9ProcStage10startStageEiNS_15StageDifficulty1TE
// DEMANGLED: advancealtar::ProcStage::startStage(int,advancealtar::StageDifficulty::T)
int ProcStage::startStage(int requested_stage_index, int difficulty) {
    CUser* const user = character_data ? character_data->getUser() : 0;
    if (!user) {
        return 3;
    }

    dungeon_ = G_CDataManager()->find_dungeon(requested_stage_index);
    if (!dungeon_) {
        return 21;
    }

    int map_index = 0;
    if (getStageType() == StageType::SURVIVAL) {
        reset(1);
        if (isSurvivalModeFirstRound()) {
            timeline_actions = dungeon_->dungeon_runtime_values_;
            std::random_shuffle(timeline_actions.begin(), timeline_actions.end());
            state = 1;
            start_stage_time = GlobalData::system_time()
                                   ? GlobalData::system_time()->getCurSec()
                                   : 0;
            HistoryLog::enterDungeon(*user, *this);
        } else {
            ++current_survival_round;
        }

        if (static_cast<unsigned int>(current_survival_round) >=
            timeline_actions.size()) {
            state = 7;
            return 21;
        }
        map_index = timeline_actions[current_survival_round];
        map_basis_level = current_survival_round;
    } else {
        reset(0);
        stage_difficulty = difficulty;
        state = 1;
        map_index = dungeon_->script_values_808_[difficulty];
        map_basis_level = dungeon_->script_values_820_[difficulty];
        start_stage_time = GlobalData::system_time()
                               ? GlobalData::system_time()->getCurSec()
                               : 0;
        HistoryLog::enterDungeon(*user, *this);
    }

    map_ = G_CDataManager()->find_map(map_index);
    if (!map_) {
        return 21;
    }
    timeline = &map_->advance_altar_timeline_;
    return 0;
}

// IDA_EA: 0x812EB04
// IDA_NAME: _ZNK12advancealtar9ProcStage12getStageTypeEv
// DEMANGLED: advancealtar::ProcStage::getStageType(void)const
StageType::T ProcStage::getStageType() const {
    return dungeon_
               ? static_cast<StageType::T>(dungeon_->advance_altar_stage_type_)
               : StageType::NORMAL;
}

// IDA_EA: 0x812EB26
// IDA_NAME: _ZNK12advancealtar9ProcStage13getStageIndexEv
// DEMANGLED: advancealtar::ProcStage::getStageIndex(void)const
int ProcStage::getStageIndex() const {
    return dungeon_ ? dungeon_->get_index() : 0;
}

// IDA_EA: 0x812F770
// IDA_NAME: _ZNK12advancealtar9ProcStage13isRewardStateEv
// DEMANGLED: advancealtar::ProcStage::isRewardState(void)const
bool ProcStage::isRewardState() const {
    return state == static_cast<int>(3) || state == static_cast<int>(4);
}

// IDA_EA: 0x8134838
// IDA_NAME: _ZNK12advancealtar9ProcStage8getStateEv
// DEMANGLED: advancealtar::ProcStage::getState(void)const
ProcStage::State::T ProcStage::getState() const {
    return static_cast<State::T>(state);
}

// IDA_EA: 0x8134844
// IDA_NAME: _ZN12advancealtar9ProcStage8setStateENS0_5State1TE
// DEMANGLED: advancealtar::ProcStage::setState(advancealtar::ProcStage::State::T)
ProcStage* ProcStage::setState(State::T state_value) {
    state = static_cast<int>(state_value);
    return this;
}

// IDA_EA: 0x812F70C
// IDA_NAME: _ZNK12advancealtar9ProcStage24isSurvivalModeFirstRoundEv
// DEMANGLED: advancealtar::ProcStage::isSurvivalModeFirstRound(void)const
bool ProcStage::isSurvivalModeFirstRound() const {
    return getStageType() == StageType::SURVIVAL &&
           survival_mode_first_round != 0;
}

// IDA_EA: 0x812F74C
// IDA_NAME: _ZN12advancealtar9ProcStage28setNotSurvivalModeFirstRoundEv
// DEMANGLED: advancealtar::ProcStage::setNotSurvivalModeFirstRound(void)
bool ProcStage::setNotSurvivalModeFirstRound() {
    const bool survival = getStageType() == StageType::SURVIVAL;
    if (survival) {
        survival_mode_first_round = 0;
    }
    return survival;
}

// IDA_EA: 0x8134852
// IDA_NAME: _ZNK12advancealtar9ProcStage11getTimeLineEv
// DEMANGLED: advancealtar::ProcStage::getTimeLine(void)const
const TimeLine* ProcStage::getTimeLine() const {
    return timeline;
}

// IDA_EA: 0x8134860
// IDA_NAME: _ZNK12advancealtar9ProcStage6getMapEv
// DEMANGLED: advancealtar::ProcStage::getMap(void)const
CMap* ProcStage::getMap() const {
    return map_;
}

// IDA_EA: 0x813486C
// IDA_NAME: _ZNK12advancealtar9ProcStage18getStageDifficultyEv
// DEMANGLED: advancealtar::ProcStage::getStageDifficulty(void)const
int ProcStage::getStageDifficulty() const {
    return stage_difficulty;
}

// IDA_EA: 0x8134878
// IDA_NAME: _ZNK12advancealtar9ProcStage25getCurrentSurvivalRoundNoEv
// DEMANGLED: advancealtar::ProcStage::getCurrentSurvivalRoundNo(void)const
int ProcStage::getCurrentSurvivalRoundNo() const {
    return current_survival_round + 1;
}

// IDA_EA: 0x812EB4E
// IDA_NAME: _ZNK12advancealtar9ProcStage23getClearSurvivalRoundNoEv
// DEMANGLED: advancealtar::ProcStage::getClearSurvivalRoundNo(void)const
int ProcStage::getClearSurvivalRoundNo() const {
    return current_survival_round;
}

// IDA_EA: 0x8134886
// IDA_NAME: _ZNK12advancealtar9ProcStage17getStartStageTimeEv
// DEMANGLED: advancealtar::ProcStage::getStartStageTime(void)const
int ProcStage::getStartStageTime() const {
    return start_stage_time;
}

// IDA_EA: 0x8134894
// IDA_NAME: _ZNK12advancealtar9ProcStage16getMapBasisLevelEv
// DEMANGLED: advancealtar::ProcStage::getMapBasisLevel(void)const
int ProcStage::getMapBasisLevel() const {
    return map_basis_level;
}

// IDA_EA: 0x812E16E
// IDA_NAME: _ZN12advancealtar9ProcStage20addCreatedMapMonsterERNS_18_MapMonsterSummaryE
// DEMANGLED: advancealtar::ProcStage::addCreatedMapMonster(advancealtar::_MapMonsterSummary &)
int ProcStage::addCreatedMapMonster(_MapMonsterSummary& summary) {
    summary.unique_id = static_cast<uint16_t>(next_created_monster_id);
    ++next_created_monster_id;
    return created_monsters.insert(
        std::make_pair(static_cast<int16_t>(summary.unique_id), summary))
        .second;
}

// IDA_EA: 0x812F9A2
// IDA_NAME: _ZN12advancealtar12StageControlC2Ev
StageControl::StageControl()
    : stage_control_index(-1),
      // IDA calls ProcStage C2 with the later +0x98 CharacData member before
      // its own C2.  Naming that real member preserves the observed pointer
      // relation without a raw storage or address-layout substitute.
      proc_stage(charac_data),
      charac_data() {}

// IDA_EA: 0x812FA06
// IDA_NAME: _ZN12advancealtar12StageControlD2Ev
StageControl::~StageControl() {}

// IDA_EA: 0x812FA54
// IDA_NAME: _ZN12advancealtar12StageControl5resetEv
int StageControl::reset() {
    proc_stage.resetAll();
    return charac_data.reset();
}

// IDA_EA: 0x812FA7A
// IDA_NAME: _ZN12advancealtar12StageControl11selectStageEP5CUseriNS_15StageDifficulty1TE
// DEMANGLED: advancealtar::StageControl::selectStage(CUser *,int,advancealtar::StageDifficulty::T)
int StageControl::selectStage(CUser* user,
                              int requested_stage_index,
                              int difficulty) {
    if (!user) {
        return 3;
    }
    if (!user->advance_altar_manager_.isAvailableTciket()) {
        return 245;
    }
    if (!user->advance_altar_manager_.isEnterStage(requested_stage_index)) {
        return 8;
    }

    charac_data.updated =
        user->advance_altar_manager_.getCurrentRidableId();
    charac_data.setUser(user);
    charac_data.setSlotItem();
    proc_stage.resetAll();
    const int started =
        proc_stage.startStage(requested_stage_index, difficulty);
    if (started) {
        return started;
    }
    sendStageInfo();
    sendMapInfo();
    proc_stage.setNotSurvivalModeFirstRound();
    user->set_state(13);
    return 0;
}

// IDA_EA: 0x812FC4E
// IDA_NAME: _ZN12advancealtar12StageControl9leaveUserEv
// DEMANGLED: advancealtar::StageControl::leaveUser(void)
int StageControl::leaveUser() {
    CUser* const user = charac_data.getUser();
    if (!user) {
        destroy();
        return 0;
    }
    if (!user->getCurCharacR()) {
        // Character teardown can reach this path after the selected record
        // was detached.  No world/inventory packets are safe, but ownership
        // and the pool node still must be released.
        user->setAdvanceAltarIndex(-1);
        destroy();
        return 1;
    }

    HistoryLog::leaveDungeon(*user, proc_stage);
    GameWorld* const game_world = G_GameWorld();
    game_world->out_from_dungeon(user);

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    game_world->send_all(packet, user);
    user->send_itemspace(0);
    user->getCurCharacInvenR()->SendItemLockListInven();
    user->setAdvanceAltarIndex(-1);
    destroy();
    return 1;
}

// IDA_EA: 0x81300CE
// IDA_NAME: _ZN12advancealtar12StageControl7destroyEv
// DEMANGLED: advancealtar::StageControl::destroy(void)
int StageControl::destroy() {
    CUser* const owner = charac_data.getUser();
    const int owned_index = stage_control_index;
    if (owner && owner->getAdvanceAltarIndex() == owned_index) {
        owner->setAdvanceAltarIndex(-1);
    }
    reset();
    CGameManager* const manager = G_CGameManager();
    if (manager) {
        manager->returnAdvanceAltar(this);
    }
    return 0;
}

// IDA_EA: 0x81300F6
// IDA_NAME: _ZN12advancealtar12StageControl13sendStageInfoEv
// DEMANGLED: advancealtar::StageControl::sendStageInfo(void)
void StageControl::sendStageInfo() {
    CUser* const user = charac_data.getUser();
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_ADVANCE_ALTAR_STAGE_INFO);
    packet.put_int(proc_stage.getStageIndex());
    packet.put_int(proc_stage.getStageDifficulty());
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81307A6
// IDA_NAME: _ZNK12advancealtar12StageControl14getCenterCountEv
// DEMANGLED: advancealtar::StageControl::getCenterCount(void)const
int StageControl::getCenterCount() const {
    CMap* const map = proc_stage.getMap();
    CUser* const user = charac_data.getUser();
    if (!map || !user) {
        return 0;
    }

    int count = 0;
    const std::list<_mapMonster> monsters = map->get_monster_list();
    for (std::list<_mapMonster>::const_iterator it = monsters.begin();
         it != monsters.end(); ++it) {
        int tower_index = 0;
        int tower_level = 0;
        if (user->advance_altar_manager_.getTowerInfo(
                it->map_index, tower_index, tower_level) == 1) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x81301E0
// IDA_NAME: _ZN12advancealtar12StageControl11sendMapInfoEv
// DEMANGLED: advancealtar::StageControl::sendMapInfo(void)
void StageControl::sendMapInfo() {
    CUser* const user = charac_data.getUser();
    CMap* const map = proc_stage.getMap();
    const TimeLine* const timeline = proc_stage.getTimeLine();
    if (!user || !map || !timeline) {
        return;
    }

    NOTIPacketStruct::_NotiAdvanceAltarMapInfo map_info;
    map_info.stage_type = proc_stage.getStageType();
    map_info.survival_round_no = proc_stage.getCurrentSurvivalRoundNo();
    map_info.map_index = map->get_index();
    map_info.timeline_total_duration_ms =
        timeline->summary.total_duration_ms;
    map_info.map_basis_level = proc_stage.getMapBasisLevel();

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_ADVANCE_ALTAR_START_MAP);
    packet.put_binary(reinterpret_cast<const char*>(&map_info),
                      sizeof(map_info));
    packet.put_int(static_cast<int>(timeline->summary.phase_summary.size()));
    for (std::vector<_PhaseSummary>::const_iterator it =
             timeline->summary.phase_summary.begin();
         it != timeline->summary.phase_summary.end(); ++it) {
        packet.put_binary(reinterpret_cast<const char*>(&*it), sizeof(*it));
    }

    if (proc_stage.getStageType() != StageType::SURVIVAL ||
        proc_stage.isSurvivalModeFirstRound()) {
        const std::list<_mapMonster> monsters = map->get_monster_list();
        packet.put_int(getCenterCount());
        for (std::list<_mapMonster>::const_iterator it = monsters.begin();
             it != monsters.end(); ++it) {
            _MapMonsterSummary summary;
            summary.map_monster_id = it->raw_2c;
            int tower_index = 0;
            int tower_level = 0;
            if (user->advance_altar_manager_.getTowerInfo(
                    it->map_index, tower_index, tower_level) != 1) {
                continue;
            }
            summary.tower_index = static_cast<int16_t>(tower_index);
            summary.tower_level = static_cast<uint8_t>(tower_level);
            summary.map_value_18 = static_cast<uint8_t>(it->raw_18);
            summary.map_value_20 = static_cast<uint8_t>(it->raw_20);
            summary.map_value_1c = it->raw_1c;
            proc_stage.addCreatedMapMonster(summary);
            packet.put_binary(reinterpret_cast<const char*>(&summary),
                              sizeof(summary));
        }
    } else {
        packet.put_int(0);
    }

    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x812FB98
// IDA_NAME: _ZN12advancealtar12StageControl16onTimerStageTickEv
// DEMANGLED: advancealtar::StageControl::onTimerStageTick(void)
int StageControl::onTimerStageTick() {
    if (proc_stage.getState() == static_cast<ProcStage::State::T>(3) ||
        proc_stage.getState() == static_cast<ProcStage::State::T>(4)) {
        charac_data.upGage();
        proc_stage.procTimeLine();
    }
    return 1;
}

// IDA_EA: 0x812FBFA
// IDA_NAME: _ZN12advancealtar12StageControl7onTimerE13TIMER_MESSAGENS_12StageEndType1TE
// DEMANGLED: advancealtar::StageControl::onTimer(TIMER_MESSAGE,advancealtar::StageEndType::T)
int StageControl::onTimer(int timer_message, StageEndType::T end_type) {
    switch (timer_message) {
    case 168:
        return onClearReward(end_type);
    case 169:
        return onEplp(end_type);
    case 170:
        return onEplpReturnVillage();
    default:
        return timer_message;
    }
}

// IDA_EA: 0x81305F6
// IDA_NAME: _ZN12advancealtar12StageControl13onClearRewardENS_12StageEndType1TE
// DEMANGLED: advancealtar::StageControl::onClearReward(advancealtar::StageEndType::T)
int StageControl::onClearReward(StageEndType::T end_type) {
    if (static_cast<int>(end_type) == 3) {
        proc_stage.setState(static_cast<ProcStage::State::T>(6));
        return Timer_StageControl::RegistTimerEPLP(stage_control_index,
                                                   end_type);
    }
    return static_cast<int>(end_type);
}

// IDA_EA: 0x8130644
// IDA_NAME: _ZN12advancealtar12StageControl6onEplpENS_12StageEndType1TE
// DEMANGLED: advancealtar::StageControl::onEplp(advancealtar::StageEndType::T)
int StageControl::onEplp(StageEndType::T end_type) {
    sendEplp(end_type);
    return Timer_StageControl::RegistTimerEPLPReturnVillage(
        stage_control_index);
}

// IDA_EA: 0x8130692
// IDA_NAME: _ZN12advancealtar12StageControl19onEplpReturnVillageEv
// DEMANGLED: advancealtar::StageControl::onEplpReturnVillage(void)
int StageControl::onEplpReturnVillage() {
    if (proc_stage.getState() == static_cast<ProcStage::State::T>(6)) {
        return leaveUser();
    }
    return 0;
}

// IDA_EA: 0x81306BE
// IDA_NAME: _ZN12advancealtar12StageControl8sendEplpENS_12StageEndType1TE
// DEMANGLED: advancealtar::StageControl::sendEplp(advancealtar::StageEndType::T)
void StageControl::sendEplp(StageEndType::T end_type) {
    CUser* const user = charac_data.getUser();
    if (!user) {
        return;
    }

    NOTIPacketStruct::_NotiAdvanceAltarEplpState notification;
    notification.end_type = static_cast<int>(end_type);
    notification.has_available_ticket =
        user->advance_altar_manager_.isAvailableTciket();

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_ADVANCE_ALTAR_EPLP_STATE);
    packet.put_binary(reinterpret_cast<const char*>(&notification),
                      sizeof(notification));
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81348AE
// IDA_NAME: _ZN12advancealtar12StageControl12getProcStageEv
// DEMANGLED: advancealtar::StageControl::getProcStage(void)
ProcStage* StageControl::getProcStage() {
    return &proc_stage;
}

// IDA_EA: 0x81348BA
// IDA_NAME: _ZNK12advancealtar12StageControl8getIndexEv
__attribute__((used, noinline)) int StageControl::getIndex() const {
    return stage_control_index;
}

// IDA_EA: 0x82A669C
// IDA_NAME: _ZN12advancealtar12StageControl8setIndexEi
__attribute__((used, noinline)) StageControl* StageControl::setIndex(int index) {
    stage_control_index = index;
    return this;
}

// IDA_EA: 0x812FDD4
// IDA_NAME: _ZN12advancealtar12StageControl9gotoStageEP5CUser
// DEMANGLED: advancealtar::StageControl::gotoStage(CUser *)
void StageControl::gotoStage(CUser* user) {
    GameWorld* const game_world = G_GameWorld();
    Village* const village = game_world->getUserVillage(user);
    if (!village) {
        cMyTrace trace(
            "static void advancealtar::StageControl::gotoStage(CUser*)",
            1413, 5);
        trace("%d", user->getCurCharacVill());
        return;
    }

    Area* const area = village->getArea(user->get_area(false));
    if (!area) {
        cMyTrace trace(
            "static void advancealtar::StageControl::gotoStage(CUser*)",
            1420, 5);
        trace("%d", user->get_area(false));
        return;
    }

    area->erase_user(user);
    PacketGuard packet;
    packet.put_header(0, 23);
    packet.put_short(user->get_unique_id());
    packet.put_byte(user->getCurCharacVill());
    packet.put_byte(0xff);
    packet.put_short(user->get_posX());
    packet.put_short(user->get_posY());
    packet.put_byte(user->get_direction());
    packet.put_byte(user->get_charac_visible_values());
    packet.finalize(true);
    user->Send(packet);

    packet.clear();
    packet.put_header(0, 3);
    packet.put_byte(1);
    packet.put_short(user->get_unique_id());
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    G_GameWorld()->send_all(packet, user);
    user->set_state(13);
}

}  // namespace advancealtar
