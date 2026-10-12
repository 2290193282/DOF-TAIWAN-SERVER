#include "infra/concurrent/timer/daily_schedule_manager.h"

#include <cstring>
#include <time.h>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"


namespace {

const ENUM_NOTIPACKET kDailyScheduleStatePacket =
    ENUM_NOTIPACKET_DAILYSCHEDULE_CONTENTS_STATE;

struct DailyScheduleStatePacket {
    int32_t schedule_type;
    int32_t state;
    uint32_t schedule_id;
    uint32_t remaining_seconds;
};

static_assert(sizeof(DailyScheduleStatePacket) == 0x10u,
              "daily-schedule notification must remain 16 bytes");

const uint8_t kGmOpenStartTime[3] = {0, 0, 0};
const uint8_t kGmOpenEndTime[3] = {0x17u, '2', '2'};

uint8_t LowByteOfPointer(const void* pointer) {
    return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(pointer));
}

void SetGmScheduleWindow(_DailySchedule& schedule,
                         const uint8_t start_time[3],
                         const uint8_t end_time[3]) {
    schedule.state = 7;
    std::memcpy(&schedule.timing.start_hour, start_time, 3);
    std::memcpy(&schedule.timing.end_hour, end_time, 3);
}

}  // namespace


// IDA_EA: 0x8127B12
// IDA_NAME: _ZN14_DailyScheduleD2Ev
// DEMANGLED: _DailySchedule::~_DailySchedule()
_DailySchedule::~_DailySchedule() {}

// IDA_EA: 0x8125C0C
// IDA_NAME: _ZN21CDailyScheduleManagerC2Ev
// DEMANGLED: CDailyScheduleManager::CDailyScheduleManager(void)
CDailyScheduleManager::CDailyScheduleManager()
    : schedules_() {}

// IDA_EA: 0x8125C24
// IDA_NAME: _ZN21CDailyScheduleManagerD2Ev
// DEMANGLED: CDailyScheduleManager::~CDailyScheduleManager()
CDailyScheduleManager::~CDailyScheduleManager() {}

// IDA_EA: 0x8125C3C
// IDA_NAME: _ZN21CDailyScheduleManager15AddDailScheduleERKSt6vectorI14_DailyScheduleSaIS1_EE
// DEMANGLED: CDailyScheduleManager::AddDailSchedule(std::vector<_DailySchedule> const&)
bool CDailyScheduleManager::AddDailSchedule(
    const std::vector<_DailySchedule>& schedules) {
    const unsigned int count = schedules.size();
    for (unsigned int index = 0; index < count; ++index) {
        schedules_.push_back(schedules[index]);
    }
    return false;
}

// IDA_EA: 0x8125C94
// IDA_NAME: _ZN21CDailyScheduleManager29CheckAndSendToAllUserScheduleEv
// DEMANGLED: CDailyScheduleManager::CheckAndSendToAllUserSchedule(void)
int CDailyScheduleManager::CheckAndSendToAllUserSchedule() {
    return CheckAndSendToUserSchedule(0, false);
}

// IDA_EA: 0x8125CB8
// IDA_NAME: _ZN21CDailyScheduleManager29CheckAndSendToOneUserScheduleER5CUser
// DEMANGLED: CDailyScheduleManager::CheckAndSendToOneUserSchedule(CUser &)
int CDailyScheduleManager::CheckAndSendToOneUserSchedule(CUser& user) {
    return CheckAndSendToUserSchedule(&user, false);
}

// IDA_EA: 0x8125CDA
// IDA_NAME: _ZN21CDailyScheduleManager26CheckAndSendToUserScheduleEP5CUserb
// DEMANGLED: CDailyScheduleManager::CheckAndSendToUserSchedule(CUser *,bool)
bool CDailyScheduleManager::CheckAndSendToUserSchedule(CUser* user,
                                                        bool check_now) {
    const unsigned int count = schedules_.size();
    const time_t now = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    struct tm local_time;
    localtime_r(&now, &local_time);

    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        unsigned int remaining_seconds = 0;
        bool state_changed = false;

        if (!user) {
            const int schedule_state =
                CheckScheduleState(schedule, remaining_seconds, check_now);
            if (schedule_state == 1) {
                if (schedule.state == 0) {
                    schedule.state = 1;
                    state_changed = true;
                }
            } else if (schedule_state == 0 && schedule.state == 1) {
                schedule.state = 0;
                state_changed = true;
            }
        }

        if (state_changed) {
            _DailySchedule* previous_schedule = 0;
            _DailySchedule* next_schedule = 0;
            if (CheckContinueSchedule(schedule,
                                      &previous_schedule,
                                      &next_schedule)) {
                if (schedule.state != 0) {
                    if (schedule.state == 1) {
                        if (previous_schedule) {
                            previous_schedule->state = 0;
                        }
                        state_changed = false;
                    }
                } else {
                    if (next_schedule) {
                        next_schedule->state = 1;
                    }
                    state_changed = false;
                }
            }
        }

        if (state_changed) {
            PacketGuard packet;
            packet.put_header(0, kDailyScheduleStatePacket);
            DailyScheduleStatePacket payload;
            payload.schedule_type = schedule.schedule_type;
            payload.state = schedule.state;
            payload.schedule_id = schedule.schedule_id;
            payload.remaining_seconds = remaining_seconds;
            packet.put_binary(reinterpret_cast<const char*>(&payload),
                              sizeof(payload));
            packet.finalize(true);
            G_GameWorld()->send_all(packet);
        } else if (schedule.state == 1 && user) {
            PacketGuard packet;
            packet.put_header(0, kDailyScheduleStatePacket);
            DailyScheduleStatePacket payload;
            payload.schedule_type = schedule.schedule_type;
            payload.state = 2;
            payload.schedule_id = schedule.schedule_id;
            payload.remaining_seconds = remaining_seconds;
            packet.put_binary(reinterpret_cast<const char*>(&payload),
                              sizeof(payload));
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return count != 0;
}

// IDA_EA: 0x8125FF4
// IDA_NAME: _ZNK21CDailyScheduleManager18CheckScheduleStateERK14_DailyScheduleRjb
// DEMANGLED: CDailyScheduleManager::CheckScheduleState(_DailySchedule const&,unsigned int &,bool)const
int CDailyScheduleManager::CheckScheduleState(
    const _DailySchedule& schedule,
    unsigned int& remaining_seconds,
    bool check_now) const {
    (void)check_now;

    const time_t now = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    struct tm local_time;
    localtime_r(&now, &local_time);

    if (schedule.repeat_event_id() != 0) {
        if (!GlobalData::s_event_manager ||
            !GlobalData::s_event_manager->GetRepeatEvent(
                schedule.repeat_event_id())) {
            remaining_seconds = 0;
            return 0;
        }
        CEventBase* const repeat_event =
            GlobalData::s_event_manager->GetRepeatEvent(
                schedule.repeat_event_id());
        if (repeat_event->IsEventing(0) != 1) {
            remaining_seconds = 0;
            return 0;
        }
    }

    struct tm start_time = local_time;
    start_time.tm_hour = schedule.timing.start_hour;
    start_time.tm_min = schedule.timing.start_minute;
    start_time.tm_sec = schedule.timing.start_second;
    const time_t schedule_start_time = mktime(&start_time);

    struct tm end_time = local_time;
    end_time.tm_hour = schedule.timing.end_hour;
    end_time.tm_min = schedule.timing.end_minute;
    end_time.tm_sec = schedule.timing.end_second;
    const time_t schedule_end_time = mktime(&end_time);

    const bool is_active =
        schedule.timing.start_date_sec <= now &&
        schedule.timing.end_date_sec >= now &&
        (schedule.timing.weekday ==
             static_cast<uint32_t>(local_time.tm_wday) ||
         schedule.timing.weekday == 7) &&
        now >= schedule_start_time && now <= schedule_end_time;
    remaining_seconds = is_active
        ? static_cast<unsigned int>(schedule_end_time - now)
        : 0;
    return is_active ? 1 : 0;
}

// IDA_EA: 0x81261A8
// IDA_NAME: _ZN21CDailyScheduleManager21CheckContinueScheduleERK14_DailySchedulePPS0_S4_
// DEMANGLED: CDailyScheduleManager::CheckContinueSchedule(_DailySchedule const&,_DailySchedule**,_DailySchedule**)
bool CDailyScheduleManager::CheckContinueSchedule(
    const _DailySchedule& schedule,
    _DailySchedule** previous_schedule,
    _DailySchedule** next_schedule) {
    const time_t now = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    struct tm local_time;
    localtime_r(&now, &local_time);

    struct tm prior_boundary = local_time;
    prior_boundary.tm_hour = schedule.timing.start_hour;
    prior_boundary.tm_min = schedule.timing.start_minute;
    prior_boundary.tm_sec = schedule.timing.start_second;
    const time_t previous_second = mktime(&prior_boundary) - 1;
    localtime_r(&previous_second, &prior_boundary);

    struct tm following_boundary = local_time;
    following_boundary.tm_hour = schedule.timing.end_hour;
    following_boundary.tm_min = schedule.timing.end_minute;
    following_boundary.tm_sec = schedule.timing.end_second;
    const time_t next_second = mktime(&following_boundary) + 1;
    localtime_r(&next_second, &following_boundary);

    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& candidate = schedules_[index];
        if (schedule.schedule_id != candidate.schedule_id ||
            schedule.schedule_type != candidate.schedule_type ||
            &candidate == &schedule) {
            continue;
        }

        bool matches_previous = false;
        bool matches_next = false;
        if (schedule.state != 0) {
            if (schedule.state == 1 &&
                prior_boundary.tm_hour == candidate.timing.end_hour &&
                prior_boundary.tm_min == candidate.timing.end_minute &&
                prior_boundary.tm_sec == candidate.timing.end_second) {
                matches_previous = true;
            }
        } else if (following_boundary.tm_hour ==
                       candidate.timing.start_hour &&
                   following_boundary.tm_min ==
                       candidate.timing.start_minute &&
                   following_boundary.tm_sec ==
                       candidate.timing.start_second) {
            matches_next = true;
        }

        if (matches_next) {
            struct tm candidate_time;
            localtime_r(&now, &candidate_time);
            const int current_weekday = candidate_time.tm_wday;
            if (candidate.timing.weekday ==
                    static_cast<uint32_t>(current_weekday) ||
                candidate.timing.weekday == 7) {
                candidate_time.tm_hour = candidate.timing.start_hour;
                candidate_time.tm_min = candidate.timing.start_minute;
                candidate_time.tm_sec = candidate.timing.start_second;
                const time_t candidate_start_time = mktime(&candidate_time);
                candidate_time.tm_hour = candidate.timing.end_hour;
                candidate_time.tm_min = candidate.timing.end_minute;
                candidate_time.tm_sec = candidate.timing.end_second;
                const time_t candidate_end_time = mktime(&candidate_time);
                if (now >= candidate_start_time && now <= candidate_end_time) {
                    *next_schedule = &candidate;
                }
            }
        } else if (matches_previous) {
            *previous_schedule = &candidate;
        }
    }
    return *next_schedule || *previous_schedule;
}

// IDA_EA: 0x81266E2
// IDA_NAME: _ZNK21CDailyScheduleManager16GetScheduleStateE18_DailyScheduleTypej
// DEMANGLED: CDailyScheduleManager::GetScheduleState(_DailyScheduleType,unsigned int) const
int CDailyScheduleManager::GetScheduleState(
    _DailyScheduleType type,
    unsigned int schedule_id) const {
    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        const _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id == schedule_id &&
            schedule.schedule_type == type &&
            schedule.state == 1) {
            return schedule.state;
        }
    }
    return 0;
}

// IDA_EA: 0x81267BC
// IDA_NAME: _ZN21CDailyScheduleManager19GM_OpenSouthernDaleEv
// DEMANGLED: CDailyScheduleManager::GM_OpenSouthernDale(void)
uint8_t CDailyScheduleManager::GM_OpenSouthernDale() {
    const time_t now = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    struct tm start_tm;
    struct tm end_tm;
    localtime_r(&now, &start_tm);
    const time_t end_time = now + 600;
    localtime_r(&end_time, &end_tm);

    const uint8_t start_time[3] = {
        static_cast<uint8_t>(start_tm.tm_hour),
        static_cast<uint8_t>(start_tm.tm_min),
        static_cast<uint8_t>(start_tm.tm_sec),
    };
    const uint8_t finish_time[3] = {
        static_cast<uint8_t>(end_tm.tm_hour),
        static_cast<uint8_t>(end_tm.tm_min),
        static_cast<uint8_t>(end_tm.tm_sec),
    };

    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id != 100u ||
            schedule.schedule_type != DAILY_SCHEDULE_DUNGEON) {
            continue;
        }
        if (southern_dale_dungeon_snapshot_.schedule.schedule_id != 100u) {
            std::memcpy(&southern_dale_dungeon_snapshot_.schedule,
                        &schedule,
                        sizeof(schedule));
            southern_dale_dungeon_snapshot_.schedule.state = 0;
        }
        schedule.timing.weekday = 7;
        std::memcpy(&schedule.timing.start_hour, start_time, 3);
        std::memcpy(&schedule.timing.end_hour, finish_time, 3);
        break;
    }

    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id != 20549u ||
            schedule.schedule_type != DAILY_SCHEDULE_QUEST) {
            continue;
        }
        if (southern_dale_quest_snapshot_.schedule.schedule_id != 20549u) {
            std::memcpy(&southern_dale_quest_snapshot_.schedule,
                        &schedule,
                        sizeof(schedule));
            southern_dale_quest_snapshot_.schedule.state = 0;
        }
        schedule.timing.weekday = 7;
        std::memcpy(&schedule.timing.start_hour, start_time, 3);
        return LowByteOfPointer(
            std::memcpy(&schedule.timing.end_hour, finish_time, 3));
    }
    return false;
}

// IDA_EA: 0x8126AD2
// IDA_NAME: _ZN21CDailyScheduleManager20GM_CloseSouthernDaleEv
// DEMANGLED: CDailyScheduleManager::GM_CloseSouthernDale(void)
bool CDailyScheduleManager::GM_CloseSouthernDale() {
    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id == 100u &&
            schedule.schedule_type == DAILY_SCHEDULE_DUNGEON &&
            southern_dale_dungeon_snapshot_.schedule.schedule_id == 100u) {
            std::memcpy(&schedule,
                        &southern_dale_dungeon_snapshot_.schedule,
                        sizeof(schedule));
        }
    }
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id == 20549u &&
            schedule.schedule_type == DAILY_SCHEDULE_QUEST &&
            southern_dale_quest_snapshot_.schedule.schedule_id == 20549u) {
            std::memcpy(&schedule,
                        &southern_dale_quest_snapshot_.schedule,
                        sizeof(schedule));
        }
    }
    return false;
}

// IDA_EA: 0x8126C5C
// IDA_NAME: _ZN21CDailyScheduleManager15GM_OpenKingHornEv
// DEMANGLED: CDailyScheduleManager::GM_OpenKingHorn(void)
int CDailyScheduleManager::GM_OpenKingHorn() {
    return GM_OpenSchedule(1,
                           DAILY_SCHEDULE_EVENT,
                           king_horn_snapshot_.schedule);
}

// IDA_EA: 0x8126C8A
// IDA_NAME: _ZN21CDailyScheduleManager16GM_CloseKingHornEv
// DEMANGLED: CDailyScheduleManager::GM_CloseKingHorn(void)
int CDailyScheduleManager::GM_CloseKingHorn() {
    return GM_CloseSchedule(1,
                            DAILY_SCHEDULE_EVENT,
                            king_horn_snapshot_.schedule);
}

// IDA_EA: 0x8126CB8
// IDA_NAME: _ZN21CDailyScheduleManager16GM_OpenBossTowerEv
// DEMANGLED: CDailyScheduleManager::GM_OpenBossTower(void)
int CDailyScheduleManager::GM_OpenBossTower() {
    GM_OpenSchedule(3505,
                    DAILY_SCHEDULE_DUNGEON,
                    boss_tower_dungeon_snapshot_.schedule);
    return GM_OpenSchedule(20570,
                           DAILY_SCHEDULE_QUEST,
                           boss_tower_quest_snapshot_.schedule);
}

// IDA_EA: 0x8126D0C
// IDA_NAME: _ZN21CDailyScheduleManager17GM_CloseBossTowerEv
// DEMANGLED: CDailyScheduleManager::GM_CloseBossTower(void)
int CDailyScheduleManager::GM_CloseBossTower() {
    GM_CloseSchedule(3505,
                     DAILY_SCHEDULE_DUNGEON,
                     boss_tower_dungeon_snapshot_.schedule);
    return GM_CloseSchedule(20570,
                            DAILY_SCHEDULE_QUEST,
                            boss_tower_quest_snapshot_.schedule);
}

// IDA_EA: 0x8126D60
// IDA_NAME: _ZN21CDailyScheduleManager19GM_OpenAdvanceAltarEv
// DEMANGLED: CDailyScheduleManager::GM_OpenAdvanceAltar(void)
int CDailyScheduleManager::GM_OpenAdvanceAltar() {
    return GM_OpenSchedule(10,
                           DAILY_SCHEDULE_ADVANCE_ALTAR,
                           advance_altar_snapshot_.schedule);
}

// IDA_EA: 0x8126D90
// IDA_NAME: _ZN21CDailyScheduleManager20GM_CloseAdvanceAltarEv
// DEMANGLED: CDailyScheduleManager::GM_CloseAdvanceAltar(void)
int CDailyScheduleManager::GM_CloseAdvanceAltar() {
    return GM_CloseSchedule(10,
                            DAILY_SCHEDULE_ADVANCE_ALTAR,
                            advance_altar_snapshot_.schedule);
}

// IDA_EA: 0x8126DC0
// IDA_NAME: _ZN21CDailyScheduleManager15GM_OpenScheduleEi18_DailyScheduleTypeR14_DailySchedule
// DEMANGLED: CDailyScheduleManager::GM_OpenSchedule(int,_DailyScheduleType,_DailySchedule &)
uint8_t CDailyScheduleManager::GM_OpenSchedule(
    int schedule_id,
    _DailyScheduleType schedule_type,
    _DailySchedule& snapshot) {
    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id != static_cast<unsigned int>(schedule_id) ||
            schedule.schedule_type != schedule_type) {
            continue;
        }
        if (snapshot.schedule_id != static_cast<unsigned int>(schedule_id)) {
            std::memcpy(&snapshot, &schedule, sizeof(schedule));
            snapshot.state = 0;
        }
        SetGmScheduleWindow(schedule, kGmOpenStartTime, kGmOpenEndTime);
        break;
    }

    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id != static_cast<unsigned int>(schedule_id) ||
            schedule.schedule_type != DAILY_SCHEDULE_QUEST) {
            continue;
        }
        if (snapshot.schedule_id != static_cast<unsigned int>(schedule_id)) {
            std::memcpy(&snapshot, &schedule, sizeof(schedule));
            snapshot.state = 0;
        }
        schedule.state = 7;
        std::memcpy(&schedule.timing.start_hour, kGmOpenStartTime, 3);
        return LowByteOfPointer(
            std::memcpy(&schedule.timing.end_hour, kGmOpenEndTime, 3));
    }
    return false;
}

// IDA_EA: 0x812707C
// IDA_NAME: _ZN21CDailyScheduleManager16GM_CloseScheduleEi18_DailyScheduleTypeR14_DailySchedule
// DEMANGLED: CDailyScheduleManager::GM_CloseSchedule(int,_DailyScheduleType,_DailySchedule &)
bool CDailyScheduleManager::GM_CloseSchedule(
    int schedule_id,
    _DailyScheduleType schedule_type,
    _DailySchedule& snapshot) {
    const unsigned int count = schedules_.size();
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id == static_cast<unsigned int>(schedule_id) &&
            schedule.schedule_type == schedule_type &&
            snapshot.schedule_id == static_cast<unsigned int>(schedule_id)) {
            snapshot.state = schedule.state == 1;
            std::memcpy(&schedule, &snapshot, sizeof(schedule));
            break;
        }
    }
    for (unsigned int index = 0; index < count; ++index) {
        _DailySchedule& schedule = schedules_[index];
        if (schedule.schedule_id == static_cast<unsigned int>(schedule_id) &&
            schedule.schedule_type == schedule_id &&
            snapshot.schedule_id == static_cast<unsigned int>(schedule_id)) {
            std::memcpy(&schedule, &snapshot, sizeof(schedule));
        }
    }
    return false;
}
