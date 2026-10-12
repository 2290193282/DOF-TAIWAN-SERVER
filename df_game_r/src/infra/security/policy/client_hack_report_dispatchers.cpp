#include "infra/security/policy/client_hack_report_dispatchers.h"

#include <cstring>

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/security/policy/codehack_checksum.h"

namespace {

const char kReportClientHackDispatchSignature[] =
    "virtual int DisPatcher_ReportClient4Hack::dispatch_sig(CUser*, PacketBuf&)";
const char kCodeChecksumDispatchSignature[] =
    "virtual int Dispatcher_CodeCheckSum::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8595F90
// IDA_NAME: _ZN28DisPatcher_ReportClient4HackC2Ev
// DEMANGLED: DisPatcher_ReportClient4Hack::DisPatcher_ReportClient4Hack(void)
DisPatcher_ReportClient4Hack::DisPatcher_ReportClient4Hack() {}

// IDA_EA: 0x8242D18
// IDA_NAME: _ZN28DisPatcher_ReportClient4HackD2Ev
// DEMANGLED: DisPatcher_ReportClient4Hack::~DisPatcher_ReportClient4Hack()
// IDA_EA: 0x8242D48
// IDA_NAME: _ZN28DisPatcher_ReportClient4HackD0Ev
// DEMANGLED: DisPatcher_ReportClient4Hack::~DisPatcher_ReportClient4Hack()
DisPatcher_ReportClient4Hack::~DisPatcher_ReportClient4Hack() {}

// IDA_EA: 0x82041A0
// IDA_NAME: _ZN28DisPatcher_ReportClient4Hack12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_ReportClient4Hack::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_ReportClient4Hack::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1 || !user->getCurCharacR()) {
        return 0;
    }

    const int state = user->get_state();
    if (state != 6) {
        if (state > 6) {
            if (state == 7) {
                if (!G_CGameManager()->GetPvp(user->GetPvpIndex(), user, 0)) {
                    return LineFunc(39873,
                                    kReportClientHackDispatchSignature,
                                    0,
                                    0);
                }
            } else {
                if (state != 8) {
                    user->getHackAnalyzer()->beginCollectHackInfo(user);
                    user->getHackAnalyzer()->analyzeHackReport(user, &packet);
                    user->getHackAnalyzer()->reportHackInfo();
                    user->getHackAnalyzer()->resetHackInfo();
                    return 0;
                }
                if (!user->GetWarRoom()) {
                    return LineFunc(39877,
                                    kReportClientHackDispatchSignature,
                                    0,
                                    0);
                }
            }
        } else {
            if (state != 5) {
                user->getHackAnalyzer()->beginCollectHackInfo(user);
                user->getHackAnalyzer()->analyzeHackReport(user, &packet);
                user->getHackAnalyzer()->reportHackInfo();
                user->getHackAnalyzer()->resetHackInfo();
                return 0;
            }
            if (!user->GetParty()) {
                return LineFunc(39863,
                                kReportClientHackDispatchSignature,
                                0,
                                0);
            }
        }
    }

    user->getHackAnalyzer()->analyzeHackReport(user, &packet);
    return 0;
}

// IDA_EA: 0x8233738
// IDA_NAME: _ZN28DisPatcher_ReportClient4Hack4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_ReportClient4Hack::read(PacketBuf &,MSG_BASE &)
int DisPatcher_ReportClient4Hack::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233742
// IDA_NAME: _ZN28DisPatcher_ReportClient4Hack7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_ReportClient4Hack::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_ReportClient4Hack::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595FE4
// IDA_NAME: _ZN23Dispatcher_CodeCheckSumC2Ev
// DEMANGLED: Dispatcher_CodeCheckSum::Dispatcher_CodeCheckSum(void)
Dispatcher_CodeCheckSum::Dispatcher_CodeCheckSum() {}

// IDA_EA: 0x8242C2E
// IDA_NAME: _ZN23Dispatcher_CodeCheckSumD2Ev
// DEMANGLED: Dispatcher_CodeCheckSum::~Dispatcher_CodeCheckSum()
// IDA_EA: 0x8242C5E
// IDA_NAME: _ZN23Dispatcher_CodeCheckSumD0Ev
// DEMANGLED: Dispatcher_CodeCheckSum::~Dispatcher_CodeCheckSum()
Dispatcher_CodeCheckSum::~Dispatcher_CodeCheckSum() {}

// IDA_EA: 0x82044B0
// IDA_NAME: _ZN23Dispatcher_CodeCheckSum12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CodeCheckSum::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CodeCheckSum::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return 0;
    }

    int ignored = 0;
    if (packet.get_int(ignored) != 1) {
        return LineFunc(39933, kCodeChecksumDispatchSignature, 0, 0);
    }

    static_assert(sizeof(CodeHackChecksum) == 0x20u,
                  "CMD114 checksum payload must retain the raw 32-byte ABI");
    CodeHackChecksum checksum;
    if (packet.get_binary(reinterpret_cast<char*>(checksum.bytes),
                          sizeof(checksum.bytes)) != 1) {
        return LineFunc(39935, kCodeChecksumDispatchSignature, 0, 0);
    }

    char haystack[260];
    std::memset(haystack, 0, sizeof(haystack));
    const int checksum_result = user->checkCodeChecksum(checksum, haystack);
    user->IncChecksumAnswerCount();
    if (checksum_result == 1) {
        return 0;
    }

    int unused_reason = 2;
    if (!std::strstr(haystack, ".exe.bak")) {
        unused_reason = 3;
    }
    (void)unused_reason;

    if (user->get_state() > 1) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x3C8u),
            1u,
            0u,
            0u);
    }
    return 0;
}

// IDA_EA: 0x8233774
// IDA_NAME: _ZN23Dispatcher_CodeCheckSum4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CodeCheckSum::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CodeCheckSum::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823377E
// IDA_NAME: _ZN23Dispatcher_CodeCheckSum7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CodeCheckSum::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CodeCheckSum::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
