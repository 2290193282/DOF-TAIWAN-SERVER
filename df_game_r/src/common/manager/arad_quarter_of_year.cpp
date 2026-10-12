#include "common/manager/arad_quarter_of_year.h"

#include <cstdio>
#include <ctime>


namespace ARAD {

// IDA_EA: 0x819E8D0
// IDA_NAME: _ZN4ARAD16GetQuarterOfYearEPc
// DEMANGLED: ARAD::GetQuarterOfYear(char *)
int GetQuarterOfYear(char* out) {
    std::time_t timer;
    std::time(&timer);
    std::tm* const local_time = std::localtime(&timer);
    return std::sprintf(out,
                        "%02d%d",
                        (local_time->tm_year + 1900) % 2000,
                        local_time->tm_mon / 3 + 1);
}

}  // namespace ARAD
