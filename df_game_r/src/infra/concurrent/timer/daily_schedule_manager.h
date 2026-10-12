#ifndef OLDSRV_GLOBAL_DAILY_SCHEDULE_MANAGER_H_
#define OLDSRV_GLOBAL_DAILY_SCHEDULE_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>


class CUser;


// IDA_EA: CDailyScheduleManager ctor/dtor @0x8125C0C/0x8125C24,
// AddDailSchedule @0x8125C3C, GetScheduleState @0x81266E2.
// IDA unit: 99_misc_global/DailySchedule.
enum _DailyScheduleType {
    DAILY_SCHEDULE_DUNGEON = 0,
    DAILY_SCHEDULE_QUEST = 1,
    DAILY_SCHEDULE_UNKNOWN_2 = 2,
    DAILY_SCHEDULE_EVENT = 3,
    DAILY_SCHEDULE_ADVANCE_ALTAR = 4,
};

// IDA unit: 99_misc_global/DailySchedule.
struct _DailyScheduleTiming {
    int32_t start_date_sec;    // +0x0c
    int32_t end_date_sec;      // +0x10
    uint32_t weekday;          // +0x14; 0..6 or 7 for every day
    uint8_t start_hour;        // +0x18
    uint8_t start_minute;      // +0x19
    uint8_t start_second;      // +0x1a
    uint8_t end_hour;          // +0x1b
    uint8_t end_minute;        // +0x1c
    uint8_t end_second;        // +0x1d
    uint8_t reserved_1e[2];    // +0x1e
};

static_assert(sizeof(_DailyScheduleTiming) == 0x14u,
              "_DailySchedule timing region must stay 0x14 bytes");

struct _DailySchedule {
    ~_DailySchedule();

    int32_t dword_00;          // +0x00; repeat-event ID, zero for no event
    int32_t schedule_type;     // +0x04
    uint32_t schedule_id;      // +0x08
    union {
        _DailyScheduleTiming timing; // +0x0c, named IDA timing view
        uint8_t reserved_0c[0x14];   // +0x0c, importer compatibility view
    };
    int32_t state;             // +0x20

    int32_t& repeat_event_id() { return dword_00; }
    int32_t repeat_event_id() const { return dword_00; }
};

static_assert(sizeof(_DailySchedule) == 0x24u,
              "_DailySchedule IDA ABI size must stay 0x24");
static_assert(offsetof(_DailySchedule, schedule_type) == 0x04u,
              "_DailySchedule schedule_type offset must stay 0x04");
static_assert(offsetof(_DailySchedule, schedule_id) == 0x08u,
              "_DailySchedule schedule_id offset must stay 0x08");
static_assert(offsetof(_DailySchedule, timing) == 0x0Cu,
              "_DailySchedule start-date offset must stay 0x0C");
static_assert(offsetof(_DailySchedule, state) == 0x20u,
              "_DailySchedule state offset must stay 0x20");

// The manager keeps six pre-GM `_DailySchedule` copies in its first 0xd8
// bytes.  The original manager destructor destroys only `schedules_`, so the
// storage deliberately owns an inactive no-op union destructor while exposing
// the original `_DailySchedule&` ABI to GM_Open/GM_CloseSchedule.
union _DailyScheduleGmSnapshot {
    _DailySchedule schedule;

    _DailyScheduleGmSnapshot() : schedule() {}
    ~_DailyScheduleGmSnapshot() {}
};

static_assert(sizeof(_DailyScheduleGmSnapshot) == 0x24u,
              "GM schedule snapshot must retain the old 36-byte extent");

// IDA unit: 99_misc_global/CDailyScheduleManager.
class CDailyScheduleManager {
public:
    CDailyScheduleManager();
    ~CDailyScheduleManager();

    bool AddDailSchedule(const std::vector<_DailySchedule>& schedules);
    int CheckAndSendToAllUserSchedule();
    int CheckAndSendToOneUserSchedule(CUser& user);
    bool CheckAndSendToUserSchedule(CUser* user, bool check_now);
    int CheckScheduleState(const _DailySchedule& schedule,
                           unsigned int& remaining_seconds,
                           bool check_now) const;
    bool CheckContinueSchedule(const _DailySchedule& schedule,
                               _DailySchedule** previous_schedule,
                               _DailySchedule** next_schedule);
    int GetScheduleState(_DailyScheduleType type, unsigned int schedule_id)
        const;

    // IDA_EA: 0x81267BC / 0x8126AD2.
    uint8_t GM_OpenSouthernDale();
    bool GM_CloseSouthernDale();

    // IDA_EA: 0x8126C5C / 0x8126C8A.
    int GM_OpenKingHorn();
    int GM_CloseKingHorn();

    // IDA_EA: 0x8126CB8 / 0x8126D0C.
    int GM_OpenBossTower();
    int GM_CloseBossTower();

    // IDA_EA: 0x8126D60 / 0x8126D90.
    int GM_OpenAdvanceAltar();
    int GM_CloseAdvanceAltar();

    // IDA_EA: 0x8126DC0 / 0x812707C.
    uint8_t GM_OpenSchedule(int schedule_id,
                            _DailyScheduleType schedule_type,
                            _DailySchedule& snapshot);
    bool GM_CloseSchedule(int schedule_id,
                          _DailyScheduleType schedule_type,
                          _DailySchedule& snapshot);

    _DailyScheduleGmSnapshot southern_dale_dungeon_snapshot_; // +0x00
    _DailyScheduleGmSnapshot southern_dale_quest_snapshot_;   // +0x24
    _DailyScheduleGmSnapshot king_horn_snapshot_;             // +0x48
    _DailyScheduleGmSnapshot boss_tower_dungeon_snapshot_;    // +0x6c
    _DailyScheduleGmSnapshot boss_tower_quest_snapshot_;      // +0x90
    _DailyScheduleGmSnapshot advance_altar_snapshot_;         // +0xb4
    std::vector<_DailySchedule> schedules_; // +0xD8

private:
    CDailyScheduleManager(const CDailyScheduleManager&);
    CDailyScheduleManager& operator=(const CDailyScheduleManager&);
};

static_assert(sizeof(CDailyScheduleManager) == 0xE4u,
              "CDailyScheduleManager IDA ABI size must stay 0xE4");
static_assert(offsetof(CDailyScheduleManager, schedules_) == 0xD8u,
              "CDailyScheduleManager schedules offset must stay 0xD8");


#endif  // OLDSRV_GLOBAL_DAILY_SCHEDULE_MANAGER_H_
