#include "common/manager/statistc_proxy_abi.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "common/manager/statistics_packet_abi.h"
#include "old_game/player/core/cuser.h"

namespace statistc_proxy {

// IDA_EA: 0x860FDD2
// IDA_NAME: _ZN14statistc_proxy3addEP5CUserPKcS3_jS3_z
// DEMANGLED: statistc_proxy::add(CUser *,char const*,char const*,unsigned int,char const*,...)
int add(CUser* user,
        const char* category,
        const char* statistic,
        unsigned int value,
        const char* format,
        ...) {
    char formatted_value[1024];
    std::memset(formatted_value, 0, sizeof(formatted_value));

    va_list arguments;
    va_start(arguments, format);
    std::vsprintf(formatted_value, format, arguments);
    va_end(arguments);

    if (std::strlen(category) <= 0x20u && std::strlen(statistic) <= 0x20u &&
        std::strlen(formatted_value) <= 0x38u) {
        StatisticsPacket packet;
        std::strcpy(packet.category_, category);
        std::strcpy(packet.statistic_, statistic);
        std::strcpy(packet.formatted_value_, formatted_value);
        packet.value_ = value;
        return GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }

    return static_cast<int>(std::strlen(formatted_value));
}

}  // namespace statistc_proxy
