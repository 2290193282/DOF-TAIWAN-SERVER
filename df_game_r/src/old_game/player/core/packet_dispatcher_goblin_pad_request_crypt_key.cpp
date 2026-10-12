#include "old_game/player/core/packet_dispatcher_goblin_pad_request_crypt_key.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8596AD4
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyC2Ev
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::Dispatcher_GoblinPadRequestCryptKey(void)
Dispatcher_GoblinPadRequestCryptKey::Dispatcher_GoblinPadRequestCryptKey() {}

// IDA_EA: 0x8241296
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyD2Ev
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::~Dispatcher_GoblinPadRequestCryptKey()
// IDA_EA: 0x82412C6
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyD0Ev
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::~Dispatcher_GoblinPadRequestCryptKey()
Dispatcher_GoblinPadRequestCryptKey::~Dispatcher_GoblinPadRequestCryptKey() {}

// IDA_EA: 0x821E58C
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GoblinPadRequestCryptKey::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;

    user->getPad()->GenerateRandomData();

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_GOBLIN_PAD_RESPONSE_CRYPT_KEY);
    response.put_byte(1);
    response.put_short(user->getPad()->GetKeyIdx(0));
    response.put_short(user->getPad()->GetKeyIdx(1));
    response.put_byte(user->getPad()->GetShakeData(0));
    response.put_byte(user->getPad()->GetShakeData(1));
    response.put_byte(user->getPad()->GetShakeData(2));
    response.put_byte(user->getPad()->GetShakeData(3));
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233E7A
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GoblinPadRequestCryptKey::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E84
// IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GoblinPadRequestCryptKey::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
