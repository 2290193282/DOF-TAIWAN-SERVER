#include "old_game/player/core/packet_dispatcher_request_pc_room_player_list.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x8596A10
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListC2Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::Dispatcher_RequestPCRoomPlayerList(void)
Dispatcher_RequestPCRoomPlayerList::Dispatcher_RequestPCRoomPlayerList() {}

// IDA_EA: 0x8241506
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListD2Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::~Dispatcher_RequestPCRoomPlayerList()
// IDA_EA: 0x8241536
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListD0Ev
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::~Dispatcher_RequestPCRoomPlayerList()
Dispatcher_RequestPCRoomPlayerList::~Dispatcher_RequestPCRoomPlayerList() {}

// IDA_EA: 0x821D7FA
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestPCRoomPlayerList::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    (void)packet;

    if (user->get_state() > 2 && user->getCurCharacR()) {
        return 0;
    }

    const char* const account_id_text =
        NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
    const int state = user->get_state();
    cMyTrace trace(
        "virtual int Dispatcher_RequestPCRoomPlayerList::dispatch_sig(CUser*, PacketBuf&)",
        53582,
        5);
    trace("PCROOM : Dispatcher_RequestPCRoomPlayerList => User State : %d, m_id : %s",
          state,
          account_id_text);
    return LineFunc(
        53583,
        "virtual int Dispatcher_RequestPCRoomPlayerList::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233DEE
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestPCRoomPlayerList::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233DF8
// IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestPCRoomPlayerList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestPCRoomPlayerList::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
