#include "old_data/Login/security/inter_report_mannerless_return_dispatcher.h"

#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/mannerless_user_accusation_packet.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const int kReportMannerlessLogQueueSignal = 202;
const int kNoBoundUserIndex = -1;
const ENUM_CMDPACKET kReportMannerlessPacketCommand =
    ENUM_CMDPACKET_REPORT_MANNERLESS_USER;
const unsigned char kReportedByGoldHeadEquipment = 111;
const unsigned char kReportedByUnknownReason = 99;

}  // namespace

// IDA_EA: 0x84D24A2
// IDA_NAME: _ZN22Inter_ReportMannerless12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReportMannerless::dispatch_sig(CUser *,char *,int)
int Inter_ReportMannerless::dispatch_sig(CUser* user, char* payload, int) {
    if (!payload) {
        return 0;
    }

    // The DB record identifies its reporter at +0x0e and +0x1e.  When this
    // is a monitor return for a different currently connected character, the
    // original routes the reply by account id rather than the dispatcher user.
    CUser* reply_user = user;
    const int record_charac_no = *reinterpret_cast<const int*>(payload + 14);
    if (user && record_charac_no != user->get_charac_no(-1)) {
        const unsigned int record_account_id =
            *reinterpret_cast<const unsigned int*>(payload + 30);
        reply_user = G_CGameManager()->GetUserByAccId(record_account_id);
    }

    const unsigned char status = static_cast<unsigned char>(payload[350]);
    if (status == kReportedByGoldHeadEquipment) {
        if (reply_user) {
            reply_user->SendCmdErrorPacket(
                kReportMannerlessPacketCommand,
                98);
        }
        return 0;
    }
    if (status == kReportedByUnknownReason) {
        if (reply_user) {
            reply_user->SendCmdErrorPacket(
                kReportMannerlessPacketCommand,
                95);
        }
        return 0;
    }

    // Keep the old log-DB queue framing verbatim: outer signal 202, user -1,
    // then the complete packed 0x15f Monitor record.
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "InterDispatcher.cpp", 12909);
    CStreamGuard guard(stream, true);
    guard << kReportMannerlessLogQueueSignal;
    guard << kNoBoundUserIndex;
    guard.put_binary(payload, sizeof(Packet_Mannerless_User_Accusation));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);

    if (reply_user) {
        PacketGuard response;
        response.put_header(1, kReportMannerlessPacketCommand);
        response.put_byte(1);
        response.finalize(true);
        reply_user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x84ED8C0 / 0x84F956A / 0x84F959A
// IDA_NAME: _ZN22Inter_ReportMannerlessC2Ev / D2Ev / D0Ev
Inter_ReportMannerless::Inter_ReportMannerless() : ISigDispatcher() {}
Inter_ReportMannerless::~Inter_ReportMannerless() {}
