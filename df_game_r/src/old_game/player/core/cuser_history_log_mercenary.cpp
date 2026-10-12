#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8686ACA
// IDA_NAME: _ZN15cUserHistoryLog20MercenaryCompetitionEjcc
// DEMANGLED: cUserHistoryLog::MercenaryCompetition(unsigned int,char,char)
int cUserHistoryLog::MercenaryCompetition(unsigned int charac_no,
                                          char competition_type,
                                          char competition_count) {
    return owner_->LogHistory("MercenaryCompetition,%u,%d,%d",
                              charac_no,
                              competition_type,
                              competition_count);
}

// IDA_EA: 0x8686B0A
// IDA_NAME: _ZN15cUserHistoryLog15MercenaryReturnEjiiiii
// DEMANGLED: cUserHistoryLog::MercenaryReturn(
//             unsigned int,int,int,int,int,int)
int cUserHistoryLog::MercenaryReturn(unsigned int charac_no,
                                     int start_time,
                                     int end_time,
                                     int period_term,
                                     int gold,
                                     int item_count) {
    return owner_->LogHistory("MecenaryReturn,%u,%d,%d,%d,%d,%d",
                              charac_no,
                              start_time,
                              end_time,
                              period_term,
                              gold,
                              item_count);
}
