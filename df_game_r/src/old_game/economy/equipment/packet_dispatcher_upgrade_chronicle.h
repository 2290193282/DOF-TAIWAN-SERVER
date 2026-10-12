#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_CHRONICLE_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_CHRONICLE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD274 is a direct, state-0 packet transaction.  Its request is intentionally
// parsed by dispatch_sig; the base read/process virtual slots are empty in the
// old binary.
class Dispatcher_UpgradeChronicle
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596EC4
    // IDA_NAME: _ZN27Dispatcher_UpgradeChronicleC2Ev
    Dispatcher_UpgradeChronicle();
    // IDA_EA: 0x8240C30 / 0x8240C60
    // IDA_NAME: _ZN27Dispatcher_UpgradeChronicleD2Ev / D0Ev
    virtual ~Dispatcher_UpgradeChronicle();

    // IDA_EA: 0x8233C50
    // IDA_NAME: _ZN27Dispatcher_UpgradeChronicle12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8234032 / 0x823403C
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpgradeChronicle) == 0x04u,
              "CMD274 dispatcher must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_CHRONICLE_H_
