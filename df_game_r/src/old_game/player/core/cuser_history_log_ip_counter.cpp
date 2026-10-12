#include "old_game/player/core/cuser.h"


// IDA_EA: 0x8685D0C
// IDA_NAME: _ZN15cUserHistoryLog12IPCounterLogEiiiiiPc
// DEMANGLED: cUserHistoryLog::IPCounterLog(int,int,int,int,int,char *)
int cUserHistoryLog::IPCounterLog(int passed,
                                   int hack_type,
                                   int hack_sub_type,
                                   int required_count,
                                   int current_count,
                                   char* peer_ip) {
    return owner_->LogHistory("IPCount,%d,%d,%d,%d,%d,%s",
                              passed,
                              hack_type,
                              hack_sub_type,
                              required_count,
                              current_count,
                              peer_ip);
}

// IDA_EA: 0x8685D54
// IDA_NAME: _ZN15cUserHistoryLog16FullIPCounterLogEiiiiiPc
// DEMANGLED: cUserHistoryLog::FullIPCounterLog(int,int,int,int,int,char *)
int cUserHistoryLog::FullIPCounterLog(int passed,
                                       int hack_type,
                                       int hack_sub_type,
                                       int required_count,
                                       int current_count,
                                       char* peer_ip) {
    return owner_->LogHistory("D_IPCount,%d,%d,%d,%d,%d,%s",
                              passed,
                              hack_type,
                              hack_sub_type,
                              required_count,
                              current_count,
                              peer_ip);
}
