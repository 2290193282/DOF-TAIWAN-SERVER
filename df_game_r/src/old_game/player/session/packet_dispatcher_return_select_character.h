#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RETURN_SELECT_CHARACTER_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RETURN_SELECT_CHARACTER_H_

// IDA unit: 04_character_select_create/DisPatcher_ReturnToSelectCharacter.
// C2 @ 0x85959E0 proves this is the single-vptr, state-0 dispatcher object
// inserted into PacketDispatcher slot 7.

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class DisPatcher_ReturnToSelectCharacter
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_ReturnToSelectCharacter();
    virtual ~DisPatcher_ReturnToSelectCharacter();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ReturnToSelectCharacter) == 0x04u,
              "CMD7 dispatcher must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_RETURN_SELECT_CHARACTER_H_
