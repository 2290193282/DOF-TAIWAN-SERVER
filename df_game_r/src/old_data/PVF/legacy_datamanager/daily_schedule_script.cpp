#include "old_data/PVF/data/datamanager/daily_schedule_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <string.h>


// IDA_EA: 0x891826C
// IDA_NAME: _Z28importDailyScheduleEtcScriptPKcRSt6vectorI14_DailyScheduleSaIS2_EE
// DEMANGLED: importDailyScheduleEtcScript(char const*,std::vector<_DailySchedule,std::allocator<_DailySchedule>> &)
int importDailyScheduleEtcScript(const char* filename,
                                 std::vector<_DailySchedule>& schedules) {
    if (loadRDARScriptFile("", filename) != 1) {
        return 14071;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag != "[schedule]") {
            continue;
        }

        std::string start_date;
        std::string end_date;
        std::string weekday_text;
        std::string start_time_text;
        std::string end_time_text;
        while (1) {
            _DailySchedule schedule;
            ::memset(&schedule, 0, sizeof(schedule));

            schedule.dword_00 = ScanInt(&success);
            if (!success) {
                break;
            }
            schedule.schedule_type = ScanInt(&success);
            if (!success) {
                break;
            }
            schedule.schedule_id =
                static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                break;
            }
            if (ScanStr(&start_date) != 1) {
                break;
            }
            if (ScanStr(&end_date) != 1) {
                return 14113;
            }
            if (ScanStr(&weekday_text) != 1) {
                return 14115;
            }
            if (ScanStr(&start_time_text) != 1) {
                return 14117;
            }
            if (ScanStr(&end_time_text) != 1) {
                return 14119;
            }

            long start_date_value = 0;
            if (ConvertStringToTime(start_date, start_date_value) != 1) {
                return 14122;
            }
            ::memcpy(&schedule.reserved_0c[0],
                     &start_date_value,
                     sizeof(int32_t));

            long end_date_value = 0;
            if (ConvertStringToTime(end_date, end_date_value) != 1) {
                return 14124;
            }
            ::memcpy(&schedule.reserved_0c[4],
                     &end_date_value,
                     sizeof(int32_t));

            unsigned int weekday = 0;
            if (ConvertWeekdayStringToInt(weekday_text, weekday) != 1) {
                return 14128;
            }
            ::memcpy(&schedule.reserved_0c[8],
                     &weekday,
                     sizeof(uint32_t));

            unsigned char start_hour = 0;
            unsigned char start_minute = 0;
            unsigned char start_second = 0;
            if (ConvertHHMMSSStringToInt(start_time_text,
                                         start_hour,
                                         start_minute,
                                         start_second) != 1) {
                return 14133;
            }
            schedule.reserved_0c[12] = start_hour;
            schedule.reserved_0c[13] = start_minute;
            schedule.reserved_0c[14] = start_second;

            unsigned char end_hour = 0;
            unsigned char end_minute = 0;
            unsigned char end_second = 0;
            if (ConvertHHMMSSStringToInt(end_time_text,
                                         end_hour,
                                         end_minute,
                                         end_second) != 1) {
                return 14135;
            }
            schedule.reserved_0c[15] = end_hour;
            schedule.reserved_0c[16] = end_minute;
            schedule.reserved_0c[17] = end_second;

            schedules.push_back(schedule);
        }
    }

    return 0;
}
