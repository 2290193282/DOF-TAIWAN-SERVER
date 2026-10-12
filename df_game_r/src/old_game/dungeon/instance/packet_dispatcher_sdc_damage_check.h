#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_DAMAGE_CHECK_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_DAMAGE_CHECK_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_SDC_Damage_Check (CMD183).
class Dispatcher_SDC_Damage_Check
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596658
    // IDA_NAME: _ZN27Dispatcher_SDC_Damage_CheckC2Ev
    Dispatcher_SDC_Damage_Check();

    // IDA_EA: 0x82439E4 / 0x8243A14
    // IDA_NAME: _ZN27Dispatcher_SDC_Damage_CheckD2Ev /
    //           _ZN27Dispatcher_SDC_Damage_CheckD0Ev
    virtual ~Dispatcher_SDC_Damage_Check();

    // IDA_EA: 0x81E9E84
    // IDA_NAME: _ZN27Dispatcher_SDC_Damage_Check12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B96 / 0x8233BA0
    // IDA_NAME: _ZN27Dispatcher_SDC_Damage_Check4readER9PacketBufR8MSG_BASE /
    //           _ZN27Dispatcher_SDC_Damage_Check7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SDC_Damage_Check) == 0x04u,
              "CMD183 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_DAMAGE_CHECK_H_
