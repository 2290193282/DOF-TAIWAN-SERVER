#include "infra/security/policy/bv_hack_info_dispatcher.h"

#include <cstring>

#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const uint32_t kBVPacketMagic = 1849641884u;
const char kDispatchSignature[] =
    "virtual int Dispatcher_BVHackInfo::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x88920F1
// IDA_NAME: _Z19decryptBVHackPacketPcR14stBVHackInfo_t
// DEMANGLED: decryptBVHackPacket(char *,stBVHackInfo_t &)
int decryptBVHackPacket(char* packet, stBVHackInfo_t& output) {
    if (!packet) {
        return 0;
    }

    uint32_t magic = 0;
    std::memcpy(&magic, packet, sizeof(magic));
    if (magic != kBVPacketMagic) {
        return 0;
    }

    uint32_t words[13];
    std::memcpy(words, packet + sizeof(magic), sizeof(words));
    const uint32_t xor_key = words[12];
    // The thirteenth word is the XOR key; IDA mutates only words 0 through 11.
    for (unsigned int index = 0; index < 12u; ++index) {
        words[index] ^= xor_key;
    }
    std::memcpy(packet + sizeof(magic), words, sizeof(words));
    std::memcpy(&output, words, sizeof(output));
    return 1;
}

// IDA_EA: 0x8596310
// IDA_NAME: _ZN21Dispatcher_BVHackInfoC2Ev
// DEMANGLED: Dispatcher_BVHackInfo::Dispatcher_BVHackInfo(void)
Dispatcher_BVHackInfo::Dispatcher_BVHackInfo() {}

// IDA_EA: 0x8242616
// IDA_NAME: _ZN21Dispatcher_BVHackInfoD2Ev
// DEMANGLED: Dispatcher_BVHackInfo::~Dispatcher_BVHackInfo()
// IDA_EA: 0x8242646
// IDA_NAME: _ZN21Dispatcher_BVHackInfoD0Ev
// DEMANGLED: Dispatcher_BVHackInfo::~Dispatcher_BVHackInfo()
Dispatcher_BVHackInfo::~Dispatcher_BVHackInfo() {}

// IDA_EA: 0x8207568
// IDA_NAME: _ZN21Dispatcher_BVHackInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BVHackInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BVHackInfo::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(41494, kDispatchSignature, 0, 0);
    }

    uint32_t report_size = 0;
    if (packet.get_int(report_size) != 1 || report_size != 64u) {
        return LineFunc(41498, kDispatchSignature, 0, 0);
    }

    char encrypted[64];
    if (packet.get_binary(encrypted, sizeof(encrypted)) != 1) {
        return LineFunc(41500, kDispatchSignature, 0, 0);
    }

    stBVHackInfo_t report;
    if (decryptBVHackPacket(encrypted, report) != 1) {
        return LineFunc(41508, kDispatchSignature, 0, 0);
    }

    if (report.values[0] != 0.0 || report.values[1] != 0.0 ||
        report.values[2] != 0.0 || report.values[3] != 0.0 ||
        report.values[4] != 0.0) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            kDispatchSignature,
            0xA228,
            "(m_id: %s)(%5f)(%5f)(%5f)(%5f)(%5f)(%5f)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            report.values[0],
            report.values[1],
            report.values[2],
            report.values[3],
            report.values[4],
            report.values[5]);
    }

    if (report.values[1] >= 50.0 &&
        (report.values[2] != 0.0 || report.values[4] != 0.0)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x328u),
            1u,
            0u,
            0u);
        if (user->get_state() != 5) {
            user->SetTradePunishType(16);
        }
    }
    return 0;
}

// IDA_EA: 0x8233966
// IDA_NAME: _ZN21Dispatcher_BVHackInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BVHackInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BVHackInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233970
// IDA_NAME: _ZN21Dispatcher_BVHackInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BVHackInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BVHackInfo::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
