#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8684AF0
// IDA_NAME: _ZN15cUserHistoryLog15BloodRoundClearEijjPKcS1_
// DEMANGLED: cUserHistoryLog::BloodRoundClear(int,unsigned int,unsigned int,char const*,char const*)
int cUserHistoryLog::BloodRoundClear(int blood_round,
                                     unsigned int round_seconds,
                                     unsigned int /*total_play_seconds*/,
                                     const char* /*dungeon_name*/,
                                     const char* member_names) {
    return owner_->LogHistory("BloodRoundClear,%d,%u,%s",
                              blood_round,
                              round_seconds,
                              member_names);
}
