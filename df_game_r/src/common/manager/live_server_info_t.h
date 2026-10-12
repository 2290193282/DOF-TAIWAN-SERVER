#ifndef OLDSRV_GLOBAL_LIVE_SERVER_INFO_T_H_
#define OLDSRV_GLOBAL_LIVE_SERVER_INFO_T_H_

#include "old_data/channel/pvp_channel_info_t.h"

#include <map>
#include <vector>


// IDA unit: 99_misc_global/live_server_info_t.  This is a real named
// map-derived value with its own C2/D2 identities, not a typedef facade.
// CDataManager embeds it directly at +0xB4EC.
struct live_server_info_t
    : public std::map<int, std::vector<pvp_channel_info_t> > {
    // IDA_EA: 0x837DD64
    live_server_info_t();
    // IDA_EA: 0x837DD78
    ~live_server_info_t();
};

static_assert(sizeof(live_server_info_t) == 0x18u,
              "live_server_info_t must retain the i386 std::map ABI");


#endif  // OLDSRV_GLOBAL_LIVE_SERVER_INFO_T_H_
