#include "common/manager/system_time.h"

#include <ctime>
#include <cstdio>


// IDA_EA: 0x82A6936
// IDA_NAME: _ZN11CSystemTimeC2Ev
// DEMANGLED: CSystemTime::CSystemTime(void)
CSystemTime::CSystemTime()
    : cur_sec_(0),
      cur_tick_count_(0),
      cur_time_(),
      base_sec_(0) {
    gettimeofday(&cur_time_, 0);
    cur_sec_ = static_cast<int>(cur_time_.tv_sec);
    base_sec_ = cur_sec_;
    cur_tick_count_ = static_cast<int>(cur_time_.tv_usec / 1000);
}

// IDA_EA: 0x82A6986
// IDA_NAME: _ZN11CSystemTimeD2Ev
// DEMANGLED: CSystemTime::~CSystemTime()
CSystemTime::~CSystemTime() {}

// IDA_EA: 0x82A68C8
// IDA_NAME: _ZN11CSystemTime6updateEv
// DEMANGLED: CSystemTime::update(void)
CSystemTime* CSystemTime::update() {
    gettimeofday(&cur_time_, 0);
    cur_sec_ = static_cast<int>(cur_time_.tv_sec);
    cur_tick_count_ =
        1000 * (cur_sec_ - base_sec_) + static_cast<int>(cur_time_.tv_usec / 1000);
    return this;
}

// IDA_EA: 0x80CBC9E
// IDA_NAME: _ZN11CSystemTime9getCurSecEv
// DEMANGLED: CSystemTime::getCurSec(void)
int CSystemTime::getCurSec() const {
    return cur_sec_;
}

// IDA_EA: 0x81458AC
// IDA_NAME: _ZN11CSystemTime15getCurTickCountEv
// DEMANGLED: CSystemTime::getCurTickCount(void)
int CSystemTime::getCurTickCount() const {
    return cur_tick_count_;
}

// IDA_EA: 0x823445E
// IDA_NAME: _ZN11CSystemTime10getCurDateEv
// DEMANGLED: CSystemTime::getCurDate(void)
const char* CSystemTime::getCurDate() const {
    static __thread char date_text[16];
    time_t timer = static_cast<time_t>(cur_sec_);
    tm local_tm;
    localtime_r(&timer, &local_tm);
    std::snprintf(date_text, sizeof(date_text), "%04d-%02d-%02d",
                  local_tm.tm_year + 1900, local_tm.tm_mon + 1,
                  local_tm.tm_mday);
    return date_text;
}
