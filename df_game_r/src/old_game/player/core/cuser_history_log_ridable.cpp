#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8686928
// IDA_NAME: _ZN15cUserHistoryLog12StartRidableENS_12eRidableTypeEi
// DEMANGLED: cUserHistoryLog::StartRidable(cUserHistoryLog::eRidableType,int)
int cUserHistoryLog::StartRidable(eRidableType ridable_type, int object_id) {
    return owner_->LogHistory(
        "Transform,%d,%d,%d", 1, object_id, static_cast<int>(ridable_type));
}

// IDA_EA: 0x868695C
// IDA_NAME: _ZN15cUserHistoryLog10EndRidableENS_12eRidableTypeEi
// DEMANGLED: cUserHistoryLog::EndRidable(cUserHistoryLog::eRidableType,int)
int cUserHistoryLog::EndRidable(eRidableType ridable_type, int object_id) {
    return owner_->LogHistory(
        "Transform,%d,%d,%d", 0, object_id, static_cast<int>(ridable_type));
}
