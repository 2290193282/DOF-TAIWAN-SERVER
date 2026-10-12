#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x80D84B4
// IDA_NAME: _ZNK10BlueMarble22sendBlueMarbleRoomInfoEv
// DEMANGLED: BlueMarble::sendBlueMarbleRoomInfo(void)const
void BlueMarble::sendBlueMarbleRoomInfo() const {
    PacketGuard packet;
    packet.put_header(0, 430);
    packet.put_short(getMemoryPoolIndex());
    packet.put_byte(getWaitUserCount());
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        CUser* const user = users_[slot].getUser();
        if (user) {
            packet.put_byte(slot);
            packet.put_short(user->get_unique_id());
        }
    }
    packet.finalize(true);
    sendToBlueMarble(packet);
}

// IDA_EA: 0x80D8600
// IDA_NAME: _ZNK10BlueMarble22sendBlueMarbleDiceInfoEi
// DEMANGLED: BlueMarble::sendBlueMarbleDiceInfo(int)const
void BlueMarble::sendBlueMarbleDiceInfo(int user_slot) const {
    PacketGuard packet;
    packet.put_header(0, 431);
    packet.put_byte(user_slot);
    packet.put_byte(getDiceNumber());
    packet.finalize(true);
    sendToBlueMarble(packet);
}

// IDA_EA: 0x80D86B2
// IDA_NAME: _ZNK10BlueMarble19sendStartBlueMarbleEv
// DEMANGLED: BlueMarble::sendStartBlueMarble(void)const
void BlueMarble::sendStartBlueMarble() const {
    PacketGuard packet;
    packet.put_header(0, 432);
    packet.put_byte(getBlueMarbleMap());
    packet.finalize(true);
    sendToBlueMarble(packet);
}

// IDA_EA: 0x80D8752
// IDA_NAME: _ZNK10BlueMarble20sendFinishBlueMarbleEv
// DEMANGLED: BlueMarble::sendFinishBlueMarble(void)const
void BlueMarble::sendFinishBlueMarble() const {
    const int winner = getWinner();
    if (winner == -1) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 433);
    packet.put_byte(winner);
    packet.finalize(true);
    sendToBlueMarble(packet);
}

// IDA_EA: 0x80D8802
// IDA_NAME: _ZNK10BlueMarble23sendBoardZoneActionInfoEiij
// DEMANGLED: BlueMarble::sendBoardZoneActionInfo(int,int,unsigned int)const
void BlueMarble::sendBoardZoneActionInfo(int user_slot,
                                         int dungeon_index,
                                         unsigned int item_index) const {
    PacketGuard packet;
    packet.put_header(0, 434);
    packet.put_short(dungeon_index);
    packet.put_int(static_cast<int>(item_index));
    packet.finalize(true);
    CUser* const user = users_[user_slot].getUser();
    if (user) {
        user->Send(packet);
    }
}

// IDA_EA: 0x80D88C2
// IDA_NAME: _ZNK10BlueMarble18sendTileGiveUpInfoEi
// DEMANGLED: BlueMarble::sendTileGiveUpInfo(int)const
void BlueMarble::sendTileGiveUpInfo(int user_slot) const {
    PacketGuard packet;
    packet.put_header(0, 435);
    packet.put_byte(user_slot);
    packet.finalize(true);
    sendToBlueMarble(packet);
}

// IDA_EA: 0x80D8956
// IDA_NAME: _ZNK10BlueMarble18sendUpdateItemInfoEii
// DEMANGLED: BlueMarble::sendUpdateItemInfo(int,int)const
void BlueMarble::sendUpdateItemInfo(int user_slot, int inventory_slot) const {
    CUser* const user = users_[user_slot].getUser();
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    CInventory* const inventory = user->getCurCharacInvenR();
    if (inventory) {
        inventory->MakeItemPacket(static_cast<INVEN_TYPE>(1),
                                  inventory_slot,
                                  packet);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x80D99CE
// IDA_NAME: _ZNK10BlueMarble16sendToBlueMarbleER11PacketGuard
// DEMANGLED: BlueMarble::sendToBlueMarble(PacketGuard &)const
void BlueMarble::sendToBlueMarble(PacketGuard& packet) const {
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        CUser* const user = users_[slot].getUser();
        if (user) {
            user->Send(packet);
        }
    }
}
