#include "old_game/dungeon/instance/packet_dispatcher_report_bad_p2p_user.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_ReportBadP2PUser::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8595AC0
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserC2Ev
// DEMANGLED: Dispatcher_ReportBadP2PUser::Dispatcher_ReportBadP2PUser(void)
Dispatcher_ReportBadP2PUser::Dispatcher_ReportBadP2PUser() {}

// IDA_EA: 0x8243774
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserD2Ev
// DEMANGLED: Dispatcher_ReportBadP2PUser::~Dispatcher_ReportBadP2PUser()
// IDA_EA: 0x82437A4
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUserD0Ev
// DEMANGLED: Dispatcher_ReportBadP2PUser::~Dispatcher_ReportBadP2PUser()
Dispatcher_ReportBadP2PUser::~Dispatcher_ReportBadP2PUser() {}

// IDA_EA: 0x81FD0A8
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ReportBadP2PUser::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ReportBadP2PUser::dispatch_sig(CUser* user, PacketBuf& packet) {
    char reported_seat = 0;
    if (packet.get_byte(reported_seat) != 1) {
        return LineFunc(36315, kDispatchSignature, 0, 0);
    }

    cMyTraceNoop trace;
    trace("user %s Lack votes slot %d as Lack",
          user->getCurCharacName(),
          static_cast<int>(reported_seat));

    if (static_cast<unsigned char>(user->CheckInPvp()) != 0u) {
        PvP_Room* const room = user->GetPVPRoom();
        if (room) {
            room->VoteLackUser(user, static_cast<int>(reported_seat));
        } else {
            cMyTrace failure_trace(kDispatchSignature, 36328, 5);
            failure_trace("[%s][%d]_!pPVPRoom", kDispatchSignature, 36328);
        }
    } else if (user->IsState(static_cast<ch_state>(5))) {
        CParty* const party = user->GetParty();
        if (party) {
            party->VoteLackUser(user, static_cast<int>(reported_seat));
        } else {
            cMyTrace failure_trace(kDispatchSignature, 36344, 5);
            failure_trace("[%s][%d]_!pParty", kDispatchSignature, 36344);
        }
    }
    return 0;
}

// IDA_EA: 0x823347C
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ReportBadP2PUser::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ReportBadP2PUser::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233486
// IDA_NAME: _ZN27Dispatcher_ReportBadP2PUser7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ReportBadP2PUser::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ReportBadP2PUser::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}
