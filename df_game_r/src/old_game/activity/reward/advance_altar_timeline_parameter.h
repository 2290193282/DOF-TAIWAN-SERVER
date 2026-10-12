#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_TIMELINE_PARAMETER_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_TIMELINE_PARAMETER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

namespace advancealtar {

// IDA_EA: StageTimeLineParameter @0x8899E1C/0x8899E88; reset @0x8899EF6;
// importStageTimeLineParameter @0x8899F26.
// IDA unit: 99_misc_global/advancealtar/_PhaseSummary.
struct _PhaseSummary {
    _PhaseSummary();
    ~_PhaseSummary();

    int32_t phase_index;     // +0x00
    int32_t begin_time_ms;   // +0x04
    int32_t phase_type;      // +0x08
};

static_assert(sizeof(_PhaseSummary) == 0x0Cu,
              "advancealtar::_PhaseSummary IDA size must stay 0x0C");

// IDA unit: 99_misc_global/advancealtar/_SummonObject.
struct _SummonObject {
    _SummonObject();

    int32_t summon_object_id;  // +0x00
    uint8_t is_enemy;          // +0x04
    uint8_t reserved_05[3];    // +0x05
    int32_t summon_type;       // +0x08
    int32_t monster_id;        // +0x0c
    int16_t count;             // +0x10
    uint8_t reserved_12[2];    // +0x12
    int32_t hp_rate;           // +0x14
    int32_t first_x;           // +0x18
    int32_t first_y;           // +0x1c
    int32_t direction;         // +0x20
};

static_assert(sizeof(_SummonObject) == 0x24u,
              "advancealtar::_SummonObject IDA size must stay 0x24");
static_assert(offsetof(_SummonObject, summon_type) == 0x08u,
              "_SummonObject summon_type offset must stay 0x08");

// IDA unit: 99_misc_global/advancealtar/_SummonObjectMs.
struct _SummonObjectMs {
    _SummonObjectMs();

    // IDA 0x812D658 schedules this record at +0x00, assigns the spawned
    // monster id from +0x04 and copies the original _SummonObject payload
    // from +0x14 into CMD523.  CMD42 then gates a death on +0x10/+0x0c.
    int32_t scheduled_time_ms;       // +0x00
    uint16_t monster_unique_id;      // +0x04
    uint16_t reserved_06;            // +0x06
    int32_t action_define_id;        // +0x08
    uint8_t killed_;                 // +0x0c
    uint8_t reserved_0d[3];          // +0x0d
    int32_t spawned_;                // +0x10; set to one after CMD523
    _SummonObject summon_object;     // +0x14
};

static_assert(sizeof(_SummonObjectMs) == 0x38u,
              "advancealtar::_SummonObjectMs IDA size must stay 0x38");
static_assert(offsetof(_SummonObjectMs, killed_) == 0x0cu,
              "_SummonObjectMs killed offset must stay 0x0c");
static_assert(offsetof(_SummonObjectMs, action_define_id) == 0x08u,
              "_SummonObjectMs action-define offset must stay 0x08");
static_assert(offsetof(_SummonObjectMs, spawned_) == 0x10u,
              "_SummonObjectMs spawned offset must stay 0x10");
static_assert(offsetof(_SummonObjectMs, summon_object) == 0x14u,
              "_SummonObjectMs payload offset must stay 0x14");

// IDA unit: 99_misc_global/advancealtar/_Action.
struct _Action {
    _Action();
    ~_Action();

    int32_t summon_object_id;          // +0x00
    const _SummonObject* summon_data;  // +0x04
    int32_t first_delay_ms;            // +0x08
    int32_t next_delay_ms;             // +0x0c
    uint8_t has_repeat_count;          // +0x10
    uint8_t reserved_11;               // +0x11
    int16_t repeat_count;              // +0x12
};

static_assert(sizeof(_Action) == 0x14u,
              "advancealtar::_Action IDA size must stay 0x14");
static_assert(offsetof(_Action, summon_data) == 0x04u,
              "_Action summon_data offset must stay 0x04");

// IDA unit: 99_misc_global/advancealtar/ActionDefine.
struct ActionDefine {
    ActionDefine();
    ActionDefine(const ActionDefine& other);
    ~ActionDefine();

    ActionDefine* reset();

    int32_t action_define_id;       // +0x00
    std::vector<_Action> actions;   // +0x04
};

static_assert(sizeof(ActionDefine) == 0x10u,
              "advancealtar::ActionDefine IDA size must stay 0x10");
static_assert(offsetof(ActionDefine, actions) == 0x04u,
              "ActionDefine actions offset must stay 0x04");

// IDA unit: 99_misc_global/advancealtar/_ActionAssign.
struct _ActionAssign {
    _ActionAssign();
    ~_ActionAssign();

    int32_t action_define_id;           // +0x00
    const ActionDefine* action_define;  // +0x04
    int16_t first_frame;                // +0x08
    int16_t last_frame;                 // +0x0a
    int32_t reserved_0c;                // +0x0c
};

static_assert(sizeof(_ActionAssign) == 0x10u,
              "advancealtar::_ActionAssign IDA size must stay 0x10");
static_assert(offsetof(_ActionAssign, action_define) == 0x04u,
              "_ActionAssign action-define offset must stay 0x04");

// IDA unit: 99_misc_global/advancealtar/Phase.
struct Phase {
    Phase();
    Phase(const Phase& other);
    ~Phase();

    Phase& operator=(const Phase& other);
    Phase* reset();

    int32_t duration_ms;                         // +0x00
    int32_t phase_type;                          // +0x04
    std::vector<_ActionAssign> action_assigns;   // +0x08
};

static_assert(sizeof(Phase) == 0x14u,
              "advancealtar::Phase IDA size must stay 0x14");
static_assert(offsetof(Phase, action_assigns) == 0x08u,
              "Phase action_assigns offset must stay 0x08");

// IDA unit: 99_misc_global/advancealtar/TimeLineSummary.
struct TimeLineSummary {
    TimeLineSummary();
    TimeLineSummary(const TimeLineSummary& other);
    ~TimeLineSummary();

    TimeLineSummary& operator=(const TimeLineSummary& other);
    TimeLineSummary* reset();

    int32_t total_duration_ms;                 // +0x00
    std::vector<_PhaseSummary> phase_summary;  // +0x04
};

static_assert(sizeof(TimeLineSummary) == 0x10u,
              "advancealtar::TimeLineSummary IDA size must stay 0x10");
static_assert(offsetof(TimeLineSummary, phase_summary) == 0x04u,
              "TimeLineSummary phase_summary offset must stay 0x04");

// IDA unit: 99_misc_global/advancealtar/TimeLine.
struct TimeLine {
    TimeLine();
    TimeLine(const TimeLine& other);
    ~TimeLine();

    TimeLine& operator=(const TimeLine& other);
    TimeLine* reset();

    std::vector<Phase> phases;  // +0x00
    TimeLineSummary summary;    // +0x0c
};

static_assert(sizeof(TimeLine) == 0x1Cu,
              "advancealtar::TimeLine IDA size must stay 0x1C");
static_assert(offsetof(TimeLine, summary) == 0x0Cu,
              "TimeLine summary offset must stay 0x0C");

// IDA unit: 99_misc_global/advancealtar/StageTimeLineParameter.
class StageTimeLineParameter {
public:
    StageTimeLineParameter();
    ~StageTimeLineParameter();

    StageTimeLineParameter* reset();
    int importStageTimeLineParameter(const char* filename);
    void debugLog();
    static int checkAndSetActionDefine(
        TimeLine& target_time_line,
        const std::map<int, ActionDefine>& source_action_defines);
    int importSummonObject();
    int importActionDefine();
    int importPhase(TimeLine& target_time_line);

    std::map<int, _SummonObject> summon_objects;  // +0x00
    std::map<int, ActionDefine> action_defines;   // +0x18
    TimeLine time_line;                           // +0x30
};

static_assert(sizeof(StageTimeLineParameter) == 0x4Cu,
              "advancealtar::StageTimeLineParameter IDA size must stay 0x4C");
static_assert(offsetof(StageTimeLineParameter, action_defines) == 0x18u,
              "StageTimeLineParameter action_defines offset must stay 0x18");
static_assert(offsetof(StageTimeLineParameter, time_line) == 0x30u,
              "StageTimeLineParameter time_line offset must stay 0x30");

}  // namespace advancealtar

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_TIMELINE_PARAMETER_H_
