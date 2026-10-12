#include "old_game/player/core/cuser.h"

#include <cstring>

// IDA_EA: 0x84ECED6
// IDA_NAME: _ZN5CUser18SetGuildAgitDBInfoER17STGuildAgitDBInfo
// DEMANGLED: CUser::SetGuildAgitDBInfo(STGuildAgitDBInfo &)
void* CUser::SetGuildAgitDBInfo(STGuildAgitDBInfo& info) {
    return std::memcpy(&guild_agit_info_, &info, sizeof(guild_agit_info_));
}
