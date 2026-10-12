#ifndef OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_LINK_CHARACTER_H_
#define OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_LINK_CHARACTER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class Dispatcher_ConnectLinkCharacter
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ConnectLinkCharacter();
    virtual ~Dispatcher_ConnectLinkCharacter();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user,
                        MSG_BASE& message,
                        ParamBase& param);
};

class Dispatcher_DisconnectLinkCharacter
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DisconnectLinkCharacter();
    virtual ~Dispatcher_DisconnectLinkCharacter();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user,
                        MSG_BASE& message,
                        ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ConnectLinkCharacter) == 0x04u,
              "CMD309 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_DisconnectLinkCharacter) == 0x04u,
              "CMD310 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_LINK_CHARACTER_H_
