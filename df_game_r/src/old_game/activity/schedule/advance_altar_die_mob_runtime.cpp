#include "old_game/activity/schedule/advance_altar_runtime.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace advancealtar {

// IDA_EA: 0x812DB00
int ProcStage::ProcSummonObjectMs::dieMob(unsigned short monster_id,
                                          unsigned short monster_type,
                                          const MSG_MONSTER_DIE& message) {
    (void)monster_type;
    std::map<int16_t, _SummonObjectMs*>::iterator it =
        summon_by_id.find(static_cast<int16_t>(monster_id));
    if (it == summon_by_id.end() || !it->second) {
        return 0;
    }

    _SummonObjectMs* const summon = it->second;
    if (summon->spawned_ != 1 || summon->killed_ == 1) {
        return 0;
    }

    // The original keeps the id-to-record map alive until ProcStage reset;
    // only the record's death bit changes here.  Erasing it would make the
    // scheduler/remaining-phase bookkeeping diverge after a repeated packet.
    summon->killed_ = 1;
    ++dead_count;
    if (character_data) {
        CUser* const user = character_data->getUser();
        if (user) {
            HistoryLog::killMonster(*user, summon->summon_object.monster_id,
                                    message);
        }
    }
    return 1;
}

// IDA_EA: 0x812D50A
bool ProcStage::ProcSummonObjectMs::isEndSummon() const {
    return static_cast<size_t>(next_index) >= summon_objects.size();
}

// IDA_EA: 0x8134820
int ProcStage::ProcSummonObjectMs::getAliveMonsterCount() const {
    return created_count - dead_count;
}

// IDA_EA: 0x812E1DE
int ProcStage::dieMob(unsigned short monster_id,
                      unsigned short monster_type,
                      const MSG_MONSTER_DIE& message) {
    if (summon_control.dieMob(monster_id, monster_type, message) == 1) {
        return 1;
    }

    std::map<int16_t, _MapMonsterSummary>::iterator it =
        created_monsters.find(static_cast<int16_t>(monster_id));
    if (it == created_monsters.end() || it->second.killed_ != 0) {
        return 0;
    }
    it->second.killed_ = 1;
    return 1;
}

// IDA_EA: 0x812EB5A
bool ProcStage::isEndAllPhase() const {
    return timeline &&
           static_cast<size_t>(current_phase_index) >= timeline->phases.size();
}

// IDA_EA: 0x812E2C6
int ProcStage::isEndStage(StageEndType::T& end_type) const {
    const bool end_all_phase = isEndAllPhase();
    const bool end_summon = summon_control.isEndSummon();
    const int alive_summons = summon_control.getAliveMonsterCount();
    int alive_map_monsters = 0;

    for (std::map<int16_t, _MapMonsterSummary>::const_iterator it =
             created_monsters.begin();
         it != created_monsters.end();
         ++it) {
        // IDA reads an unsigned word at pair+0x08.  The packed summary starts
        // at pair+0x02, so this is summary+0x06 (tower_index), not the
        // summary+0x04 runtime unique id.  PVF [center index] stores the tower
        // template/index (for example 64597), which must be compared unsigned
        // because it exceeds INT16_MAX.
        if (map_ && static_cast<uint16_t>(it->second.tower_index) ==
                        map_->script_value_178_ &&
            it->second.killed_ != 1) {
            ++alive_map_monsters;
        }
    }

    if (alive_map_monsters != 0) {
        if (end_all_phase && end_summon && alive_summons == 0) {
            end_type = static_cast<StageEndType::T>(3);
            return 1;
        }
        return 0;
    }

    end_type = static_cast<StageEndType::T>(0);
    return 1;
}

// IDA_EA: 0x8133B40
int HistoryLog::killMonster(CUser& user,
                            int monster_id,
                            const MSG_MONSTER_DIE& message) {
    return user.history_log().KillMonster(
        monster_id, message.field_a1e_, message.field_a1a_, message.field_a24_,
        message.field_a26_, message, 0, false, 0, 0,
        static_cast<eExpAddReason>(1));
}

// IDA_EA: 0x812EA00
int ProcStage::endProcAllKillMonster() {
    if (!character_data || !character_data->getUser()) {
        return 1;
    }
    if (static_cast<int>(getStageType()) != 1 ||
        startStage(getStageIndex(), getStageDifficulty()) != 0) {
        clearStageAndReward();
        return 1;
    }
    setState(static_cast<State::T>(3));
    return 0;
}

// IDA_EA: 0x8130BB8
int Manager::dieMob(CUser* user,
                    unsigned short monster_id,
                    unsigned short monster_type,
                    const MSG_MONSTER_DIE& message) {
    if (!user) {
        return 0;
    }
    StageControl* const control = user->getAdvanceAltar();
    if (!control) {
        return 1;
    }
    ProcStage* const stage = control->getProcStage();
    if (!stage) {
        return 1;
    }

    if (stage->dieMob(monster_id, monster_type, message) == 1) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
        packet.put_short(monster_id);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.finalize(true);
        user->Send(packet);
    }

    StageEndType::T end_type = static_cast<StageEndType::T>(0);
    if (stage->isEndStage(end_type) != 1 || !stage->isRewardState()) {
        return 1;
    }

    if (static_cast<int>(end_type) == 3) {
        if (!stage->endProcAllKillMonster()) {
            control->sendMapInfo();
            return 1;
        }
        stage->setState(static_cast<ProcStage::State::T>(5));
        Timer_StageControl::RegistTimerClearReward(control->getIndex(),
                                                   end_type);
        HistoryLog::dungeonClearInfo(*user, *control, true);
        return 1;
    }

    if (static_cast<int>(stage->getStageType()) == 1) {
        stage->setState(static_cast<ProcStage::State::T>(5));
        stage->clearStageAndReward();
        Timer_StageControl::RegistTimerClearReward(control->getIndex(),
                                                   end_type);
    } else {
        stage->setState(static_cast<ProcStage::State::T>(6));
        Timer_StageControl::RegistTimerEPLP(control->getIndex(), end_type);
        HistoryLog::dungeonClearInfo(*user, *control, false);
    }
    return 1;
}

}  // namespace advancealtar
