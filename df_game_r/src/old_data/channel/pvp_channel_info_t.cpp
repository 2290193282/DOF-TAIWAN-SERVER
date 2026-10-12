#include "old_data/channel/pvp_channel_info_t.h"


typedef char pvp_channel_info_t_Size_Must_Be_0x24[
    (sizeof(pvp_channel_info_t) == 0x24) ? 1 : -1];

// IDA_EA: 0x8370912
// IDA_NAME: _ZN18pvp_channel_info_tC2Ev
// DEMANGLED: pvp_channel_info_t::pvp_channel_info_t(void)
pvp_channel_info_t::pvp_channel_info_t()
    : channel_no(0),
      min_grade(0),
      max_grade(0),
      upgrade_revision(0),
      channel_type(0),
      min_level(0),
      max_level(0),
      pvp_mode(0),
      team_count(0) {}

// IDA_EA: 0x893812E
// IDA_NAME: _ZN18pvp_channel_info_tD2Ev
// DEMANGLED: pvp_channel_info_t::~pvp_channel_info_t()
pvp_channel_info_t::~pvp_channel_info_t() {}
