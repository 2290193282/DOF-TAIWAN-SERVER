#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// IDA unit: 13_database_dispatch/Dispatcher_SelectItem.  The concrete
// handler has no storage after the inherited dispatcher vptr.
class Dispatcher_SelectItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85977DE
    // IDA_NAME: _ZN21Dispatcher_SelectItemC2Ev
    // DEMANGLED: Dispatcher_SelectItem::Dispatcher_SelectItem(void)
    Dispatcher_SelectItem();

    // IDA_EA: 0x8240000
    // IDA_NAME: _ZN21Dispatcher_SelectItemD2Ev
    // DEMANGLED: Dispatcher_SelectItem::~Dispatcher_SelectItem()
    // IDA_EA: 0x8240030
    // IDA_NAME: _ZN21Dispatcher_SelectItemD0Ev
    // DEMANGLED: Dispatcher_SelectItem::~Dispatcher_SelectItem()
    virtual ~Dispatcher_SelectItem();

    // IDA_EA: 0x822997A
    // IDA_NAME: _ZN21Dispatcher_SelectItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_SelectItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234352
    // IDA_NAME: _ZN21Dispatcher_SelectItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_SelectItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823435C
    // IDA_NAME: _ZN21Dispatcher_SelectItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_SelectItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SelectItem) == 0x04u,
              "Dispatcher_SelectItem must retain its old vptr-only ABI");
#endif


#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_DISPATCHER_H_
