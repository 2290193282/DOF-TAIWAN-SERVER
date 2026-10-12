#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SR08_QUEST_SKILL_UNCLAIMED_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SR08_QUEST_SKILL_UNCLAIMED_H_

#include <stddef.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 99_misc_global/stEnchantParam.
//
// This is the response object shared by the third-chronicle select/enchant
// pair.  It is intentionally kept beside its first restored concrete owner
// rather than represented as an untyped four-byte scratch buffer.
class stEnchantParam : public ParamBase {
public:
    // IDA_EA: 0x823154C
    // IDA_NAME: _ZN14stEnchantParamC2Ev
    // DEMANGLED: stEnchantParam::stEnchantParam(void)
    stEnchantParam();

    // IDA_EA: 0x82470D8
    // IDA_NAME: _ZN14stEnchantParamD2Ev
    // DEMANGLED: stEnchantParam::~stEnchantParam()
    // IDA_EA: 0x8247108
    // IDA_NAME: _ZN14stEnchantParamD0Ev
    // DEMANGLED: stEnchantParam::~stEnchantParam()
    virtual ~stEnchantParam();

    int result;  // IDA +0x04
};

// PacketDispatcher slot 370.  The old server intentionally does no work in
// read/process for this selection acknowledgement; the state/trade gate is
// the whole concrete behavior.  Registration stays disabled while the
// original 606-slot PacketDispatcher root remains unavailable.
class Dispatcher_Select_3rdChronicleItem_ForEnchant
    : public IPacketDispatcher<MSG_BASE,
                               stEnchantParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597602
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantC2Ev
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::Dispatcher_Select_3rdChronicleItem_ForEnchant(void)
    Dispatcher_Select_3rdChronicleItem_ForEnchant();

    // IDA_EA: 0x8245418
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantD2Ev
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::~Dispatcher_Select_3rdChronicleItem_ForEnchant()
    // IDA_EA: 0x8245448
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchantD0Ev
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::~Dispatcher_Select_3rdChronicleItem_ForEnchant()
    virtual ~Dispatcher_Select_3rdChronicleItem_ForEnchant();

    // IDA_EA: 0x81D3416
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D3420
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8232854
    // IDA_NAME: _ZN45Dispatcher_Select_3rdChronicleItem_ForEnchant11check_stateEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Select_3rdChronicleItem_ForEnchant::check_state(CUser *,PacketBuf &)
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stEnchantParam) == 0x08u,
              "stEnchantParam must retain the old i386 ABI");
static_assert(sizeof(Dispatcher_Select_3rdChronicleItem_ForEnchant) == 0x04u,
              "slot 370 dispatcher must remain vptr-only on i386");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(stEnchantParam, result) == 0x04u,
              "stEnchantParam::result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_SR08_QUEST_SKILL_UNCLAIMED_H_
