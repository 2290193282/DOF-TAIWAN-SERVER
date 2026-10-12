#include "old_game/player/core/packet_dispatcher_request_pc_room_player_count.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x8596A2C
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountC2Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::Dispatcher_RequestPCRoomPlayerCount(void)
Dispatcher_RequestPCRoomPlayerCount::Dispatcher_RequestPCRoomPlayerCount() {}

// IDA_EA: 0x82414B8
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountD2Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::~Dispatcher_RequestPCRoomPlayerCount()
// IDA_EA: 0x82414E8
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountD0Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::~Dispatcher_RequestPCRoomPlayerCount()
Dispatcher_RequestPCRoomPlayerCount::~Dispatcher_RequestPCRoomPlayerCount() {}

// IDA_EA: 0x821D8D0
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestPCRoomPlayerCount::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;

    if (user->get_state() > 2 && user->getCurCharacR()) {
        return 0;
    }

    const char* const account_id_text =
        NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
    const int state = user->get_state();
    cMyTrace trace(
        "virtual int Dispatcher_RequestPCRoomPlayerCount::dispatch_sig(CUser*, PacketBuf&)",
        53638,
        5);
    trace("PCROOM : Dispatcher_RequestPCRoomPlayerCount => User State : %d, m_id : %s",
          state,
          account_id_text);
    return LineFunc(
        53639,
        "virtual int Dispatcher_RequestPCRoomPlayerCount::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233E02
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestPCRoomPlayerCount::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E0C
// IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestPCRoomPlayerCount::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
