#include "common/manager/common_time_game_option_abi.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

// IDA_EA: 0x822D0E6
// IDA_NAME: _ZN10CommonTime13SetCreateTimeEl
// DEMANGLED: CommonTime::SetCreateTime(long)
CommonTime* CommonTime::SetCreateTime(long timer) {
    const time_t source = static_cast<time_t>(timer);
    tm local;
    tm* const value = localtime_r(&source, &local);
    year = static_cast<unsigned char>(value->tm_year - 100);
    month = static_cast<unsigned char>(value->tm_mon + 1);
    day = static_cast<unsigned char>(value->tm_mday);
    hour = static_cast<unsigned char>(value->tm_hour);
    minute = static_cast<unsigned char>(value->tm_min);
    week_day = static_cast<unsigned char>(value->tm_wday);
    create_time = static_cast<int32_t>(timer);
    current_time = static_cast<int32_t>(timer);
    return this;
}

// IDA_EA: 0x822D16E
// IDA_NAME: _ZNK10CommonTime8MakeTimeER2tm
// DEMANGLED: CommonTime::MakeTime(tm &)const
tm* CommonTime::MakeTime(tm& output) const {
    output.tm_sec = 0;
    output.tm_min = minute;
    output.tm_hour = hour;
    output.tm_mday = day;
    output.tm_mon = static_cast<int>(month) - 1;
    output.tm_year = static_cast<int>(year) + 100;
    output.tm_isdst = 0;
    return &output;
}

// IDA_EA: 0x822D1DC
// IDA_NAME: _ZNK10CommonTimecvlEv
// DEMANGLED: CommonTime::operator long(void)const
CommonTime::operator long() const {
    tm output;
    MakeTime(output);
    return static_cast<long>(mktime(&output));
}

// IDA_EA: 0x86947D8
// IDA_NAME: _ZN10CommonTime5clearEv
// DEMANGLED: CommonTime::clear(void)
CommonTime* CommonTime::clear() {
    // The old body clears only the six calendar bytes.  create_time and
    // current_time remain untouched until their dedicated setters run.
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    week_day = 0;
    return this;
}

// IDA_EA: 0x82A4224
// IDA_NAME: _ZN10CommonTime10SetCurTimeEv
// DEMANGLED: CommonTime::SetCurTime(void)
CommonTime* CommonTime::SetCurTime() {
    time_t timer = 0;
    tm tm_value;
    time(&timer);
    tm* current = localtime_r(&timer, &tm_value);
    year = static_cast<unsigned char>(current->tm_year - 100);
    month = static_cast<unsigned char>(current->tm_mon + 1);
    day = static_cast<unsigned char>(current->tm_mday);
    hour = static_cast<unsigned char>(current->tm_hour);
    minute = static_cast<unsigned char>(current->tm_min);
    week_day = static_cast<unsigned char>(current->tm_wday);
    create_time = static_cast<int32_t>(timer);
    current_time = static_cast<int32_t>(timer);
    return this;
}

// IDA_EA: 0x8694806
// IDA_NAME: _ZNK10CommonTime13GetCreateTimeEPc
// DEMANGLED: CommonTime::GetCreateTime(char *)const
int CommonTime::GetCreateTime(char* destination) const {
    return std::sprintf(destination,
                        "%4d-%2d-%2d %2d:%2d:%2d",
                        static_cast<int>(year) + 2000,
                        static_cast<int>(month),
                        static_cast<int>(day),
                        static_cast<int>(hour),
                        static_cast<int>(minute),
                        0);
}

// IDA_EA: 0x8694884
// IDA_NAME: _ZN10CommonTime13SetCreateTimeEPc
// DEMANGLED: CommonTime::SetCreateTime(char *)
CommonTime* CommonTime::SetCreateTime(char* source) {
    char dest[3];
    dest[0] = 0;
    dest[1] = 0;
    dest[2] = 0;

    source += 2;
    std::strncpy(dest, source, 2u);
    year = static_cast<unsigned char>(std::atoi(dest));
    source += 3;
    std::strncpy(dest, source, 2u);
    month = static_cast<unsigned char>(std::atoi(dest));
    source += 3;
    std::strncpy(dest, source, 2u);
    day = static_cast<unsigned char>(std::atoi(dest));
    source += 3;
    std::strncpy(dest, source, 2u);
    hour = static_cast<unsigned char>(std::atoi(dest));
    source += 3;
    std::strncpy(dest, source, 2u);
    minute = static_cast<unsigned char>(std::atoi(dest));
    return this;
}

// IDA_EA: 0x84B7336
// IDA_NAME: _ZN10CETCOption12setETCOptionE15ENUM_OPTION_ETCs
// DEMANGLED: CETCOption::setETCOption(ENUM_OPTION_ETC,short)
int CETCOption::setETCOption(int etc_option, short value) {
    if (static_cast<unsigned int>(etc_option) <
        oldsrv::config::kGameEtcOptionCount) {
        values_[etc_option] = value;
        return reinterpret_cast<int>(this);
    }
    return etc_option;
}

// IDA_EA: 0x84B7216
// IDA_NAME: _ZN11CGameOption14set_etc_optionE15ENUM_OPTION_ETCs
// DEMANGLED: CGameOption::set_etc_option(ENUM_OPTION_ETC,short)
int CGameOption::set_etc_option(int etc_option, short value) {
    return etc_option_.setETCOption(etc_option, value);
}

// IDA_EA: 0x84B7266
// IDA_NAME: _ZN11CGameOption17get_hotkey_optionEh
// DEMANGLED: CGameOption::get_hotkey_option(unsigned char)
char* CGameOption::get_hotkey_option(unsigned char key_type) {
    if (key_type > 1u) {
        return 0;
    }
    return hotkeys_[key_type].bytes_;
}

unsigned int CGameOption::get_hotkey_option_length(
    unsigned char key_type) const {
    if (key_type > 1u) {
        return 0;
    }
    return hotkeys_[key_type].getTextLength();
}

// IDA_EA: 0x84B72B0
// IDA_NAME: _ZN11CGameOption20get_load_hotkey_flagEh
// DEMANGLED: CGameOption::get_load_hotkey_flag(unsigned char)
int CGameOption::get_load_hotkey_flag(unsigned char key_type) {
    if (key_type <= 1u) {
        return hotkey_load_flags_[key_type];
    }
    return 0;
}

// IDA_EA: 0x86B1F5A
// IDA_NAME: _Z22getDailyInitializeTimeiii
// DEMANGLED: getDailyInitializeTime(int,int,int)
long getDailyInitializeTime(int tm_hour, int tm_min, int tm_sec) {
    time_t timer = 0;
    CSystemTime* system_time = GlobalData::system_time();
    if (system_time) {
        timer = static_cast<time_t>(
            system_time->getCurSec());
    } else {
        time(&timer);
    }

    tm tp;
    tm* current = localtime_r(&timer, &tp);
    current->tm_hour = tm_hour;
    current->tm_min = tm_min;
    current->tm_sec = tm_sec;
    time_t result = mktime(current);
    if (result > timer) {
        result -= 86400;
    }
    return static_cast<long>(result);
}

// IDA_EA: 0x86B1FC0
// IDA_NAME: _Z19__GetCurrentDNFDateR2tm
// DEMANGLED: __GetCurrentDNFDate(tm &)
tm* __GetCurrentDNFDate(tm& out_time) {
    time_t timer = 0;
    CSystemTime* system_time = GlobalData::system_time();
    if (system_time) {
        timer = static_cast<time_t>(
            system_time->getCurSec());
    } else {
        time(&timer);
    }

    const CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    localtime_r(&timer, &out_time);
    if (out_time.tm_hour < reset_hour) {
        timer -= 86400;
        return localtime_r(&timer, &out_time);
    }
    return &out_time;
}
