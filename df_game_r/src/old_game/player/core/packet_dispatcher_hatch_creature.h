#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HATCH_CREATURE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HATCH_CREATURE_H_

#include "old_game/player/core/creature_guild_params.h"
#include "old_game/player/core/userinfo_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_HatchCreature.  This class
// has no trailing state beyond the IPacketDispatcher base on i386.
class Dispatcher_HatchCreature
    : public IPacketDispatcher<MSG_HATCH_CREATURE,
                               stHatchCreature,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8595F58
    // IDA_NAME: _ZN24Dispatcher_HatchCreatureC2Ev
    // DEMANGLED: Dispatcher_HatchCreature::Dispatcher_HatchCreature(void)
    Dispatcher_HatchCreature();

    // IDA_EA: 0x8245F10 / 0x8245F40
    // IDA_NAME: _ZN24Dispatcher_HatchCreatureD2Ev /
    //           _ZN24Dispatcher_HatchCreatureD0Ev
    // DEMANGLED: Dispatcher_HatchCreature::~Dispatcher_HatchCreature()
    virtual ~Dispatcher_HatchCreature();

    // IDA_EA: 0x81CD3AE
    // IDA_NAME: _ZN24Dispatcher_HatchCreature4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_HatchCreature::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CD446
    // IDA_NAME: _ZN24Dispatcher_HatchCreature7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_HatchCreature::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CD572
    // IDA_NAME: _ZN24Dispatcher_HatchCreature4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_HatchCreature::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CD5AA
    // IDA_NAME: _ZN24Dispatcher_HatchCreature11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_HatchCreature::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_HatchCreature) == 0x04u,
              "CMD105 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HATCH_CREATURE_H_
