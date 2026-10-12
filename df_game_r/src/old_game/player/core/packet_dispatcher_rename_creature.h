#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RENAME_CREATURE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RENAME_CREATURE_H_

#include "old_game/player/core/creature_guild_params.h"
#include "old_game/player/core/userinfo_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_RenameCreature.
// The state-0 IPacketDispatcher specialization is owned by the matching .cpp;
// this declaration must remain base-only on the original i386 ABI.
class Dispatcher_RenameCreature
    : public IPacketDispatcher<MSG_RENAME_CREATURE,
                               stRenameCreature,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595F20
    // IDA_NAME: _ZN25Dispatcher_RenameCreatureC2Ev
    // DEMANGLED: Dispatcher_RenameCreature::Dispatcher_RenameCreature(void)
    Dispatcher_RenameCreature();

    // IDA_EA: 0x8245F5E / 0x8245F8E
    // IDA_NAME: _ZN25Dispatcher_RenameCreatureD2Ev /
    //           _ZN25Dispatcher_RenameCreatureD0Ev
    // DEMANGLED: Dispatcher_RenameCreature::~Dispatcher_RenameCreature()
    virtual ~Dispatcher_RenameCreature();

    // IDA_EA: 0x81CCFDC
    // IDA_NAME: _ZN25Dispatcher_RenameCreature4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RenameCreature::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CD110
    // IDA_NAME: _ZN25Dispatcher_RenameCreature7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RenameCreature::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CD21C
    // IDA_NAME: _ZN25Dispatcher_RenameCreature4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_RenameCreature::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CD254
    // IDA_NAME: _ZN25Dispatcher_RenameCreature11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_RenameCreature::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RenameCreature) == 0x04u,
              "CMD103 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RENAME_CREATURE_H_
