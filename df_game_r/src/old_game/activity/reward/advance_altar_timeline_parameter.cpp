#include "old_game/activity/reward/advance_altar_timeline_parameter.h"

#include <stdio.h>
#include <string.h>

namespace advancealtar {

// IDA_EA: 0x889B954
// IDA_NAME: _ZN12advancealtar13_PhaseSummaryC2Ev
// DEMANGLED: advancealtar::_PhaseSummary::_PhaseSummary(void)
_PhaseSummary::_PhaseSummary() {
    memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x889EBC0
// IDA_NAME: _ZN12advancealtar13_PhaseSummaryD2Ev
// DEMANGLED: advancealtar::_PhaseSummary::~_PhaseSummary()
_PhaseSummary::~_PhaseSummary() {}

// IDA_EA: 0x8134650
// IDA_NAME: _ZN12advancealtar13_SummonObjectC2Ev
// DEMANGLED: advancealtar::_SummonObject::_SummonObject(void)
_SummonObject::_SummonObject() {
    memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x8134674
// IDA_NAME: _ZN12advancealtar15_SummonObjectMsC2Ev
// DEMANGLED: advancealtar::_SummonObjectMs::_SummonObjectMs(void)
_SummonObjectMs::_SummonObjectMs() {
    memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x889B978
// IDA_NAME: _ZN12advancealtar7_ActionC2Ev
// DEMANGLED: advancealtar::_Action::_Action(void)
_Action::_Action() {
    memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x889F056
// IDA_NAME: _ZN12advancealtar7_ActionD2Ev
// DEMANGLED: advancealtar::_Action::~_Action()
_Action::~_Action() {}

// IDA_EA: 0x8899C62
// IDA_NAME: _ZN12advancealtar12ActionDefineC2Ev
// DEMANGLED: advancealtar::ActionDefine::ActionDefine(void)
ActionDefine::ActionDefine()
    : action_define_id(0),
      actions() {}

// IDA_EA: 0x889C264
// IDA_NAME: _ZN12advancealtar12ActionDefineC2ERKS0_
// DEMANGLED: advancealtar::ActionDefine::ActionDefine(ActionDefine const&)
ActionDefine::ActionDefine(const ActionDefine& other)
    : action_define_id(other.action_define_id),
      actions(other.actions) {}

// IDA_EA: 0x8899C82
// IDA_NAME: _ZN12advancealtar12ActionDefineD2Ev
// DEMANGLED: advancealtar::ActionDefine::~ActionDefine()
ActionDefine::~ActionDefine() {}

// IDA_EA: 0x8899C98
// IDA_NAME: _ZN12advancealtar12ActionDefine5resetEv
// DEMANGLED: advancealtar::ActionDefine::reset(void)
ActionDefine* ActionDefine::reset() {
    action_define_id = 0;
    actions.clear();
    return this;
}

// IDA_EA: 0x889B99C
// IDA_NAME: _ZN12advancealtar13_ActionAssignC2Ev
// DEMANGLED: advancealtar::_ActionAssign::_ActionAssign(void)
_ActionAssign::_ActionAssign() {
    memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x889F4B0
// IDA_NAME: _ZN12advancealtar13_ActionAssignD2Ev
// DEMANGLED: advancealtar::_ActionAssign::~_ActionAssign()
_ActionAssign::~_ActionAssign() {}

// IDA_EA: 0x8899CB8
// IDA_NAME: _ZN12advancealtar5PhaseC2Ev
// DEMANGLED: advancealtar::Phase::Phase(void)
Phase::Phase()
    : duration_ms(0),
      phase_type(0),
      action_assigns() {}

// IDA_EA: 0x83D3468
// IDA_NAME: _ZN12advancealtar5PhaseC2ERKS0_
// DEMANGLED: advancealtar::Phase::Phase(Phase const&)
Phase::Phase(const Phase& other)
    : duration_ms(other.duration_ms),
      phase_type(other.phase_type),
      action_assigns(other.action_assigns) {}

// IDA_EA: 0x8899CE2
// IDA_NAME: _ZN12advancealtar5PhaseD2Ev
// DEMANGLED: advancealtar::Phase::~Phase()
Phase::~Phase() {}

// IDA_EA: 0x83E0B3E
// IDA_NAME: _ZN12advancealtar5PhaseaSERKS0_
// DEMANGLED: advancealtar::Phase::operator=(Phase const&)
Phase& Phase::operator=(const Phase& other) {
    if (this != &other) {
        duration_ms = other.duration_ms;
        phase_type = other.phase_type;
        action_assigns = other.action_assigns;
    }
    return *this;
}

// IDA_EA: 0x8899CF8
// IDA_NAME: _ZN12advancealtar5Phase5resetEv
// DEMANGLED: advancealtar::Phase::reset(void)
Phase* Phase::reset() {
    duration_ms = 0;
    action_assigns.clear();
    return this;
}

// IDA_EA: 0x8899D18
// IDA_NAME: _ZN12advancealtar15TimeLineSummaryC2Ev
// DEMANGLED: advancealtar::TimeLineSummary::TimeLineSummary(void)
TimeLineSummary::TimeLineSummary()
    : total_duration_ms(0),
      phase_summary() {}

// IDA_EA: 0x837E618
// IDA_NAME: _ZN12advancealtar15TimeLineSummaryC2ERKS0_
// DEMANGLED: advancealtar::TimeLineSummary::TimeLineSummary(TimeLineSummary const&)
TimeLineSummary::TimeLineSummary(const TimeLineSummary& other)
    : total_duration_ms(other.total_duration_ms),
      phase_summary(other.phase_summary) {}

// IDA_EA: 0x8899D38
// IDA_NAME: _ZN12advancealtar15TimeLineSummaryD2Ev
// DEMANGLED: advancealtar::TimeLineSummary::~TimeLineSummary()
TimeLineSummary::~TimeLineSummary() {}

// IDA_EA: 0x8377912
// IDA_NAME: _ZN12advancealtar15TimeLineSummaryaSERKS0_
// DEMANGLED: advancealtar::TimeLineSummary::operator=(TimeLineSummary const&)
TimeLineSummary& TimeLineSummary::operator=(const TimeLineSummary& other) {
    if (this != &other) {
        total_duration_ms = other.total_duration_ms;
        phase_summary = other.phase_summary;
    }
    return *this;
}

// IDA_EA: 0x8899D4E
// IDA_NAME: _ZN12advancealtar15TimeLineSummary5resetEv
// DEMANGLED: advancealtar::TimeLineSummary::reset(void)
TimeLineSummary* TimeLineSummary::reset() {
    total_duration_ms = 0;
    phase_summary.clear();
    return this;
}

// IDA_EA: 0x8899D6E
// IDA_NAME: _ZN12advancealtar8TimeLineC2Ev
// DEMANGLED: advancealtar::TimeLine::TimeLine(void)
TimeLine::TimeLine()
    : phases(),
      summary() {}

// IDA_EA: 0x837E642
// IDA_NAME: _ZN12advancealtar8TimeLineC2ERKS0_
// DEMANGLED: advancealtar::TimeLine::TimeLine(TimeLine const&)
TimeLine::TimeLine(const TimeLine& other)
    : phases(other.phases),
      summary(other.summary) {}

// IDA_EA: 0x8899DB4
// IDA_NAME: _ZN12advancealtar8TimeLineD2Ev
// DEMANGLED: advancealtar::TimeLine::~TimeLine()
TimeLine::~TimeLine() {}

// IDA_EA: 0x8377940
// IDA_NAME: _ZN12advancealtar8TimeLineaSERKS0_
// DEMANGLED: advancealtar::TimeLine::operator=(TimeLine const&)
TimeLine& TimeLine::operator=(const TimeLine& other) {
    if (this != &other) {
        phases = other.phases;
        summary = other.summary;
    }
    return *this;
}

// IDA_EA: 0x8899DFA
// IDA_NAME: _ZN12advancealtar8TimeLine5resetEv
// DEMANGLED: advancealtar::TimeLine::reset(void)
TimeLine* TimeLine::reset() {
    phases.clear();
    summary.reset();
    return this;
}

// IDA_EA: 0x8899E1C
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameterC2Ev
// DEMANGLED: advancealtar::StageTimeLineParameter::StageTimeLineParameter(void)
StageTimeLineParameter::StageTimeLineParameter()
    : summon_objects(),
      action_defines(),
      time_line() {}

// IDA_EA: 0x8899E88
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameterD2Ev
// DEMANGLED: advancealtar::StageTimeLineParameter::~StageTimeLineParameter()
StageTimeLineParameter::~StageTimeLineParameter() {}

// IDA_EA: 0x8899EF6
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter5resetEv
// DEMANGLED: advancealtar::StageTimeLineParameter::reset(void)
StageTimeLineParameter* StageTimeLineParameter::reset() {
    summon_objects.clear();
    action_defines.clear();
    time_line.reset();
    return this;
}

// IDA_EA: 0x889A02C
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter8debugLogEv
// DEMANGLED: advancealtar::StageTimeLineParameter::debugLog(void)
void StageTimeLineParameter::debugLog() {}

// IDA_EA: 0x889A032
// IDA_NAME: _ZN12advancealtar22StageTimeLineParameter23checkAndSetActionDefineERNS_8TimeLineERKSt3mapIiNS_12ActionDefineESt4lessIiESaISt4pairIKiS4_EEE
// DEMANGLED: advancealtar::StageTimeLineParameter::checkAndSetActionDefine(TimeLine&, std::map<int,ActionDefine> const&)
int StageTimeLineParameter::checkAndSetActionDefine(
    TimeLine& target_time_line,
    const std::map<int, ActionDefine>& source_action_defines) {
    int begin_time_ms = 0;
    int phase_index = 0;
    target_time_line.summary.phase_summary.clear();

    for (std::vector<Phase>::iterator phase_it =
             target_time_line.phases.begin();
         phase_it != target_time_line.phases.end();
         ++phase_it, ++phase_index) {
        for (std::vector<_ActionAssign>::iterator assign_it =
                 phase_it->action_assigns.begin();
             assign_it != phase_it->action_assigns.end();
             ++assign_it) {
            std::map<int, ActionDefine>::const_iterator define_it =
                source_action_defines.find(assign_it->action_define_id);
            if (define_it == source_action_defines.end()) {
                printf("advancealtar : not found ActionDefine Id = %d\n",
                       assign_it->action_define_id);
                return 0;
            }
            // IDA 0x889A032 stores the address of the map value itself at
            // _ActionAssign+0x04.  ProcStage later reads the define id at
            // +0x00 and the action vector at +0x04 from that same object.
            assign_it->action_define = &define_it->second;
        }

        _PhaseSummary phase_summary;
        phase_summary.phase_index = phase_index;
        phase_summary.begin_time_ms = begin_time_ms;
        phase_summary.phase_type = phase_it->phase_type;
        target_time_line.summary.phase_summary.push_back(phase_summary);
        begin_time_ms += phase_it->duration_ms;
    }

    target_time_line.summary.total_duration_ms = begin_time_ms;
    return 1;
}

}  // namespace advancealtar
