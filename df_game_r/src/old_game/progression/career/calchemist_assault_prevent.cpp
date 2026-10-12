#include "old_game/progression/quest/quest_runtime.h"

#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85D137C
// IDA_NAME: _ZN10expert_job10CAlchemist17GetAssaultPreventEP5CUserPb
int expert_job::CAlchemist::GetAssaultPrevent(
    CUser* user, bool* party_member_prevent) {
    if (party_member_prevent) *party_member_prevent = false;
    if (!user) return 0;

    if (IsExistContinuousEffectItem(user, 0x27AC4A)) return 1;

    CParty* const party = user->GetParty();
    if (!party) return 0;
    for (int slot = 0; slot <= 3; ++slot) {
        CUser* const member = party->get_user(slot);
        if (member && member != user &&
            IsExistContinuousEffectItem(member, 0x27AC4A)) {
            if (party_member_prevent) *party_member_prevent = true;
            return 1;
        }
    }
    return 0;
}
