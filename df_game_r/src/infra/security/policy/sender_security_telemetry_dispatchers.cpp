#include "infra/security/policy/sender_security_telemetry_dispatchers.h"

#include <stdint.h>

#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/arena/pvp_room.h"

namespace {

const char kLogFile[] = "sender_security_telemetry_0828.cpp";
const char kAddHackTypeSignature[] =
    "virtual int Dispatcher_AddHackTypeCount0828::dispatch_sig(CUser*, PacketBuf&)";
const char kAbnormalFunctionSignature[] =
    "virtual int Dispatcher_AbnormalFunctionCall0828::dispatch_sig(CUser*, PacketBuf&)";
const char kReportClientHackSignature[] =
    "virtual int Dispatcher_ReportClientHack0828::dispatch_sig(CUser*, PacketBuf&)";
const char kReportPvpLagSignature[] =
    "virtual int Dispatcher_ReportPvPLag0828::dispatch_sig(CUser*, PacketBuf&)";
const char kVerifyPvpLagSignature[] =
    "virtual int Dispatcher_VerifyPvPLagUser0828::dispatch_sig(CUser*, PacketBuf&)";
const char kSecurityStatusSignature[] =
    "virtual int Dispatcher_SecurityStatus0828::dispatch_sig(CUser*, PacketBuf&)";

const int kPvpSeatCount = 8;
const int kClientHackEvidenceBodySize =
    static_cast<int>(sizeof(uint16_t) + sizeof(uint32_t) + 20u);

unsigned int CurrentCharacNo(CUser* user) {
    return user->getCurCharacR()
               ? static_cast<unsigned int>(user->getCurCharacNo())
               : 0u;
}

int RejectMalformed(const char* signature) {
    return LineFunc(__LINE__, signature, 0, 0);
}

CUser* FindPvpLagTarget(PvP_Room* room,
                        CUser* reporter,
                        uint16_t target_uid) {
    if (!room || !reporter) {
        return 0;
    }
    for (int seat = 0; seat < kPvpSeatCount; ++seat) {
        CUser* const candidate = room->get_user_seat(seat);
        if (!candidate || candidate == reporter || room->IsPvpObserver(seat)) {
            continue;
        }
        if (static_cast<uint16_t>(candidate->get_unique_id()) == target_uid) {
            return candidate;
        }
    }
    return 0;
}

void MakeEvidenceHex(const unsigned char* evidence, char* output) {
    static const char kHex[] = "0123456789ABCDEF";
    for (int index = 0; index < 20; ++index) {
        output[index * 2] = kHex[evidence[index] >> 4];
        output[index * 2 + 1] = kHex[evidence[index] & 0x0Fu];
    }
    output[40] = 0;
}

}  // namespace

Dispatcher_AddHackTypeCount::Dispatcher_AddHackTypeCount() {}
Dispatcher_AddHackTypeCount::~Dispatcher_AddHackTypeCount() {}

int Dispatcher_AddHackTypeCount::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    uint16_t hack_type = 0;
    unsigned char count = 0;
    if (packet.get_short(hack_type) != 1 || packet.get_byte(count) != 1) {
        return RejectMalformed(kAddHackTypeSignature);
    }

    LogManager::logFormat(
        1, kLogFile, kAddHackTypeSignature, 0,
        "[SECURITY0828][CMD253] account=%u charac=%u hack_type=%u count=%u state=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        CurrentCharacNo(user),
        static_cast<unsigned int>(hack_type),
        static_cast<unsigned int>(count), user->get_state());

    if (user->get_state() <= 1 || !user->getCurCharacR()) {
        return 0;
    }

    // The observed 0828 client sends raw types 49/50.  The old
    // CHackAnalyzer counter arrays are indexed by (type - 101), so routing
    // these values through addServerHackCnt would underflow the CUser object.
    // Hades and the existing dnf_hack_log writer both preserve raw u16 types.
    user->getHades()->AddHackTypeCnt(hack_type, count, 0, 0);
    reinterpret_cast<WongWork::CHackAnalyzer*>(
        static_cast<uintptr_t>(user->get_acc_id()))->ReqDBSaveHackInfo(
        CurrentCharacNo(user), hack_type, 0,
        static_cast<int>(count));
    return 0;
}

int Dispatcher_AddHackTypeCount::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_AddHackTypeCount::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_AbnormalFunctionCall::Dispatcher_AbnormalFunctionCall() {}
Dispatcher_AbnormalFunctionCall::~Dispatcher_AbnormalFunctionCall() {}

int Dispatcher_AbnormalFunctionCall::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    unsigned char abnormal_function_code = 0;
    if (packet.get_byte(abnormal_function_code) != 1) {
        return RejectMalformed(kAbnormalFunctionSignature);
    }
    LogManager::logFormat(
        1, kLogFile, kAbnormalFunctionSignature, 0,
        "[SECURITY0828][CMD393] account=%u charac=%u code=%u state=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        CurrentCharacNo(user),
        static_cast<unsigned int>(abnormal_function_code), user->get_state());
    return 0;
}

int Dispatcher_AbnormalFunctionCall::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_AbnormalFunctionCall::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_ReportClientHack::Dispatcher_ReportClientHack() {}
Dispatcher_ReportClientHack::~Dispatcher_ReportClientHack() {}

int Dispatcher_ReportClientHack::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    uint16_t report_kind = 0;
    if (packet.get_short(report_kind) != 1) {
        return RejectMalformed(kReportClientHackSignature);
    }

    // The scalar form has four semantic bytes remaining.  Its transport block
    // can add at most fifteen opaque bytes, while the evidence form requires
    // another 26 semantic bytes.  Use only that minimum-field boundary to
    // select the proven form; neither branch treats the remaining cipher bytes
    // as business data.
    if (packet.is_len_end(kClientHackEvidenceBodySize)) {
        uint32_t scalar_value = 0;
        if (packet.get_int(scalar_value) != 1) {
            return RejectMalformed(kReportClientHackSignature);
        }
        LogManager::logFormat(
            1, kLogFile, kReportClientHackSignature, 0,
            "[SECURITY0828][CMD471_SCALAR] account=%u charac=%u kind=%u value=%u state=%d",
            static_cast<unsigned int>(user->get_acc_id()),
            CurrentCharacNo(user),
            static_cast<unsigned int>(report_kind), scalar_value,
            user->get_state());
        return 0;
    }

    uint16_t rolling_key = 0;
    uint32_t evidence_size = 0;
    unsigned char evidence[20] = {};
    if (packet.get_short(rolling_key) != 1 ||
        packet.get_int(evidence_size) != 1 || evidence_size != 20u ||
        packet.get_binary(reinterpret_cast<char*>(evidence),
                          sizeof(evidence)) != 1) {
        return RejectMalformed(kReportClientHackSignature);
    }

    char evidence_hex[41];
    MakeEvidenceHex(evidence, evidence_hex);
    LogManager::logFormat(
        1, kLogFile, kReportClientHackSignature, 0,
        "[SECURITY0828][CMD471_EVIDENCE] account=%u charac=%u kind=%u key=%u size=%u evidence=%s state=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        CurrentCharacNo(user),
        static_cast<unsigned int>(report_kind),
        static_cast<unsigned int>(rolling_key), evidence_size, evidence_hex,
        user->get_state());
    return 0;
}

int Dispatcher_ReportClientHack::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_ReportClientHack::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_ReportPvPLag::Dispatcher_ReportPvPLag() {}
Dispatcher_ReportPvPLag::~Dispatcher_ReportPvPLag() {}

int Dispatcher_ReportPvPLag::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    uint16_t target_uid = 0;
    uint32_t average_lag = 0;
    if (packet.get_short(target_uid) != 1 ||
        packet.get_int(average_lag) != 1) {
        return RejectMalformed(kReportPvpLagSignature);
    }

    PvP_Room* const room = user->GetPVPRoom();
    const int reporter_seat = room ? room->get_user_seat(user) : -1;
    CUser* const target =
        reporter_seat >= 0 && reporter_seat < kPvpSeatCount &&
                !room->IsPvpObserver(reporter_seat)
            ? FindPvpLagTarget(room, user, target_uid)
            : 0;
    LogManager::logFormat(
        1, kLogFile, kReportPvpLagSignature, 0,
        "[SECURITY0828][CMD478] reporter_account=%u reporter_uid=%u target_uid=%u average_lag=%u room=%d target_found=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(
            static_cast<uint16_t>(user->get_unique_id())),
        static_cast<unsigned int>(target_uid), average_lag,
        room ? room->get_index() : -1, target ? 1 : 0);

    // The 0828 sender is reached from the periodic PVP tick and can report
    // every remote seat on each roughly ten-second cycle.  The original
    // server-side threshold/consensus decision that selected a target for
    // NOTI417 is not present in the 0627 source (its CMD481/482 slots are
    // unregistered).  Treat this packet as telemetry only: emitting NOTI417
    // for every sample would create a client prompt storm and invent policy.
    return 0;
}

int Dispatcher_ReportPvPLag::read(PacketBuf& packet,
                                      MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_ReportPvPLag::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_VerifyPvPLagUser::Dispatcher_VerifyPvPLagUser() {}
Dispatcher_VerifyPvPLagUser::~Dispatcher_VerifyPvPLagUser() {}

int Dispatcher_VerifyPvPLagUser::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    (void)packet;

    PvP_Room* const room = user->GetPVPRoom();
    LogManager::logFormat(
        1, kLogFile, kVerifyPvpLagSignature, 0,
        "[SECURITY0828][CMD479] target_account=%u target_uid=%u room=%d state=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(
            static_cast<uint16_t>(user->get_unique_id())),
        room ? room->get_index() : -1, user->get_state());

    // The client proves that NOTI417 causes this empty ACK, but the 0627
    // server has no registered CMD482 owner and the 0828 server's challenge
    // selection state is unavailable.  Preserve the exact packet as telemetry
    // without fabricating a successful correlation or a NOTI418 punishment.
    return 0;
}

int Dispatcher_VerifyPvPLagUser::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_VerifyPvPLagUser::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_SecurityStatus::Dispatcher_SecurityStatus() {}
Dispatcher_SecurityStatus::~Dispatcher_SecurityStatus() {}

int Dispatcher_SecurityStatus::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    uint16_t status_code = 0;
    if (packet.get_short(status_code) != 1) {
        return RejectMalformed(kSecurityStatusSignature);
    }

    // CLIENT_IDA_EVIDENCE: all eight 140828 senders are exactly
    // BeginCmd(587) -> put_short(status 0..7) -> Finalize.  On the live
    // crypto graph this two-byte semantic body is block-aligned to sixteen
    // bytes.  Cipher::Decrypt intentionally reports the aligned input length
    // unchanged, so the inbound PacketBuf contains fourteen transport-padding
    // bytes after status_code (wire size 29 = CS_Header 13 + cipher block 16).
    // Original dispatchers consume their proven fields and leave those
    // transport-owned bytes unread.
    LogManager::logFormat(
        1, kLogFile, kSecurityStatusSignature, 0,
        "[SECURITY0828][CMD587] account=%u charac=%u status=%u state=%d",
        static_cast<unsigned int>(user->get_acc_id()),
        CurrentCharacNo(user),
        static_cast<unsigned int>(status_code), user->get_state());
    return 0;
}

int Dispatcher_SecurityStatus::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SecurityStatus::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
