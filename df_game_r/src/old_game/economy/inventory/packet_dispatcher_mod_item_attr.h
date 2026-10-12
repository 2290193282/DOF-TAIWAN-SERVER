#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOD_ITEM_ATTR_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOD_ITEM_ATTR_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD84 is an old direct dispatcher.  It consumes two inventory slots and a
// client item index, then performs the reseal or random-attribute mutation.
class Dispatcher_ModItemAttr
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595D28
    // IDA_NAME: _ZN22Dispatcher_ModItemAttrC2Ev
    Dispatcher_ModItemAttr();

    // IDA_EA: 0x8243246 / 0x8243276
    // IDA_NAME: _ZN22Dispatcher_ModItemAttrD2Ev / D0Ev
    virtual ~Dispatcher_ModItemAttr();

    // IDA_EA: 0x8200B08
    // IDA_NAME: _ZN22Dispatcher_ModItemAttr12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8201938
    // IDA_NAME: _ZN22Dispatcher_ModItemAttr11_SendResultEP5CUserttt
    void _SendResult(CUser* user,
                     uint16_t operation,
                     uint16_t material_slot,
                     uint16_t target_slot);

    // IDA_EA: 0x82335D0 / 0x82335DA
    // IDA_NAME: _ZN22Dispatcher_ModItemAttr4readER9PacketBufR8MSG_BASE /
    //           _ZN22Dispatcher_ModItemAttr7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ModItemAttr) == 0x04u,
              "CMD84 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOD_ITEM_ATTR_H_
