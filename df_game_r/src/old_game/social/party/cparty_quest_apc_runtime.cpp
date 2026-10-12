#include "old_game/social/party/party.h"

#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85B65A2
// IDA_NAME: _ZN6CParty26HasNoQuestGoWithApcInPartyEPK5Quest
// DEMANGLED: CParty::HasNoQuestGoWithApcInParty(Quest const*)
int CParty::HasNoQuestGoWithApcInParty(const Quest* quest) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat)) {
            CUser* const user = get_user(seat);
            if (user->get_state() == 5 || user->get_state() == 10) {
                const int quest_index = quest->quest_index;
                if (user->getCurCharacQuestR()->IsDoingQuest(quest_index) &&
                    quest->trigger_type == 3 && !quest->trigger_subtype) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
