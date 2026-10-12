#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_CREATURE_EVOLUTION_ITEM_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_CREATURE_EVOLUTION_ITEM_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// IDA unit: 06_userinfo_stat_inventory/Dispatcher_UseCreatureEvolutionItem.
class Dispatcher_UseCreatureEvolutionItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597736
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemC2Ev
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::Dispatcher_UseCreatureEvolutionItem(void)
    Dispatcher_UseCreatureEvolutionItem();

    // IDA_EA: 0x82400EA
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemD2Ev
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::~Dispatcher_UseCreatureEvolutionItem()
    // IDA_EA: 0x824011A
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemD0Ev
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::~Dispatcher_UseCreatureEvolutionItem()
    virtual ~Dispatcher_UseCreatureEvolutionItem();

    // IDA_EA: 0x8228A76
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8228BCC
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem7processEP5CUserhsi
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::process(CUser *,unsigned char,short,int)
    int process(CUser* user,
                uint8_t item_space,
                int16_t item_slot,
                int member_count);

    // IDA_EA: 0x8234302
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823430C
    // IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UseCreatureEvolutionItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseCreatureEvolutionItem) == 0x04u,
              "Dispatcher_UseCreatureEvolutionItem must retain old vptr-only ABI");
#endif


#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_CREATURE_EVOLUTION_ITEM_DISPATCHER_H_
