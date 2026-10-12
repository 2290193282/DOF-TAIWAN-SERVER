#include "old_game/player/core/cuser.h"

#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x865DB6C
// IDA_NAME: _ZN5CUser14send_itemspaceEi
// DEMANGLED: CUser::send_itemspace(int)
int CUser::send_itemspace(int item_space) {
    PacketGuard packet;
    switch (item_space) {
    case 0:
        if (getCurCharacInvenR()->MakeItemList(
                static_cast<INVEN_TYPE>(1), &packet) != 1) {
            return 0;
        }
        return Send(packet);

    case 1:
        if (getCurCharacInvenR()->MakeItemList(
                static_cast<INVEN_TYPE>(2), &packet) != 1) {
            return 0;
        }
        return Send(packet);

    case 2:
        if (getCurCharacCargoR()->MakeItemList(&packet) != 1) {
            return 0;
        }
        return Send(packet);

    case 7:
        // The official itemspace-7 sequence is two S2C packets: NOTI105
        // creature metadata first, then NOTI13 itemspace 7.  Preserve the
        // short-circuit so a failed first send never builds the second body.
        if (getCurCharacInvenR()->GetCreatureMgrR()->SendCreatureItemList() !=
                1 ||
            getCurCharacInvenR()->MakeItemList(
                static_cast<INVEN_TYPE>(3), &packet) != 1) {
            return 0;
        }
        return Send(packet);

    case 12:
        if (IsExistAccountCargo()) {
            return static_cast<CAccountCargo*>(GetAccountCargo())
                ->SendItemList();
        }
        return 1;
    }
    return 0;
}
