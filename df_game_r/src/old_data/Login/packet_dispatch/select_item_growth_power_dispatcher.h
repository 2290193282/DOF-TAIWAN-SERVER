#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_GROWTH_POWER_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_GROWTH_POWER_DISPATCHER_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD432.  Preserve the original spelling in the exported C++ identities:
// "Grwoth" is a typo in the retail class name, not a new compatibility name.
// The object is a vptr-only state-3 dispatcher and remains unregistered until
// the full root PacketDispatcher construction/teardown transaction is owned.
class Dispatcher_Select_Item_Grwoth_Power
    : public IPacketDispatcher<MSG_GROWTH_POWER,
                               stGrowthPowerParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85978F6
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_PowerC2Ev
    Dispatcher_Select_Item_Grwoth_Power();

    // IDA_EA: 0x824496E / 0x824499E
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_PowerD2Ev /
    //           _ZN35Dispatcher_Select_Item_Grwoth_PowerD0Ev
    virtual ~Dispatcher_Select_Item_Grwoth_Power();

    // IDA_EA: 0x81DB4C4
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DB60C
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DBC66
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81DB580
    // IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power11check_errorEP5CUserR8MSG_BASER9ParamBase
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Select_Item_Grwoth_Power) == 0x04u,
              "CMD432 must retain the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_ITEM_GROWTH_POWER_DISPATCHER_H_
