#include "infra/security/policy/antibot_noop_dispatchers.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"

namespace {

const uint32_t kMaxDprotoPayload = 0x567u;
const uint16_t kDecodedPacketCapacity = 2800u;
const int kAntibotAdapterUnavailableFailure = 1;

const char kAntibotDispatchSignature[] =
    "virtual int Dispatcher_Antibot::dispatch_sig(CUser*, PacketBuf&)";
const char kDprotoCallbackDispatchSignature[] =
    "virtual int Dispatcher_Antibot_DPCallBack::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x85965E8
// IDA_NAME: _ZN18Dispatcher_AntibotC2Ev
// DEMANGLED: Dispatcher_Antibot::Dispatcher_Antibot(void)
Dispatcher_Antibot::Dispatcher_Antibot() {}

// IDA_EA: 0x8241F62
// IDA_NAME: _ZN18Dispatcher_AntibotD2Ev
// DEMANGLED: Dispatcher_Antibot::~Dispatcher_Antibot()
// IDA_EA: 0x8241F92
// IDA_NAME: _ZN18Dispatcher_AntibotD0Ev
// DEMANGLED: Dispatcher_Antibot::~Dispatcher_Antibot()
Dispatcher_Antibot::~Dispatcher_Antibot() {}

// IDA_EA: 0x820B50C
// IDA_NAME: OldSrv_Dispatcher_Antibot_dispatch_sig_CMD_ANTIBOT
// DEMANGLED: OldSrv_Dispatcher_Antibot_dispatch_sig_CMD_ANTIBOT
int Dispatcher_Antibot::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user) {
        return 0;
    }

    uint32_t client_buffer_size = 0u;
    if (packet.get_int(client_buffer_size) != 1) {
        return LineFunc(42945, kAntibotDispatchSignature, 0, 0);
    }

    // IDA_EA: 0x820B557..0x820B571.  This is an inbound PacketBuf, which is
    // deliberately not finalized: the original reads the payload cursor and
    // obtains its backing pointer directly.  Calling get_len() here is an
    // ABI error because get_len() is reserved for finalized outbound packets.
    const int payload_index = packet.get_index();
    uint8_t* const client_buffer = reinterpret_cast<uint8_t*>(
        packet.get_buf_ptr(payload_index));

    if (user->antibot_packet_state_ != 0u) {
        if (user->antibot_packet_state_ != 4u) {
            user->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(4), 603,
                static_cast<int>(client_buffer_size),
                static_cast<int>(user->antibot_packet_state_));
        }
    } else {
        user->history_log().AntibotPacketLog(
            static_cast<AntibotPacketLogType::T>(0));
        user->antibot_packet_state_ = 1u;
    }

    // IDA_EA: 0x820B5ED branches directly to the checker call when the
    // PacketBuf backing pointer is null.  The serial read/update is therefore
    // conditional on the original pointer, rather than an unconditional
    // client_buffer + 2 dereference.
    if (client_buffer) {
        const int previous_serial_number = user->getCharacAntibotSerialNum();
        uint32_t serial_number_network = 0u;
        // The old `buf_ptr == -2` sentinel reaches this block but leaves the
        // serial image zeroed: 0x820B643 adds two, then 0x820B64D skips memcpy
        // when that resulting source is null.
        if (reinterpret_cast<intptr_t>(client_buffer) != -2) {
            memcpy(&serial_number_network,
                   client_buffer + 2,
                   sizeof(serial_number_network));
        }
        const int serial_number =
            static_cast<int>(ntohl(serial_number_network));
        if (previous_serial_number != 0 &&
            previous_serial_number + 1 != serial_number) {
            // IDA_EA: 0x820B69A..0x820B702.
            cMyTrace trace(kAntibotDispatchSignature, 42985, 0);
            trace("Antibot Packet Serial Number host_byte_order:%u, "
                  "network_byte_order:%u, m_id:%u, charac_no:%d, "
                  "pre_serial_number:%u",
                  static_cast<unsigned int>(serial_number),
                  serial_number_network,
                  static_cast<unsigned int>(user->get_acc_id()),
                  user->getCurCharacNo(),
                  static_cast<unsigned int>(previous_serial_number));
        }
        user->setCharacAntibotSerialNum(serial_number);
    }

    int plugin_result = 0;
    CEnvironment* const environment = G_CEnvironment();
    if (!environment ||
        !TryAntibotCheckerProcessClientPacket(
            GlobalData::antibot_checker(), environment->getProcessSequence(),
            0, static_cast<uint32_t>(user->get_acc_id()), client_buffer,
            static_cast<uint16_t>(client_buffer_size), &plugin_result)) {
        // The old service starts only after libantisvrimport is available.
        // Treat a missing foreign boundary as a rejected anti-bot request,
        // never as a successful verification response.
        return kAntibotAdapterUnavailableFailure;
    }

    if (user->antibot_packet_state_ == 1u) {
        user->history_log().AntibotPacketLog(
            static_cast<AntibotPacketLogType::T>(1));
        user->antibot_packet_state_ = 2u;
    } else if (user->antibot_packet_state_ != 4u) {
        user->history_log().AbnormalUser(
            static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(4), 603,
            static_cast<int>(client_buffer_size),
            static_cast<int>(user->antibot_packet_state_));
    }
    return 0;
}

// IDA_EA: 0x8233B6E
// IDA_NAME: _ZN18Dispatcher_Antibot4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Antibot::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Antibot::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B78
// IDA_NAME: _ZN18Dispatcher_Antibot7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Antibot::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Antibot::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85965B0
// IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackC2Ev
// DEMANGLED: Dispatcher_Antibot_DPCallBack::Dispatcher_Antibot_DPCallBack(void)
Dispatcher_Antibot_DPCallBack::Dispatcher_Antibot_DPCallBack() {}

// IDA_EA: 0x8241FFE
// IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackD2Ev
// DEMANGLED: Dispatcher_Antibot_DPCallBack::~Dispatcher_Antibot_DPCallBack()
// IDA_EA: 0x824202E
// IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackD0Ev
// DEMANGLED: Dispatcher_Antibot_DPCallBack::~Dispatcher_Antibot_DPCallBack()
Dispatcher_Antibot_DPCallBack::~Dispatcher_Antibot_DPCallBack() {}

// IDA_EA: 0x820B17A
// IDA_NAME: OldSrv_Dispatcher_Antibot_DPCallBack_dispatch_sig_CMD_DPROTO_CALLBACK
// DEMANGLED: OldSrv_Dispatcher_Antibot_DPCallBack_dispatch_sig_CMD_DPROTO_CALLBACK
int Dispatcher_Antibot_DPCallBack::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (!user) {
        return 0;
    }
    if (user->antibot_dproto_callback_count_ <= 4u) {
        ++user->antibot_dproto_callback_count_;
    }

    uint32_t encrypted_packet_size = 0u;
    if (packet.get_int(encrypted_packet_size) != 1) {
        if (user->antibot_dproto_callback_count_ <= 5u) {
            user->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(1), 605,
                user->antibot_dproto_callback_count_, 0);
        }
        return 42858;
    }

    if (encrypted_packet_size > kMaxDprotoPayload) {
        if (user->antibot_dproto_callback_count_ <= 5u) {
            user->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(2), 605,
                user->antibot_dproto_callback_count_,
                static_cast<int>(encrypted_packet_size));
        }
        return LineFunc(42880, kDprotoCallbackDispatchSignature, 0, 0);
    }

    // IDA_EA: 0x820B2A3..0x820B2B7 follows the same inbound PacketBuf rule
    // as CMD603: retrieve the backing pointer directly without get_len().
    const int payload_index = packet.get_index();
    uint8_t* const encrypted_packet = reinterpret_cast<uint8_t*>(
        packet.get_buf_ptr(payload_index));

    uint8_t decoded_packet[kDecodedPacketCapacity];
    uint16_t decoded_packet_size = kDecodedPacketCapacity;
    int plugin_result = 0;
    if (!TryAntibotCheckerRecvBufferForDecrypt(
            GlobalData::antibot_checker(),
            static_cast<uint32_t>(user->getAntibotKey()),
            encrypted_packet,
            static_cast<uint16_t>(encrypted_packet_size), decoded_packet,
            &decoded_packet_size, &plugin_result)) {
        return kAntibotAdapterUnavailableFailure;
    }
    return 0;
}

// IDA_EA: 0x8233B46
// IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBack4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Antibot_DPCallBack::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Antibot_DPCallBack::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B50
// IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBack7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Antibot_DPCallBack::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Antibot_DPCallBack::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
