#include "old_game/economy/inventory/gen_unique_no.h"

#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"

namespace WongWork {

// IDA global: _ZN8WongWork12CGenUniqueNo12m_nTimeCountE @ 0x943DC8C.
uint16_t CGenUniqueNo::m_nTimeCount = 0;

// IDA_EA: 0x84B76AC
// IDA_NAME: _ZN8WongWork12CGenUniqueNo16_updateTimeCountEv
// DEMANGLED: WongWork::CGenUniqueNo::_updateTimeCount(void)
int CGenUniqueNo::_updateTimeCount() {
    const int result = static_cast<uint16_t>(++m_nTimeCount);
    if (m_nTimeCount > 0xC34Fu) {
        m_nTimeCount = 1;
    }
    return result;
}

// IDA_EA: 0x84B76D8
// IDA_NAME: _ZN8WongWork12CGenUniqueNo11GenUniqueNoEPc
// DEMANGLED: WongWork::CGenUniqueNo::GenUniqueNo(char *)
char* CGenUniqueNo::GenUniqueNo(char* destination) {
    _updateTimeCount();
    const int current_second =
        static_cast<int>(GlobalData::s_systemTime_.getCurSec());
    const int server_group = G_CEnvironment()->get_server_group();
    const int channel_no = G_CEnvironment()->get_channel_no();
    const int time_count = static_cast<uint16_t>(m_nTimeCount);
    OS_API::snprintf(destination,
                     0x14u,
                     "%02d%02d%10d%05d",
                     server_group,
                     channel_no,
                     current_second,
                     time_count);
    return destination;
}

// IDA_EA: 0x84B7750
// IDA_NAME: _ZN8WongWork12CGenUniqueNo8genIPGNoENS0_15ENUM_IPGNO_TYPEEjPc
// DEMANGLED: WongWork::CGenUniqueNo::genIPGNo(WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE,unsigned int,char *)
char* CGenUniqueNo::genIPGNo(ENUM_IPGNO_TYPE type,
                             unsigned int charac_no,
                             char* destination) {
    static const char type_const[] = "CLQBVFOE";
    time_t current = GlobalData::s_systemTime_.getCurSec();
    tm local_time;
    const tm* const parts = localtime_r(&current, &local_time);
    OS_API::snprintf(destination,
                     0x14u,
                     "%c%02d%02d%02d%02d%010d",
                     type_const[static_cast<int>(type)],
                     G_CEnvironment()->get_channel_no(),
                     (parts->tm_year + 1900) % 2000,
                     parts->tm_mon + 1,
                     parts->tm_mday,
                     charac_no);
    return destination;
}

}  // namespace WongWork
