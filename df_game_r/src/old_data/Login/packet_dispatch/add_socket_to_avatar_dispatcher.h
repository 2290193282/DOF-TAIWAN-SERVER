#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_ADD_SOCKET_TO_AVATAR_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_ADD_SOCKET_TO_AVATAR_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA 0x822DD2C constructs MSG_BASE then zeroes only these three C2S
// payload fields.  The request is used directly by CMD209's dispatch_sig;
// it is not a template-owned read/process message.
#pragma pack(push, 1)
struct MSG_ADD_SOCKET_TO_AVATAR : MSG_BASE {
    uint16_t avatar_slot;     // IDA +0x0d
    uint32_t avatar_item_id;  // IDA +0x0f
    uint16_t material_slot;   // IDA +0x13

    // IDA_EA: 0x822DD2C
    // IDA_NAME: _ZN24MSG_ADD_SOCKET_TO_AVATARC2Ev
    // DEMANGLED: MSG_ADD_SOCKET_TO_AVATAR::MSG_ADD_SOCKET_TO_AVATAR(void)
    MSG_ADD_SOCKET_TO_AVATAR();
};
#pragma pack(pop)

static_assert(sizeof(MSG_ADD_SOCKET_TO_AVATAR) == 0x15u,
              "CMD209 request must retain its old 21-byte extent");
static_assert(offsetof(MSG_ADD_SOCKET_TO_AVATAR, avatar_slot) == 0x0du &&
                  offsetof(MSG_ADD_SOCKET_TO_AVATAR, avatar_item_id) == 0x0fu &&
                  offsetof(MSG_ADD_SOCKET_TO_AVATAR, material_slot) == 0x13u,
              "CMD209 request field offsets changed");

// CMD209 is restored as a source-complete direct dispatcher.  It is kept out
// of the unfinished root PacketDispatcher 606-slot registration table.
class Dispatcher_AddSocketToAvatar
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596914
    // IDA_NAME: _ZN28Dispatcher_AddSocketToAvatarC2Ev
    // DEMANGLED: Dispatcher_AddSocketToAvatar::Dispatcher_AddSocketToAvatar(void)
    Dispatcher_AddSocketToAvatar();

    // IDA_EA: 0x8241776 / 0x82417A6
    // IDA_NAME: _ZN28Dispatcher_AddSocketToAvatarD2Ev / _ZN28Dispatcher_AddSocketToAvatarD0Ev
    // DEMANGLED: Dispatcher_AddSocketToAvatar::~Dispatcher_AddSocketToAvatar()
    virtual ~Dispatcher_AddSocketToAvatar();

    // IDA_EA: 0x821A412
    // IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AddSocketToAvatar::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // These virtuals are the retail zero-return generic routes.  CMD209 uses
    // dispatch_sig directly, so retaining them prevents a fabricated second
    // mutation path.
    // IDA_EA: 0x8233D4E
    // IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AddSocketToAvatar::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8233D58
    // IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AddSocketToAvatar::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AddSocketToAvatar) == 0x04u,
              "CMD209 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_ADD_SOCKET_TO_AVATAR_DISPATCHER_H_
