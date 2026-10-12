#include "old_game/pvp/battlefield/dispatcher_request_pvp_exp_of_week.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8595B84
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekC2Ev
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::Dispatcher_Request_PvpExpOfWeek(void)
Dispatcher_Request_PvpExpOfWeek::Dispatcher_Request_PvpExpOfWeek() {}

// IDA_EA: 0x82435A0
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekD2Ev
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::~Dispatcher_Request_PvpExpOfWeek()
// IDA_EA: 0x82435D0
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeekD0Ev
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::~Dispatcher_Request_PvpExpOfWeek()
Dispatcher_Request_PvpExpOfWeek::~Dispatcher_Request_PvpExpOfWeek() {}

// IDA_EA: 0x81FECAA
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Request_PvpExpOfWeek::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    (void)packet;

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                        ENUM_NOTIPACKET_PVPEXP_OF_WEEK);
    response.put_int(user->get_pvp_exp_of_week());
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82334F4
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Request_PvpExpOfWeek::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82334FE
// IDA_NAME: _ZN31Dispatcher_Request_PvpExpOfWeek7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Request_PvpExpOfWeek::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Request_PvpExpOfWeek::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
