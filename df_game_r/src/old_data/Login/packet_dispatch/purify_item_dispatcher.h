#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PURIFY_ITEM_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PURIFY_ITEM_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
struct Inven_Item;

#pragma pack(push, 1)
// IDA unit: 99_misc_global/MSG_PURIFY_ITEM.  The mixed 16/32-bit wire order
// is part of CMD204's inventory identity checks; it is not a generic item
// action payload.  The dword begins at the unaligned +0x0f offset in the
// original C2, so normal C++ field alignment would be ABI-incompatible.
struct MSG_PURIFY_ITEM : public MSG_BASE {
    // IDA_EA: 0x822E42E
    // IDA_NAME: _ZN15MSG_PURIFY_ITEMC2Ev
    // DEMANGLED: MSG_PURIFY_ITEM::MSG_PURIFY_ITEM(void)
    MSG_PURIFY_ITEM();

    uint16_t target_slot;           // +0x0d
    unsigned long target_item_id;   // +0x0f, PacketBuf::get_item_idx ABI
    uint16_t material_slot;         // +0x13
    unsigned long material_item_id; // +0x15, PacketBuf::get_item_idx ABI
};
#pragma pack(pop)

#if defined(__i386__)
static_assert(sizeof(MSG_PURIFY_ITEM) == 0x19u,
              "CMD204 request must retain its old 0x19-byte ABI");
#endif

class Dispatcher_PurifyItem
    : public IPacketDispatcher<MSG_PURIFY_ITEM,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85968DC
    // IDA_NAME: _ZN21Dispatcher_PurifyItemC2Ev
    // DEMANGLED: Dispatcher_PurifyItem::Dispatcher_PurifyItem(void)
    Dispatcher_PurifyItem();

    // IDA_EA: 0x8241812 / 0x8241842
    virtual ~Dispatcher_PurifyItem();

    // IDA_EA: 0x8218B94 / 0x8218CB6
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x82194B6
    int processPurifyType(int material_type, Inven_Item& target_item);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PurifyItem) == 0x04u,
              "CMD204 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_PURIFY_ITEM_DISPATCHER_H_
