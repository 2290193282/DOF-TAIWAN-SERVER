#include "old_game/player/core/cuser.h"

#include "old_game/pvp/battlefield/battle_field.h"
#include "infra/network/packet/packet_guard.h"

extern bool fnEnumDungeonClear(short dungeon_index,
                               char difficulty,
                               char* callback_arguments);

// IDA_EA: 0x86781B4
// IDA_NAME: _ZN5CUser20SendDungeonClearListEv
// DEMANGLED: CUser::SendDungeonClearList(void)
void CUser::SendDungeonClearList() {
    PacketGuard packet;
    packet.put_header(0, 5);

    const WongWork::CDungeonClear* const dungeon_clear =
        reinterpret_cast<const WongWork::CDungeonClear*>(getDungeonClearRefR());
    packet.put_short(dungeon_clear->getDungeonClearSize());
    dungeon_clear->enumDungeonClear(this, fnEnumDungeonClear, &packet);

    packet.finalize(true);
    Send(packet);
}
