#include "old_game/player/core/cuser.h"


// IDA_EA: 0x8685320
// IDA_NAME: _ZN15cUserHistoryLog12HackDetectedEjjjjjj
// DEMANGLED: cUserHistoryLog::HackDetected(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int)
int cUserHistoryLog::HackDetected(unsigned int hack_type,
                                  unsigned int count,
                                  unsigned int server_count,
                                  unsigned int accumulated_count,
                                  unsigned int value1,
                                  unsigned int value2) {
    if (value1 == 0u && value2 == 0u) {
        return owner_->LogHistory("Hack,%d,%d,%d,%d",
                                  hack_type,
                                  count,
                                  server_count,
                                  accumulated_count);
    }
    if (value2 == 0u) {
        return owner_->LogHistory("Hack,%d,%d,%d,%d,%d",
                                  hack_type,
                                  count,
                                  server_count,
                                  accumulated_count,
                                  value1);
    }
    return owner_->LogHistory("Hack,%d,%d,%d,%d,%d,%d",
                              hack_type,
                              count,
                              server_count,
                              accumulated_count,
                              value1,
                              value2);
}
