#ifndef OLDSRV_RUNTIME_SYSTEM_TIME_H_
#define OLDSRV_RUNTIME_SYSTEM_TIME_H_

#include <stdint.h>
#include <sys/time.h>


// IDA: CSystemTime @0x82A6936. The old object stores current seconds at
// offset +0, current millisecond tick at +4, raw timeval at +8, and base second
// at +16.
class CSystemTime {
public:
    CSystemTime();
    ~CSystemTime();

    // IDA: CSystemTime::update @0x82A68C8.
    CSystemTime* update();

    // IDA: CSystemTime::getCurSec @0x80CBC9E.
    int getCurSec() const;

    // IDA: CSystemTime::getCurTickCount @0x81458AC.
    int getCurTickCount() const;

    // IDA: CSystemTime::getCurDate @0x823445E.
    const char* getCurDate() const;

private:
    int cur_sec_;
    int cur_tick_count_;
    timeval cur_time_;
    int base_sec_;
};


#endif  // OLDSRV_RUNTIME_SYSTEM_TIME_H_
