#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_PC_ROOM_RENT_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_PC_ROOM_RENT_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/MSG_RENT_ITEM.
// IDA_EA: 0x822E334
// IDA_NAME: _ZN13MSG_RENT_ITEMC2Ev
// DEMANGLED: MSG_RENT_ITEM::MSG_RENT_ITEM(void)
#pragma pack(push, 1)
struct MSG_RENT_ITEM : public MSG_BASE {
    struct Selection {
        uint8_t section_index;
        uint8_t package_index;
    };

    MSG_RENT_ITEM();

    // The reader at 0x81E512E rejects every count other than one before it
    // touches the sole selection pair.  This one-element typed record is the
    // original packet extent, not a compatibility cap.
    int8_t selection_count;          // +0x0d
    int8_t rental_type;              // +0x0e
    Selection selections[1];         // +0x0f
};
#pragma pack(pop)

static_assert(sizeof(MSG_RENT_ITEM::Selection) == 0x02u,
              "PC-room rent selection must retain its two-byte wire ABI");
static_assert(sizeof(MSG_RENT_ITEM) == 0x11u,
              "MSG_RENT_ITEM must retain its original 17-byte extent");
static_assert(offsetof(MSG_RENT_ITEM, selection_count) == 0x0du,
              "MSG_RENT_ITEM count must remain at +0x0d");
static_assert(offsetof(MSG_RENT_ITEM, rental_type) == 0x0eu,
              "MSG_RENT_ITEM rental type must remain at +0x0e");
static_assert(offsetof(MSG_RENT_ITEM, selections) == 0x0fu,
              "MSG_RENT_ITEM selection must remain at +0x0f");

// IDA unit: 99_misc_global/stRentItemParam.
// IDA_EA: C2/D2/D0 @0x8231CDA/0x82466FC/0x824675E.
struct stRentItemParam : public ParamBase {
    stRentItemParam();
    virtual ~stRentItemParam();

    int error_code;                                // +0x04
    InstanceRentalSystem::RentOutput rent_output;  // +0x08
};

#if defined(__i386__)
static_assert(sizeof(stRentItemParam) == 0x20u,
              "stRentItemParam must retain its original 32-byte ABI");
static_assert(offsetof(stRentItemParam, error_code) == 0x04u,
              "stRentItemParam error must remain at +0x04");
static_assert(offsetof(stRentItemParam, rent_output) == 0x08u,
              "stRentItemParam output must remain at +0x08");
#endif

// One A-owned object emits this complete generic instantiation.  `extern
// template` suppresses accidental COMDAT emission in the derived dispatcher
// while still allowing packet_dispatcher_pc_room_rent_templates.cpp to emit
// the real members; member-specialization declarations here would instead
// suppress that provider too.
extern template class IPacketDispatcher<MSG_RENT_ITEM,
                                        stRentItemParam,
                                        static_cast<ch_state>(0)>;

// IDA unit: 09_dungeon_pvp_party/Dispatcher_PcRoomRentItem.
// IDA_EA: read/process/send/check/give @0x81E512E/0x81E52C0/0x81E542E/
//         0x81E5762/0x81E583E; D2/D0/C2 @0x8243D8C/0x8243DBC/0x8597D8E.
class Dispatcher_PcRoomRentItem
    : public IPacketDispatcher<MSG_RENT_ITEM,
                               stRentItemParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_PcRoomRentItem();
    virtual ~Dispatcher_PcRoomRentItem();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
    void giveDimensionItems(CUser* user,
                            InstanceRentalSystem::RentInput& input);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PcRoomRentItem) == 0x04u,
              "CMD494 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_PC_ROOM_RENT_H_
