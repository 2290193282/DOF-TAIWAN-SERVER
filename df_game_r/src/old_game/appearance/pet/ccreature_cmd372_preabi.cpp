#include "old_game/appearance/pet/creature.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

namespace user_creature {

// IDA_EA: 0x833F258
// IDA_NAME: _ZN13user_creature15CCreatureScript20GetSkillRecoveryTimeEv
// DEMANGLED: user_creature::CCreatureScript::GetSkillRecoveryTime(void)
int CCreatureScript::GetSkillRecoveryTime() {
    return skill_recovery_time;
}

// IDA_EA: 0x833F264
// IDA_NAME: _ZN13user_creature15CCreatureScript24GetOverSkillRecoveryTimeEv
// DEMANGLED: user_creature::CCreatureScript::GetOverSkillRecoveryTime(void)
int CCreatureScript::GetOverSkillRecoveryTime() {
    return over_skill_recovery_time;
}

// IDA_EA: 0x833F270
// IDA_NAME: _ZN13user_creature15CCreatureScript22GetLearnOverSkillLevelEv
// DEMANGLED: user_creature::CCreatureScript::GetLearnOverSkillLevel(void)
int CCreatureScript::GetLearnOverSkillLevel() {
    return static_cast<unsigned char>(learn_overskill_level);
}

// IDA_EA: 0x8338C28
// IDA_NAME: _ZN13user_creature9CCreature20GetSkillRecoveryTimeEv
// DEMANGLED: user_creature::CCreature::GetSkillRecoveryTime(void)
int CCreature::GetSkillRecoveryTime() {
    CCreatureScript* const script = CCreatureCmd372View(*this).creature_script;
    return script ? script->GetSkillRecoveryTime() : -1;
}

// IDA_EA: 0x8338C50
// IDA_NAME: _ZN13user_creature9CCreature24GetOverSkillRecoveryTimeEv
// DEMANGLED: user_creature::CCreature::GetOverSkillRecoveryTime(void)
int CCreature::GetOverSkillRecoveryTime() {
    CCreatureScript* const script = CCreatureCmd372View(*this).creature_script;
    return script ? script->GetOverSkillRecoveryTime() : -1;
}

// IDA_EA: 0x8338C78
// IDA_NAME: _ZN13user_creature9CCreature22GetLearnOverSkillLevelEv
// DEMANGLED: user_creature::CCreature::GetLearnOverSkillLevel(void)
int CCreature::GetLearnOverSkillLevel() {
    CCreatureScript* const script = CCreatureCmd372View(*this).creature_script;
    return script ? script->GetLearnOverSkillLevel() : -1;
}

// IDA_EA: 0x833C41E
// IDA_NAME: _ZN13user_creature12CCreatureMgr16SetOverSkillTimeERKi
// DEMANGLED: user_creature::CCreatureMgr::SetOverSkillTime(int const&)
int CCreatureMgr::SetOverSkillTime(const int& item_id) {
    CCreature* const creature = active_creature_;
    if (!creature) {
        return 0;
    }
    if (creature->IsDieCreature()) {
        return 1;
    }

    CCreatureCmd372Layout& creature_layout = CCreatureCmd372View(*creature);
    // CCreature begins with the real polymorphic CCreatureItem base.  The
    // CMD372 layout deliberately keeps that range opaque to preserve its
    // checked offsets, so dispatch through the actual base instead of
    // treating the byte view as a fabricated CCreatureItem object.
    if (item_id != creature->GetItemId()) {
        return 0;
    }
    if (!creature_layout.creature_script) {
        cMyTrace trace(
            "int user_creature::CCreatureMgr::SetOverSkillTime(const int&)",
            3682,
            0);
        trace("SetOverSkillTime::creature_script is null(creature_itemno=%u)",
              static_cast<unsigned int>(item_id));
        return 19;
    }

    const int learn_level = creature->GetLearnOverSkillLevel();
    if (learn_level > creature->GetExpLevel(creature->GetExp())) {
        return 19;
    }

    COverSkillTime& over_skill_time = creature_layout.over_skill_time;
    const int last_used_time = over_skill_time.GetTime()->current_time();
    const int skill_interval = creature->GetSkillRecoveryTime() / 1000;
    const int over_skill_interval =
        creature->GetOverSkillRecoveryTime() / 1000;
    const int deadline = last_used_time + skill_interval + over_skill_interval;
    const int current_time = GlobalData::system_time()->getCurSec();
    if (deadline > current_time + 2) {
        const unsigned int account_id =
            static_cast<unsigned int>(owner_->get_acc_id());
        const unsigned int charac_no =
            static_cast<unsigned int>(owner_->getCurCharacNo());
        cMyTrace trace(
            "int user_creature::CCreatureMgr::SetOverSkillTime(const int&)",
            3692,
            0);
        trace("fail:cur_time(%u),last_used_skilltime(%u),interval(%d) "
              "st(%d),ovst(%d),m_id(%s),ch_no(%u)",
              static_cast<unsigned int>(current_time),
              static_cast<unsigned int>(last_used_time),
              skill_interval + over_skill_interval,
              skill_interval,
              over_skill_interval,
              NumberToString(account_id, 0),
              charac_no);
        return 19;
    }

    over_skill_time.SetTime();
    return 0;
}

}  // namespace user_creature
