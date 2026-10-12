#include "common/manager/live_server_info_t.h"

// IDA_EA: 0x837DD64
// IDA_NAME: _ZN18live_server_info_tC2Ev
// DEMANGLED: live_server_info_t::live_server_info_t(void)
live_server_info_t::live_server_info_t()
    : std::map<int, std::vector<pvp_channel_info_t> >() {}

// IDA_EA: 0x837DD78
// IDA_NAME: _ZN18live_server_info_tD2Ev
// DEMANGLED: live_server_info_t::~live_server_info_t()
live_server_info_t::~live_server_info_t() {}
