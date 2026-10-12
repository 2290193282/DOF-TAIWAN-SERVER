#include "old_game/world/region/game_world.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86C8694
// IDA_NAME: _ZN9GameWorld20send_buddy_conn_listEjP9BuddyListi
// DEMANGLED: GameWorld::send_buddy_conn_list(unsigned int,BuddyList *,int)
void GameWorld::send_buddy_conn_list(unsigned int account_id,
                                     BuddyList* buddy_list,
                                     int count) {
    BuddyList* current = buddy_list;
    int remaining = count;
    CUser* const user = find_user_from_world_byaccid(account_id);
    if (user) {
        PacketGuard packet;
        packet.put_header(0, 56);
        packet.put_byte(count);
        while (remaining-- != 0) {
            packet.put_binary(reinterpret_cast<const char*>(current),
                              sizeof(BuddyList));
            ++current;
        }
        packet.finalize(true);
        user->Send(packet);
    }
}
