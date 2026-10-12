#include "infra/security/policy/inter_antibot_return_dispatchers.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const ENUM_NOTIPACKET kNotifyAntibot = ENUM_NOTIPACKET_ANTIBOT;
const ENUM_NOTIPACKET kNotifyDproto = ENUM_NOTIPACKET_DPROTO;
const ENUM_NOTIPACKET kNotifySafeMode = ENUM_NOTIPACKET_SAFE_MODE;

struct AntibotSendBufferHeader {
    uint32_t destination_kind;
    uint32_t user_uid;
    int16_t buffer_size;
    uint8_t padding[2];
};

struct AntibotPunishHeader {
    uint32_t user_uid;
    uint8_t punish_mode;
    uint8_t padding[3];
    uint32_t punish_info_0;
    int32_t punish_info_1;
};

static_assert(offsetof(AntibotSendBufferHeader, destination_kind) == 0u,
              "Inter slot 594 destination flag must be at +0");
static_assert(offsetof(AntibotSendBufferHeader, user_uid) == 4u,
              "Inter slot 594 target UID must be at +4");
static_assert(offsetof(AntibotSendBufferHeader, buffer_size) == 8u,
              "Inter slot 594 payload length must be at +8");
static_assert(sizeof(AntibotSendBufferHeader) == 12u,
              "Inter slot 594 header must remain 12 bytes");
static_assert(offsetof(AntibotPunishHeader, user_uid) == 0u,
              "Inter slot 595 target UID must be at +0");
static_assert(offsetof(AntibotPunishHeader, punish_mode) == 4u,
              "Inter slot 595 punish mode must be at +4");
static_assert(offsetof(AntibotPunishHeader, punish_info_0) == 8u,
              "Inter slot 595 first punish word must be at +8");
static_assert(offsetof(AntibotPunishHeader, punish_info_1) == 12u,
              "Inter slot 595 second punish word must be at +12");
static_assert(sizeof(AntibotPunishHeader) == 16u,
              "Inter slot 595 packet must remain 16 bytes");

uint16_t ReadU16(const char* source, int offset) {
    uint16_t value = 0;
    memcpy(&value, source + offset, sizeof(value));
    return value;
}

int16_t ReadS16(const char* source, int offset) {
    int16_t value = 0;
    memcpy(&value, source + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* source, int offset) {
    uint32_t value = 0;
    memcpy(&value, source + offset, sizeof(value));
    return value;
}

int32_t ReadS32(const char* source, int offset) {
    int32_t value = 0;
    memcpy(&value, source + offset, sizeof(value));
    return value;
}

CUser* FindUserByUid(uint32_t user_uid) {
    CGameManager* const game_manager = G_CGameManager();
    return game_manager ? game_manager->getUser(static_cast<int>(user_uid), 0)
                        : 0;
}

void SendSafeModeState(CUser* user, int enabled) {
    PacketGuard packet;
    packet.put_header(0, kNotifySafeMode);
    packet.put_byte(enabled);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x84E43BC
// IDA_NAME: OldSrv_Inter_Antibot_SendBufferClient_dispatch_sig_send_NOTI_DPROTO_OR_ANTIBOT
// DEMANGLED: Inter_Antibot_SendBufferClient::dispatch_sig(CUser *,char *,int)
int Inter_Antibot_SendBufferClient::dispatch_sig(CUser*,
                                                 char* payload,
                                                 int payload_size) {
    if (!payload || payload_size <
                         static_cast<int>(sizeof(AntibotSendBufferHeader))) {
        return 0;
    }

    const int buffer_size = static_cast<int>(ReadS16(payload, 8));
    if (buffer_size < 0 ||
        buffer_size > payload_size -
                          static_cast<int>(sizeof(AntibotSendBufferHeader))) {
        return 0;
    }

    CUser* const target = FindUserByUid(ReadU32(payload, 4));
    if (!target || target->get_state() == 0) {
        if (target && target->antibot_send_buffer_count_ <= 4u) {
            const uint8_t previous = target->antibot_send_buffer_count_;
            ++target->antibot_send_buffer_count_;
            target->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(3), 594,
                previous, 0);
        }
        return 0;
    }

    const ENUM_NOTIPACKET notify_code =
        ReadU32(payload, 0) != 0u ? kNotifyDproto : kNotifyAntibot;
    if (notify_code == kNotifyAntibot) {
        if (target->antibot_packet_state_ == 2u) {
            target->history_log().AntibotPacketLog(
                static_cast<AntibotPacketLogType::T>(2));
            target->antibot_packet_state_ = 3u;
        } else if (target->antibot_packet_state_ != 4u) {
            target->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(4), 603,
                buffer_size,
                static_cast<int>(target->antibot_packet_state_));
        }
    }

    PacketGuard packet;
    packet.put_header(0, notify_code);
    packet.put_binary(payload + sizeof(AntibotSendBufferHeader), buffer_size);
    packet.finalize(false);
    const int send_result = target->Send(packet);

    if (notify_code == kNotifyAntibot) {
        if (target->antibot_packet_state_ == 3u) {
            target->history_log().AntibotPacketLog(
                static_cast<AntibotPacketLogType::T>(3));
            target->antibot_packet_state_ = 4u;
        } else if (target->antibot_packet_state_ != 4u) {
            target->history_log().AbnormalUser(
                static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(4), 603,
                buffer_size,
                static_cast<int>(target->antibot_packet_state_));
        }
    } else if (target->antibot_send_buffer_count_ <= 4u) {
        const uint8_t previous = target->antibot_send_buffer_count_;
        ++target->antibot_send_buffer_count_;
        target->history_log().SendBuffertoClientLog(
            static_cast<int>(ReadU32(payload, 0)),
            static_cast<int16_t>(buffer_size), previous, send_result != 0);
    }
    return 0;
}

// IDA_EA: 0x84E4774
// IDA_NAME: _ZN26Inter_Antibot_PunishClient12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Antibot_PunishClient::dispatch_sig(CUser *,char *,int)
int Inter_Antibot_PunishClient::dispatch_sig(CUser*,
                                             char* payload,
                                             int payload_size) {
    if (!payload || payload_size < static_cast<int>(sizeof(AntibotPunishHeader))) {
        return 0;
    }

    CUser* const target = FindUserByUid(ReadU32(payload, 0));
    if (!target || target->get_state() == 0) {
        return 0;
    }

    const uint8_t punish_mode = static_cast<uint8_t>(payload[4]);
    if (punish_mode == 2u ||
        (punish_mode == 3u && ReadS32(payload, 12) > 0)) {
        target->DisConnSig(static_cast<DISCONN_SIG>(35), true, 0);
    }
    return 0;
}

// IDA_EA: 0x84E48C2
// IDA_NAME: _ZN31Inter_Antibot_ProcessProtectMsg12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Antibot_ProcessProtectMsg::dispatch_sig(CUser *,char *,int)
int Inter_Antibot_ProcessProtectMsg::dispatch_sig(CUser*,
                                                  char* payload,
                                                  int payload_size) {
    // Every decoded tagTSafeModePkg variant begins with its 16-bit type and
    // 32-bit destination UID at offset +2.
    if (!payload || payload_size < 6) {
        return 0;
    }

    CUser* const target = FindUserByUid(ReadU32(payload, 2));
    if (!target || target->get_state() == 0) {
        return 0;
    }

    uint8_t protection_level = 0u;
    uint16_t event_type = 0u;
    switch (ReadU16(payload, 0)) {
        case 19:
            if (payload_size < 8) {
                return 0;
            }
            event_type = ReadU16(payload, 6);
            break;
        case 18:
        case 20:
            if (payload_size < 12) {
                return 0;
            }
            protection_level = static_cast<uint8_t>(payload[8]);
            event_type = ReadU16(payload, 10);
            break;
        default:
            break;
    }

    // The original code reports this value to its trace sink only; retain the
    // exact payload read without inventing a gameplay action for it.
    (void)event_type;

    if (protection_level == 1u || protection_level == 8u) {
        SendSafeModeState(target, 1);
    } else if (protection_level == 0u) {
        SendSafeModeState(target, 0);
    }
    return 0;
}

// IDA_EA: 0x84EE74C
// IDA_NAME: _ZN30Inter_Antibot_SendBufferClientC2Ev
// DEMANGLED: Inter_Antibot_SendBufferClient::Inter_Antibot_SendBufferClient(void)
Inter_Antibot_SendBufferClient::Inter_Antibot_SendBufferClient()
    : ISigDispatcher() {}

// IDA_EA: 0x84F640E
// IDA_NAME: _ZN30Inter_Antibot_SendBufferClientD2Ev
// DEMANGLED: Inter_Antibot_SendBufferClient::~Inter_Antibot_SendBufferClient()
// IDA_EA: 0x84F643E
// IDA_NAME: _ZN30Inter_Antibot_SendBufferClientD0Ev
// DEMANGLED: Inter_Antibot_SendBufferClient::~Inter_Antibot_SendBufferClient()
Inter_Antibot_SendBufferClient::~Inter_Antibot_SendBufferClient() {}

// IDA_EA: 0x84EE768
// IDA_NAME: _ZN26Inter_Antibot_PunishClientC2Ev
// DEMANGLED: Inter_Antibot_PunishClient::Inter_Antibot_PunishClient(void)
Inter_Antibot_PunishClient::Inter_Antibot_PunishClient() : ISigDispatcher() {}

// IDA_EA: 0x84F63C0
// IDA_NAME: _ZN26Inter_Antibot_PunishClientD2Ev
// DEMANGLED: Inter_Antibot_PunishClient::~Inter_Antibot_PunishClient()
// IDA_EA: 0x84F63F0
// IDA_NAME: _ZN26Inter_Antibot_PunishClientD0Ev
// DEMANGLED: Inter_Antibot_PunishClient::~Inter_Antibot_PunishClient()
Inter_Antibot_PunishClient::~Inter_Antibot_PunishClient() {}

// IDA_EA: 0x84EE784
// IDA_NAME: _ZN31Inter_Antibot_ProcessProtectMsgC2Ev
// DEMANGLED: Inter_Antibot_ProcessProtectMsg::Inter_Antibot_ProcessProtectMsg(void)
Inter_Antibot_ProcessProtectMsg::Inter_Antibot_ProcessProtectMsg()
    : ISigDispatcher() {}

// IDA_EA: 0x84F6372
// IDA_NAME: _ZN31Inter_Antibot_ProcessProtectMsgD2Ev
// DEMANGLED: Inter_Antibot_ProcessProtectMsg::~Inter_Antibot_ProcessProtectMsg()
// IDA_EA: 0x84F63A2
// IDA_NAME: _ZN31Inter_Antibot_ProcessProtectMsgD0Ev
// DEMANGLED: Inter_Antibot_ProcessProtectMsg::~Inter_Antibot_ProcessProtectMsg()
Inter_Antibot_ProcessProtectMsg::~Inter_Antibot_ProcessProtectMsg() {}
