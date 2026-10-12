#ifndef OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_PACKET_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_PACKET_DISPATCHER_H_

#include "old_game/player/core/title_book_dispatcher_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class Dispatcher_TitleBookPut
    : public IPacketDispatcher<MSG_TITLEBOOK_PUT,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_TitleBookPut();
    virtual ~Dispatcher_TitleBookPut();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_TitleBookGet
    : public IPacketDispatcher<MSG_TITLEBOOK_GET,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_TitleBookGet();
    virtual ~Dispatcher_TitleBookGet();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_TitleBookOtherUser
    : public IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_TitleBookOtherUser();
    virtual ~Dispatcher_TitleBookOtherUser();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_TitleBookPut) == 0x04u,
              "CMD412 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_TitleBookGet) == 0x04u,
              "CMD413 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_TitleBookOtherUser) == 0x04u,
              "CMD424 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_PACKET_DISPATCHER_H_
