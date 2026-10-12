#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_ACTIVE_STATUS_DAMAGE_CHECK_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_ACTIVE_STATUS_DAMAGE_CHECK_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_SDC_ActiveStatus_Damage_Check.
// The dispatcher is state-0 and consumes its compressed payload directly;
// read/process are genuinely empty in the old binary.
class Dispatcher_SDC_ActiveStatus_Damage_Check
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596674
    // IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_CheckC2Ev
    Dispatcher_SDC_ActiveStatus_Damage_Check();

    // IDA_EA: 0x8243996 / 0x82439C6
    // IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_CheckD2Ev / D0Ev
    virtual ~Dispatcher_SDC_ActiveStatus_Damage_Check();

    // IDA_EA: 0x81EA16E
    // IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_Check12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233BAA / 0x8233BB4
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SDC_ActiveStatus_Damage_Check) == 0x04u,
              "CMD184 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SDC_ACTIVE_STATUS_DAMAGE_CHECK_H_
