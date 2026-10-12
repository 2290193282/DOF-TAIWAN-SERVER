#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_LEVELUP_SUPPORT_REQUEST_ITEM_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_LEVELUP_SUPPORT_REQUEST_ITEM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_LevelupSupportReqestItem.
// It is intentionally not installed into the frozen 606-entry root table.
class Dispatcher_LevelupSupportReqestItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597F16
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItemC2Ev
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::Dispatcher_LevelupSupportReqestItem(void)
    Dispatcher_LevelupSupportReqestItem();

    // IDA_EA: 0x8260956 / 0x8260986
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItemD2Ev / D0Ev
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::~Dispatcher_LevelupSupportReqestItem()
    virtual ~Dispatcher_LevelupSupportReqestItem();

    // IDA_EA: 0x825FC3A
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82605E4
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82605EE
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x82605F8
    // IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_LevelupSupportReqestItem::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_LevelupSupportReqestItem) == 0x04u,
              "CMD597 dispatcher must retain its vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_LEVELUP_SUPPORT_REQUEST_ITEM_H_
