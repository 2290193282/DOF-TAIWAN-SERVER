#ifndef OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_BUY_ITEM_H_
#define OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_BUY_ITEM_H_

#include <stdint.h>

#include "old_game/economy/shop/buy_item_param.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_BuyItem (CMD21).  The payload is
// the legacy 13-byte C2S header followed by item index, stack count/seed, and
// NPC shop id.
struct __attribute__((packed)) MSG_BUY_ITEM : MSG_BASE {
    uint32_t item_id;            // +0x0d
    uint32_t count_or_seed;      // +0x11
    uint32_t npc_id;             // +0x15
};

static_assert(sizeof(MSG_BUY_ITEM) == 0x19u,
              "CMD21 request must retain the 25-byte client ABI");

// These declarations must precede the derived class.  Its base-class use
// otherwise instantiates generic members before the one canonical CMD21
// template provider can define the old-server specializations.
template <>
IPacketDispatcher<MSG_BUY_ITEM,
                  BuyItemParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BUY_ITEM,
                  BuyItemParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BUY_ITEM,
                       BuyItemParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

class DisPatcher_BuyItem
    : public IPacketDispatcher<MSG_BUY_ITEM,
                               BuyItemParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85950A8
    // IDA_NAME: _ZN18DisPatcher_BuyItemC2Ev
    DisPatcher_BuyItem();

    // IDA_EA: 0x8231F48 / 0x8231F78
    // IDA_NAME: _ZN18DisPatcher_BuyItemD2Ev / D0Ev
    virtual ~DisPatcher_BuyItem();

    // IDA_EA: 0x81BDBA8
    // IDA_NAME: _ZN18DisPatcher_BuyItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_BuyItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81BDC7E
    // IDA_NAME: _ZN18DisPatcher_BuyItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_BuyItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81BE20A
    // IDA_NAME: _ZN18DisPatcher_BuyItem4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_BuyItem::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81BE46A
    // IDA_NAME: _ZN18DisPatcher_BuyItem11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_BuyItem::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81BE658
    // IDA_NAME: _ZN18DisPatcher_BuyItem8get_dataEP5CUserR12MSG_BUY_ITEMR12BuyItemParam
    // DEMANGLED: DisPatcher_BuyItem::get_data(CUser *,MSG_BUY_ITEM &,BuyItemParam &)
    int get_data(CUser* user, MSG_BUY_ITEM& message, BuyItemParam& param);

    // IDA_EA: 0x81BEAB0
    // IDA_NAME: _ZN18DisPatcher_BuyItem20get_nMaxBuyableCountEP5CUserR12BuyItemParam
    // DEMANGLED: DisPatcher_BuyItem::get_nMaxBuyableCount(CUser *,BuyItemParam &)
    int get_nMaxBuyableCount(CUser* user, BuyItemParam& param);

    // IDA_EA: 0x81BEC5E
    // IDA_NAME: _ZN18DisPatcher_BuyItem17statistic_processEP5CUserR12BuyItemParam
    // DEMANGLED: DisPatcher_BuyItem::statistic_process(CUser *,BuyItemParam &)
    int statistic_process(CUser* user, BuyItemParam& param);

    // IDA_EA: 0x81BED44
    // IDA_NAME: _ZN18DisPatcher_BuyItem15_onBuyItemEventERK12BuyItemParamP5CUser
    // DEMANGLED: DisPatcher_BuyItem::_onBuyItemEvent(BuyItemParam const&,CUser *)
    void _onBuyItemEvent(const BuyItemParam& param, CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_BuyItem) == 0x04u,
              "CMD21 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_BUY_ITEM_H_
