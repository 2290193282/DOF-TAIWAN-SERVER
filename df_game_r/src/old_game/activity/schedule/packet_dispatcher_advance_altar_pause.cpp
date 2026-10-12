#include "old_game/activity/schedule/packet_dispatcher_advance_altar_pause.h"

#include <cstring>

#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace CMDPacketStruct {

// IDA_EA: 0x8141EC6
// IDA_NAME: _ZN15CMDPacketStruct21_ReqAdvanceAltarPauseC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarPause::_ReqAdvanceAltarPause(void)
_ReqAdvanceAltarPause::_ReqAdvanceAltarPause() : STBaseRequest() {
    std::memset(this, 0, 0x0eu);
}

// IDA_EA: 0x8141EF4
// IDA_NAME: _ZN15CMDPacketStruct21_ResAdvanceAltarPauseC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarPause::_ResAdvanceAltarPause(void)
_ResAdvanceAltarPause::_ResAdvanceAltarPause() : STBaseResponse() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x81426A2 / 0x81426D2
// IDA_NAME: _ZN15CMDPacketStruct21_ResAdvanceAltarPauseD2Ev /
//           _ZN15CMDPacketStruct21_ResAdvanceAltarPauseD0Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarPause::~_ResAdvanceAltarPause()
_ResAdvanceAltarPause::~_ResAdvanceAltarPause() {}

}  // namespace CMDPacketStruct

namespace advancealtar {

// IDA_EA: 0x8140A1C
// IDA_NAME: _ZN12advancealtar16Dispatcher_Pause12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_Pause::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Pause::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 1324;
    }

    CMDPacketStruct::_ReqAdvanceAltarPause request;
    CMDPacketStruct::_ResAdvanceAltarPause response;
    read_packet(packet, request);
    process_packet(user, request, response);
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x8140B04
// IDA_NAME: _ZNK12advancealtar16Dispatcher_Pause11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_Pause::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_Pause::read_packet(
    PacketBuf& packet, CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 0x0e);
    return 0;
}

// IDA_EA: 0x8140B32
// IDA_NAME: _ZNK12advancealtar16Dispatcher_Pause11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_Pause::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_Pause::check_error(
    CUser* user, CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return 0;
}

// IDA_EA: 0x8140B3C
// IDA_NAME: _ZNK12advancealtar16Dispatcher_Pause14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_Pause::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_Pause::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request,
    CMDPacketStruct::STBaseResponse& response) const {
    CMDPacketStruct::_ReqAdvanceAltarPause& req =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarPause&>(request);
    CMDPacketStruct::_ResAdvanceAltarPause& res =
        static_cast<CMDPacketStruct::_ResAdvanceAltarPause&>(response);
    res.result_04 = check_error(user, request);

    StageControl* const altar = user ? user->getAdvanceAltar() : 0;
    ProcStage* const stage = altar ? altar->getProcStage() : 0;
    if (!stage) {
        if (!res.result_04) {
            res.result_04 = 1;
        }
        res.paused_08 = 0;
        return 0;
    }

    if (!res.result_04 && !Manager::pause(user, req.paused_0d != 0)) {
        // The client response handler ignores result_04 and applies paused_08
        // whenever the command envelope is successful.  Preserve that
        // compatible envelope, but mark the business failure and return the
        // authoritative state so client and server still converge.
        res.result_04 = 1;
    }
    res.paused_08 = stage->isPaused() ? 1 : 0;
    return 0;
}

// IDA_EA: 0x8140BAE
// IDA_NAME: _ZN12advancealtar16Dispatcher_Pause11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_Pause::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_Pause::send_packet(CUser* user,
                                   CMDPacketStruct::STBaseResponse& response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_PAUSE);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&response), 0x0c);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81423C2
// IDA_NAME: _ZN12advancealtar16Dispatcher_PauseD2Ev
// DEMANGLED: advancealtar::Dispatcher_Pause::~Dispatcher_Pause()
// IDA_EA: 0x81423F2
// IDA_NAME: _ZN12advancealtar16Dispatcher_PauseD0Ev
// DEMANGLED: advancealtar::Dispatcher_Pause::~Dispatcher_Pause()
Dispatcher_Pause::~Dispatcher_Pause() {}

// IDA_EA: 0x8142410
// IDA_NAME: _ZN12advancealtar16Dispatcher_Pause4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_Pause::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Pause::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x814241A
// IDA_NAME: _ZN12advancealtar16Dispatcher_Pause7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_Pause::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Pause::process(CUser* user, MSG_BASE& message, ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595778
// IDA_NAME: _ZN12advancealtar16Dispatcher_PauseC2Ev
// DEMANGLED: advancealtar::Dispatcher_Pause::Dispatcher_Pause(void)
Dispatcher_Pause::Dispatcher_Pause()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar
