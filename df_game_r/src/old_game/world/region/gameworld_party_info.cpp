#include "old_game/world/region/game_world.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>
#include <stdint.h>

namespace {

uint8_t HasUserEventCharacterParty(CParty* party) {
    if (!party) return 0u;
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member && member->getUserEventCharacterFlag() != 0) return 1u;
    }
    return 0u;
}

}  // namespace


// IDA_EA: 0x86C878A
// IDA_NAME: _ZN9GameWorld22send_party_info_to_allEP6CPartyi
// DEMANGLED: GameWorld::send_party_info_to_all(CParty *,int)
void GameWorld::send_party_info_to_all(CParty* party, int info_type) {
    const int member_count = party->get_member_count();
    if (member_count <= 0) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PARTY_INFO);
    packet.put_short(1);
    packet.put_short(party->GetPartyIndex());
    packet.put_byte(info_type);

    if (info_type <= 1) {
        const int title_index = party->getTitleIndex();
        packet.put_byte(title_index);
        if (!title_index) {
            char* const title = party->getTitle();
            const int title_length = static_cast<int>(std::strlen(title));
            packet.put_int(title_length);
            packet.put_str(title, title_length);
        }
        packet.put_byte(party->IsReturnUserParty());
        // 0828 keeps the return-user flag and the user-event-character-party
        // flag as separate bytes.  The latter is the OR of the authoritative
        // per-character event_charac_flag restored at login.
        packet.put_byte(HasUserEventCharacterParty(party));
        packet.put_byte(party->getUserMax());
        packet.put_short(party->getDungIndex());
        packet.put_byte(party->getDungDiffi());
        packet.put_byte(party->IsEventCharacParty());
    }

    if (!info_type || info_type == 2) {
        int manager_seat = 0;
        for (int seat = 0; seat < 8; ++seat) {
            // CLIENT_IDA_EVIDENCE: sub_597E20 reads and installs all eight
            // wire slots (0x5980F1..0x598221).  Ordinary CParty still owns
            // only four live members; the generic client-slot width is not
            // an ordinary-party capacity increase.
            CUser* const user = seat < 4 ? party->get_user(seat) : 0;
            if (user) {
                packet.put_short(user->get_unique_id());
                if (party->getManager() == user) {
                    manager_seat = seat;
                }
                int sex = user->getSex();
                if (sex == -1) {
                    sex = 1;
                }
                packet.put_byte(sex);
            } else {
                packet.put_short(0xFFFF);
                packet.put_byte(0xFF);
            }
        }
        packet.put_byte(party->IsReturnUserParty());
        packet.put_byte(HasUserEventCharacterParty(party));
        packet.put_byte(manager_seat);
        packet.put_byte(party->is_quick_party());
        packet.put_byte(party->IsEventCharacParty());
    }

    if (info_type <= 2) {
        // 0627's four-slot tag pairs do not match the 0828 tag-record layout.
        packet.put_byte(0);
    }

    packet.finalize(true);
    send_all(packet);
}
