#include "infra/security/policy/antibot_dproto_dispatcher.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"
#include "infra/security/dynamic_abi/packet_dispatcher_do_dispatch_abi.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_Antibot_DP::dispatch_sig(CUser*, PacketBuf&)";
const uint32_t kMaxEncryptedPayloadLength = 0x567u;
const uint16_t kDecodedPacketCapacity = 2800u;
const ENUM_CMDPACKET kDprotoCommand = ENUM_CMDPACKET_DPROTO;

// The generic packet-dispatch template treats every positive handler result
// as a dispatch failure. This value is used only for the non-original
// adapter-unavailable boundary; no checker result is folded into it.
const int kAntibotAdapterUnavailableFailure = 1;

}  // namespace

// IDA_EA: 0x85965CC
// IDA_NAME: _ZN21Dispatcher_Antibot_DPC2Ev
// DEMANGLED: Dispatcher_Antibot_DP::Dispatcher_Antibot_DP(void)
Dispatcher_Antibot_DP::Dispatcher_Antibot_DP() {}

// IDA_EA: 0x8241FB0
// IDA_NAME: _ZN21Dispatcher_Antibot_DPD2Ev
// DEMANGLED: Dispatcher_Antibot_DP::~Dispatcher_Antibot_DP()
// IDA_EA: 0x8241FE0
// IDA_NAME: _ZN21Dispatcher_Antibot_DPD0Ev
// DEMANGLED: Dispatcher_Antibot_DP::~Dispatcher_Antibot_DP()
Dispatcher_Antibot_DP::~Dispatcher_Antibot_DP() {}

// IDA_EA: 0x820B322
// IDA_NAME: OldSrv_Dispatcher_Antibot_DP_dispatch_sig_CMD_DPROTO
// DEMANGLED: OldSrv_Dispatcher_Antibot_DP_dispatch_sig_CMD_DPROTO
// IDA_SYNTHETIC_MEMBER_LABEL: see the header and exact inventory.
int Dispatcher_Antibot_DP::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint32_t encrypted_packet_length = 0;
    if (packet.get_int(encrypted_packet_length) != 1) {
        return LineFunc(42901, kDispatchSignature, 0, 0);
    }

    const int index = packet.get_index();
    unsigned char* const encrypted_packet =
        reinterpret_cast<unsigned char*>(packet.get_buf_ptr(index));
    if (encrypted_packet_length > kMaxEncryptedPayloadLength) {
        return LineFunc(42915, kDispatchSignature, 0, 0);
    }

    unsigned char decoded_packet[kDecodedPacketCapacity];
    uint16_t decoded_packet_length = kDecodedPacketCapacity;
    int plugin_result = 0;
    const bool checker_called = TryAntibotCheckerRecvBufferForDecrypt(
            GlobalData::antibot_checker(),
            user->getAntibotKey(),
            encrypted_packet,
            static_cast<uint16_t>(encrypted_packet_length),
            decoded_packet,
            &decoded_packet_length,
            &plugin_result);
    if (!checker_called) {
        // The old binary always had an initialized checker here. A rebuild
        // cannot safely reinterpret an absent foreign plug-in as a real -1
        // result, so reject the handler with the documented positive error.
        return kAntibotAdapterUnavailableFailure;
    }

    if (plugin_result != 0) {
        cMyTrace trace(kDispatchSignature, 42927, 0);
        trace("[Dispatcher_Antibot_DP] Key : %d, nestedPacket : %x, binarySize : %d, return : %d",
              user->getAntibotKey(),
              static_cast<unsigned int>(
                  reinterpret_cast<uintptr_t>(encrypted_packet)),
              static_cast<int>(encrypted_packet_length),
              plugin_result);
        return 0;
    }

    // Raw 0x820B322 overlays the decoded bytes as the complete inner C2S
    // header and re-enters doDispatch with source CMD_DPROTO. Do not add
    // an output-length or PacketDispatcher-null guard here: neither exists in
    // the old handler and doDispatch owns the inner header/CRC validation.
    const CS_Header* const nested_header =
        reinterpret_cast<const CS_Header*>(decoded_packet);
    (G_PacketDispatcher())->doDispatch(
        user,
        static_cast<ENUM_PACKETCLASS>(nested_header->cmd),
        nested_header->type,
        reinterpret_cast<char*>(decoded_packet),
        static_cast<int>(decoded_packet_length),
        ENUM_PACKETCLASS_CMDPACKET,
        kDprotoCommand);
    return 0;
}

// IDA_EA: 0x8233B5A
// IDA_NAME: _ZN21Dispatcher_Antibot_DP4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Antibot_DP::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Antibot_DP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B64
// IDA_NAME: _ZN21Dispatcher_Antibot_DP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Antibot_DP::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Antibot_DP::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
