#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_INVEST_ITEM_AMPLIFY_OPTION_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_INVEST_ITEM_AMPLIFY_OPTION_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_InvestItemAmplifyOption (CMD208).
class Dispatcher_InvestItemAmplifyOption
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85968F8
    Dispatcher_InvestItemAmplifyOption();

    // IDA_EA: 0x82417C4 / 0x82417F4
    virtual ~Dispatcher_InvestItemAmplifyOption();

    // IDA_EA: 0x82194F4
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233D3A / 0x8233D44
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_InvestItemAmplifyOption) == 0x04u,
              "CMD208 must retain the old vptr-only dispatcher ABI");
#endif

#endif
