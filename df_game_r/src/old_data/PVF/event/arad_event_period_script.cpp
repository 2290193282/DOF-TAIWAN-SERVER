#include "old_data/PVF/common/arad_script_data.h"

#include "old_data/PVF/common/rdar_script.h"

#include <algorithm>
#include <ctime>
#include <string>

namespace ARAD {
namespace SCRIPT {

// IDA_EA: 0x88B3970
// IDA_NAME: _ZN4ARAD6SCRIPT8readTimeEPs
// DEMANGLED: ARAD::SCRIPT::readTime(short *)
bool readTime(int16_t* value) {
    bool success = false;
    value[0] = static_cast<int16_t>(ScanInt(&success));
    if (!success) {
        throw "start year";
    }
    value[1] = static_cast<int16_t>(ScanInt(&success));
    if (!success) {
        throw "start month";
    }
    value[2] = static_cast<int16_t>(ScanInt(&success));
    if (!success) {
        throw "start day";
    }
    value[3] = static_cast<int16_t>(ScanInt(&success));
    if (!success) {
        throw "start time";
    }
    return true;
}

// IDA_EA: 0x88B3B4B
// IDA_NAME: _ZN4ARAD6SCRIPT10uint32TimeEPs
// DEMANGLED: ARAD::SCRIPT::uint32Time(short *)
time_t uint32Time(int16_t* value) {
    time_t timer = time(0);
    struct tm* tp = localtime(&timer);
    tp->tm_year = value[0] - 1900;
    tp->tm_mon = value[1] - 1;
    tp->tm_mday = value[2];
    tp->tm_hour = value[3];
    tp->tm_min = 0;
    tp->tm_sec = 0;
    return mktime(tp);
}

// IDA_EA: 0x88B3BD8
// IDA_NAME: _ZN4ARAD6SCRIPT21importAradEventScriptERSt6vectorINS0_16SEventPeriodDataESaIS2_EERS1_INS0_17SRewardPeriodDataESaIS6_EE
// DEMANGLED: ARAD::SCRIPT::importAradEventScript(std::vector<ARAD::SCRIPT::SEventPeriodData,std::allocator<ARAD::SCRIPT::SEventPeriodData>> &,std::vector<ARAD::SCRIPT::SRewardPeriodData,std::allocator<ARAD::SCRIPT::SRewardPeriodData>> &)
int importAradEventScript(std::vector<SEventPeriodData>& event_periods,
                          std::vector<SRewardPeriodData>& reward_periods) {
    event_periods.clear();
    reward_periods.clear();
    if (!loadRDARScriptFile("", "Etc/AradEventPeroid.etc")) {
        return 0;
    }

    std::string type;
    bool success = false;
    while (ScanType(type, true) == 1) {
        if (type == "[event period]") {
            while (true) {
                const int event_code = ScanInt(&success);
                if (!success) {
                    break;
                }
                int16_t start_time[4] = {0, 0, 0, 0};
                int16_t end_time[4] = {0, 0, 0, 0};
                if (!readTime(start_time)) {
                    throw "start time";
                }
                if (!readTime(end_time)) {
                    throw "end time";
                }
                const SEventPeriodData value(event_code,
                                             static_cast<unsigned int>(uint32Time(start_time)),
                                             static_cast<unsigned int>(uint32Time(end_time)));
                if (std::find(event_periods.begin(), event_periods.end(), value) !=
                    event_periods.end()) {
                    throw "already exist";
                }
                event_periods.push_back(value);
            }
        } else if (type == "[reward period]") {
            while (true) {
                const int event_code = ScanInt(&success);
                if (!success) {
                    break;
                }
                int16_t start_time[4] = {0, 0, 0, 0};
                int16_t end_time[4] = {0, 0, 0, 0};
                if (!readTime(start_time)) {
                    throw "start time";
                }
                if (!readTime(end_time)) {
                    throw "end time";
                }
                const unsigned long item_index = static_cast<unsigned long>(
                    ScanInt(&success));
                if (!success) {
                    throw "item index";
                }
                const int16_t item_count = static_cast<int16_t>(ScanInt(&success));
                if (!success) {
                    throw "item count";
                }
                const SRewardPeriodData value(
                    event_code,
                    item_index,
                    item_count,
                    static_cast<unsigned int>(uint32Time(start_time)),
                    static_cast<unsigned int>(uint32Time(end_time)));
                if (std::find(reward_periods.begin(), reward_periods.end(), value) !=
                    reward_periods.end()) {
                    throw "already exist";
                }
                reward_periods.push_back(value);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x88B4368
// IDA_NAME: _ZN4ARAD6SCRIPT16SEventPeriodDataC2Eijj
// DEMANGLED: ARAD::SCRIPT::SEventPeriodData::SEventPeriodData(int,unsigned int,unsigned int)
SEventPeriodData::SEventPeriodData(int event_code_value,
                                   unsigned int start_time_value,
                                   unsigned int end_time_value)
    : event_code(event_code_value),
      start_time(start_time_value),
      end_time(end_time_value) {}

// IDA_EA: 0x88B4388
// IDA_NAME: _ZN4ARAD6SCRIPT16SEventPeriodDataeqERKS1_
// DEMANGLED: ARAD::SCRIPT::SEventPeriodData::operator==(ARAD::SCRIPT::SEventPeriodData const&)
bool SEventPeriodData::operator==(const SEventPeriodData& other) {
    return event_code == other.event_code;
}

// IDA_EA: 0x88B439C
// IDA_NAME: _ZN4ARAD6SCRIPT17SRewardPeriodDataC2Eimsjj
// DEMANGLED: ARAD::SCRIPT::SRewardPeriodData::SRewardPeriodData(int,unsigned long,short,unsigned int,unsigned int)
SRewardPeriodData::SRewardPeriodData(int event_code_value,
                                     unsigned long item_index_value,
                                     int16_t item_count_value,
                                     unsigned int start_time_value,
                                     unsigned int end_time_value)
    : event_code(event_code_value),
      item_index(item_index_value),
      item_count(item_count_value),
      start_time(start_time_value),
      end_time(end_time_value) {}

// IDA_EA: 0x88B43DA
// IDA_NAME: _ZN4ARAD6SCRIPT17SRewardPeriodDataeqERKS1_
// DEMANGLED: ARAD::SCRIPT::SRewardPeriodData::operator==(ARAD::SCRIPT::SRewardPeriodData const&)
bool SRewardPeriodData::operator==(const SRewardPeriodData& other) {
    return event_code == other.event_code;
}

// IDA_EA: 0x88B558A
// IDA_NAME: _ZN4ARAD6SCRIPT16SEventPeriodDataD2Ev
// DEMANGLED: ARAD::SCRIPT::SEventPeriodData::~SEventPeriodData()
SEventPeriodData::~SEventPeriodData() {}

// IDA_EA: 0x88B5764
// IDA_NAME: _ZN4ARAD6SCRIPT17SRewardPeriodDataD2Ev
// DEMANGLED: ARAD::SCRIPT::SRewardPeriodData::~SRewardPeriodData()
SRewardPeriodData::~SRewardPeriodData() {}

}  // namespace SCRIPT
}  // namespace ARAD
