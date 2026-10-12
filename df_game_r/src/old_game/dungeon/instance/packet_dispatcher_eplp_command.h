#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_EPLP_COMMAND_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_EPLP_COMMAND_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CParty;
class CUser;

// CMD075 is read directly by its state-0 dispatcher.  These two bytes begin
// immediately after the fixed 13-byte MSG_BASE client header.
#pragma pack(push, 1)
struct MSG_EPLP_COMMAND : MSG_BASE {
    uint8_t request;  // +0x0d
    uint8_t command;  // +0x0e
};
#pragma pack(pop)

static_assert(sizeof(MSG_EPLP_COMMAND) == 0x0fu,
              "CMD075 request must retain its 15-byte wire ABI");

// IDA unit: 13_database_dispatch/Dispatcher_EPLPCommand.
class Dispatcher_EPLPCommand
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595BA0
    Dispatcher_EPLPCommand();
    // IDA_EA: 0x8243552 / 0x8243582
    virtual ~Dispatcher_EPLPCommand();

    // IDA_EA: 0x81FEEDC
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8233508 / 0x8233512
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    // IDA_EA: 0x81FED4C / 0x81FEE14
    void _SendEPLPError(CParty* party, uint8_t command, uint8_t error);
    void _BroadCastPacket(CParty* party, MSG_EPLP_COMMAND& request);

    // IDA_EA: 0x825F760 / 0x825F7F4
    bool excludeEplpState(CUser* user);
    bool isReTurnToVillage(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EPLPCommand) == 0x04u,
              "CMD075 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_EPLP_COMMAND_H_
