#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/user.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

CDataManager* G_CDataManager();

// IDA_EA: 0x8656808
// IDA_NAME: _ZN5CUser8SendCoinEv
// DEMANGLED: CUser::SendCoin(void)
void CUser::SendCoin() {
    if (!getCurCharacR()) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    CInventory* inventory =
        reinterpret_cast<CInventory*>(getCurCharacInvenR());
    if (inventory->MakeItemPacket(static_cast<INVEN_TYPE>(1), 1, packet) ==
        1) {
        packet.finalize(1);
        Send(packet);
    }
}

// IDA_EA: 0x867C110
// IDA_NAME: _ZN5CUser23SendCmdErrorPacket2ByteE14ENUM_CMDPACKETt
// DEMANGLED: CUser::SendCmdErrorPacket2Byte(ENUM_CMDPACKET,unsigned short)
bool CUser::SendCmdErrorPacket2Byte(ENUM_CMDPACKET command,
                                    unsigned short error) {
    PacketGuard packet;
    packet.put_header(1, static_cast<int>(command));
    // CLIENT_IDA_EVIDENCE: the common CMD receive path reads
    // result@0x93E9A3 and, on failure, generic_error@0x93E9F4 before
    // dispatching CMD80.  The upgrade handler's failure branch then reads the
    // real u16 detail error
    // at 0x57BBCC; its 15-byte success record is not consumed on this path.
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_short(error);
    packet.finalize(1);
    return Send(packet);
}

// IDA_EA: 0x868F60E
// IDA_NAME: _ZN5CUser26verifyStackableItemProcessEN15StackableAction4TypeEPK5CItem
// DEMANGLED: CUser::verifyStackableItemProcess(StackableAction::Type,CItem const*)
bool CUser::verifyStackableItemProcess(StackableAction::Type action,
                                       const CItem* item) {
    if (!item) {
        return 0;
    }
    if (item->is_stackable() != 1) {
        return 0;
    }
    CStackableItem* stackable = const_cast<CStackableItem*>(
        reinterpret_cast<const CStackableItem*>(item));
    if (!stackable->verifyStackableAction(action)) {
        return 0;
    }
    if (stackable->verifyUsablePlace(this)) {
        return stackable->verifyUsableServerType();
    }
    return 0;
}

// IDA_EA: 0x868F69A
// IDA_NAME: _ZN5CUser30verifyStackableItemActionPlaceEN15StackableAction4TypeEPK5CItem
// DEMANGLED: CUser::verifyStackableItemActionPlace(StackableAction::Type,CItem const*)
bool CUser::verifyStackableItemActionPlace(StackableAction::Type action,
                                           const CItem* item) {
    return verifyStackableItemProcess(action, item);
}

// IDA_EA: 0x868F6BC
// IDA_NAME: _ZN5CUser30verifyStackableItemActionPlaceEN15StackableAction4TypeEj
// DEMANGLED: CUser::verifyStackableItemActionPlace(StackableAction::Type,unsigned int)
bool CUser::verifyStackableItemActionPlace(StackableAction::Type action,
                                           unsigned int item_id) {
    CItem* item = G_CDataManager()->find_item(static_cast<int>(item_id));
    return verifyStackableItemProcess(action, item);
}
