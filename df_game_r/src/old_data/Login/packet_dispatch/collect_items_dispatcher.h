#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COLLECT_ITEMS_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COLLECT_ITEMS_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD483 request.  IDA 0x81E33A4 reads precisely the two shorts at +0x0e
// and +0x10; byte +0x0d is not read or initialized by this handler.
#pragma pack(push, 1)
struct MSG_COLLECT_ITEMS : MSG_BASE {
    uint8_t raw_0d;
    int16_t item_slot;
    uint16_t item_count;
};
#pragma pack(pop)

static_assert(sizeof(MSG_COLLECT_ITEMS) == 0x12u,
              "CMD483 request must retain its legacy 18-byte extent");
static_assert(offsetof(MSG_COLLECT_ITEMS, item_slot) == 0x0eu &&
                  offsetof(MSG_COLLECT_ITEMS, item_count) == 0x10u,
              "CMD483 request field offsets changed");

// Monitor TCP packet.  PacketHeader's old 10-byte initialized prefix is
// followed by four bytes which Packet_CollectItems::C2 and CMD483 both leave
// untouched.  They are transmitted as the original stack padding; assigning
// a synthetic value would alter the old wire behavior.
#pragma pack(push, 1)
struct Packet_CollectItems : PacketHeader {
    uint8_t uninitialized_0a[4];
    uint32_t account_id;
    uint32_t item_count;
    uint32_t charac_no;
    uint8_t charac_name_length;
    char charac_name[30];

    // IDA_EA: 0x822C75A
    // IDA_NAME: _ZN19Packet_CollectItemsC2Ev
    Packet_CollectItems();
};
#pragma pack(pop)

static_assert(sizeof(Packet_CollectItems) == 0x39u,
              "Packet_CollectItems must retain the old 57-byte wire layout");
static_assert(offsetof(Packet_CollectItems, account_id) == 0x0eu &&
                  offsetof(Packet_CollectItems, item_count) == 0x12u &&
                  offsetof(Packet_CollectItems, charac_no) == 0x16u &&
                  offsetof(Packet_CollectItems, charac_name_length) == 0x1au &&
                  offsetof(Packet_CollectItems, charac_name) == 0x1bu,
              "Packet_CollectItems field offsets changed");

// IDA 0x8231C16/0x824677C/0x82467AC.  The original parameter object stores
// its result at +0x04 and echoes the two request shorts at +0x08/+0x0a.
struct stCollectItems : ParamBase {
    stCollectItems();
    virtual ~stCollectItems();

    int result;
    int16_t item_slot;
    uint16_t item_count;
};

#if defined(__i386__)
static_assert(sizeof(stCollectItems) == 0x0cu,
              "stCollectItems must retain the old 12-byte ABI");
static_assert(offsetof(stCollectItems, result) == 0x04u &&
                  offsetof(stCollectItems, item_slot) == 0x08u &&
                  offsetof(stCollectItems, item_count) == 0x0au,
              "stCollectItems field offsets changed");
#endif

// CMD483's concrete dispatcher.  It remains deliberately unregistered from
// PacketDispatcher's 606-slot root until that full root lifecycle closes.
class Dispatcher_CollectItems
    : public IPacketDispatcher<MSG_COLLECT_ITEMS,
                               stCollectItems,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597D3A
    // IDA_NAME: _ZN23Dispatcher_CollectItemsC2Ev
    Dispatcher_CollectItems();

    // IDA_EA: 0x8243FAE / 0x8243FDE
    // IDA_NAME: _ZN23Dispatcher_CollectItemsD2Ev / 
    //           _ZN23Dispatcher_CollectItemsD0Ev
    virtual ~Dispatcher_CollectItems();

    // IDA_EA: 0x81E33A4 / 0x81E343C / 0x81E360C
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E329E
    // IDA_NAME: _ZN23Dispatcher_CollectItems11check_errorEP5CUserR8MSG_BASER9ParamBase
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CollectItems) == 0x04u,
              "CMD483 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COLLECT_ITEMS_DISPATCHER_H_
