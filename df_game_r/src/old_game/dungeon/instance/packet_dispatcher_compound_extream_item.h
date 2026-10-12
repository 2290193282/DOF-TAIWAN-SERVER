#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_COMPOUND_EXTREAM_ITEM_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_COMPOUND_EXTREAM_ITEM_H_

#include <stddef.h>

#include "old_game/economy/inventory/inventory.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CEquipItem;

// IDA unit: 13_database_dispatch/Dispatcher_Compound_ExtreamItem (CMD326).
// The object owns the complete BindCube inventory transaction state.
class Dispatcher_Compound_ExtreamItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597244
    // IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItemC2Ev
    Dispatcher_Compound_ExtreamItem();

    // IDA_EA: 0x824057C / 0x82405AC
    virtual ~Dispatcher_Compound_ExtreamItem();

    // IDA_EA: 0x8225EC4
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823419A / 0x82341A4
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    friend struct Dispatcher_Compound_ExtreamItem_LayoutCheck;

    // IDA_EA: 0x822577E .. 0x8225C54
    Dispatcher_Compound_ExtreamItem* _reset();
    int _read_packet(PacketBuf& packet);
    bool _remove_victim();
    bool _push_newitem();
    void _send_result();

    unsigned short cube_slot_;              // +0x04
    short result_slot_;                     // +0x06
    unsigned short source_slots_[2];        // +0x08
    Inven_Item input_items_[2];             // +0x0c
    // The compiler restores two bytes of alignment after the packed
    // Inven_Item pair before storing the i386 pointers.
    const CEquipItem* input_prototypes_[2]; // +0x88
    Inven_Item output_item_;                // +0x90
    unsigned char selection_type_;          // +0xcd
    CUser* user_;                           // +0xd0
    unsigned char sphere_type_;             // +0xd4
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct Dispatcher_Compound_ExtreamItem_LayoutCheck {
    static_assert(
        offsetof(Dispatcher_Compound_ExtreamItem, cube_slot_) == 0x04u &&
            offsetof(Dispatcher_Compound_ExtreamItem, input_items_) == 0x0cu &&
            offsetof(Dispatcher_Compound_ExtreamItem, input_prototypes_) ==
                0x88u &&
            offsetof(Dispatcher_Compound_ExtreamItem, output_item_) == 0x90u &&
            offsetof(Dispatcher_Compound_ExtreamItem, selection_type_) ==
                0xcdu &&
            offsetof(Dispatcher_Compound_ExtreamItem, user_) == 0xd0u &&
            offsetof(Dispatcher_Compound_ExtreamItem, sphere_type_) == 0xd4u &&
            sizeof(Dispatcher_Compound_ExtreamItem) == 0xd8u,
        "Dispatcher_Compound_ExtreamItem must retain its old i386 layout");
};
#endif

#endif
