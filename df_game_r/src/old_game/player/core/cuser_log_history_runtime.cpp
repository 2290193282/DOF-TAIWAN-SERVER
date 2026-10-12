#include "old_game/player/core/cuser.h"

#include "core/config/env/environment.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/clog.h"
#include "infra/log/format/history_trace.h"
#include "infra/network/transport/network_endpoint.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>


// IDA_EA: 0x867C8FE
// IDA_NAME: _ZN5CUser10LogHistoryEPKcz
// DEMANGLED: CUser::LogHistory(char const*,...)
int CUser::LogHistory(const char* format, ...) {
    va_list args;
    va_start(args, format);

    cHistoryTrace history_trace;
    history_trace.source_code = isHangameUser() ? 'X' : 'D';

    TimeLog_ now;
    now.setTime();

    char account_id_slot_7[24];
    std::memset(account_id_slot_7, 0, 4u);
    std::strcpy(account_id_slot_7,
                NumberToString(static_cast<unsigned int>(get_acc_id()), 7));

    CEnvironment* environment = G_CEnvironment();
    const int channel_no = environment->get_channel_no();

    CNetwork4096_450000& network = network_;
    const int peer_port = network.get_port();
    const char* peer_ip = network.GetPeerIP2(network.get_str_ip(), 16);
    char* web_address = getWebAddress();
    const int grow_type = getCurCharFirstGrowType();
    const int charac_job = get_charac_job();
    const int charac_level = get_charac_level();
    const int charac_no = getCurCharacNo();
    const char* charac_name = getCurCharacR() ? getCurCharacName() : "";
    const char* account_id =
        NumberToString(static_cast<unsigned int>(get_acc_id()), 0);
    const char* account_name = account_name_;

    char history_line[1024];
    std::sprintf(
        history_line,
        "\"%s\",%s,%c,%02d%02d%02d,\"%s\",%d,%d,%d,%d,%s,%s,%d,%d, ",
        account_name,
        account_id,
        history_trace.source_code,
        now.hour,
        now.minute,
        now.second,
        charac_name,
        charac_no,
        charac_level,
        charac_job,
        grow_type,
        web_address,
        peer_ip,
        peer_port,
        channel_no);
    std::vsprintf(history_line + std::strlen(history_line), format, args);
    va_end(args);
    return history_trace(history_line);
}
