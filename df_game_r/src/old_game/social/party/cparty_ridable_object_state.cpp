#include "old_game/social/party/party.h"

// IDA_EA: 0x85B9F28
// IDA_NAME: _ZN6CParty21SetRidableObjectStateEi19eRidableObjectState
// DEMANGLED: CParty::SetRidableObjectState(int,eRidableObjectState)
int CParty::SetRidableObjectState(int object_id,
                                  eRidableObjectState requested_state) {
    return battle_field_.SetRidableObjectState(object_id, requested_state);
}
