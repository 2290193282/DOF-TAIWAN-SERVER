#include "old_game/social/party/party.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85B9FD4
// IDA_NAME: _ZN6CParty18IsEventCharacPartyEv
// DEMANGLED: CParty::IsEventCharacParty(void)
int CParty::IsEventCharacParty() {
    EventNewCharacterReward* const event =
        static_cast<EventNewCharacterReward*>(
            GlobalData::s_event_manager->GetRepeatEvent(162));
    if (!event || event->IsEventing(0) != 1) {
        return 0;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) == 1u) {
            CUser* const user = members_[seat].user;
            if (user && event->isEventCharacter(user->get_charac_job())) {
                return 1;
            }
        }
    }
    return 0;
}
