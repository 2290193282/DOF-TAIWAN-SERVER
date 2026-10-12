#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_AVATAR_OPTION_CHANGE_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_AVATAR_OPTION_CHANGE_DISPATCHER_H_

#include <stdint.h>

#include "old_game/player/core/userinfo_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: stChangeAvatarOption.  The C2 only initializes ParamBase and
// installs this type's vtable: process() assigns result before any read, and
// the three success fields are intentionally untouched on error.
struct stChangeAvatarOption : ParamBase {
    // IDA_EA: 0x824BC32
    // IDA_NAME: _ZN20stChangeAvatarOptionC2Ev
    // DEMANGLED: stChangeAvatarOption::stChangeAvatarOption(void)
    stChangeAvatarOption();

    // IDA_EA: 0x824BC4E / 0x824BC7E
    // IDA_NAME: _ZN20stChangeAvatarOptionD2Ev / _ZN20stChangeAvatarOptionD0Ev
    // DEMANGLED: stChangeAvatarOption::~stChangeAvatarOption()
    virtual ~stChangeAvatarOption();

    int result;                // IDA +0x04
    uint16_t material_slot;    // IDA +0x08
    uint16_t avatar_slot;      // IDA +0x0a
    uint8_t option;            // IDA +0x0c
};

#if defined(__i386__)
static_assert(sizeof(stChangeAvatarOption) == 0x10u,
              "stChangeAvatarOption must retain its i386 ABI size");
#endif

// 0627 CMD460 / 0828 CMD457.  This is a recovered derived provider; 0828
// registers it directly through ENUM_CMDPACKET_AVATAR_OPTION_CHANGE.
class Dispatcher_AvatarOptionChange
    : public IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,
                               stChangeAvatarOption,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859591C
    Dispatcher_AvatarOptionChange();

    // IDA_EA: 0x8244308 / 0x8244338
    virtual ~Dispatcher_AvatarOptionChange();

    // IDA_EA: 0x81E04F2 / 0x81E0774 / 0x81E0E32
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    // IDA_EA: 0x81E0A40
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AvatarOptionChange) == 0x04u,
              "avatar-option dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_AVATAR_OPTION_CHANGE_DISPATCHER_H_
