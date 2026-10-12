#include "old_data/Login/inter_returns/login_inter_receivers.h"

#include <cstdio>
#include <cstring>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

#include "core/config/env/environment.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/ipg_runtime.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_data/Login/security/security_card.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/security/security_card_center.h"
#include "infra/log/format/clog.h"
#include "old_data/db/anti_cheat/auto_punish_payloads.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"
#include "infra/security/policy/network_security_scan_runtime.h"


namespace {

// This is the exact DB_SecurityCardActivate reply object.  The DB worker
// zeroes the first ten bytes before returning only activated at +0x0a.
#pragma pack(push, 1)
struct SIG_SECURITY_CARD_ACTIVATE_REPLY {
    uint32_t account_id;
    char cert_key[6];
    uint8_t activated;
    uint8_t reissue_history;
};

// Exact DB_SecurityCardIssue::_sendResult payload @0x842B410.  Its worker
// reserves 144 bytes, clears it, and writes the error dword at +0x8c.
struct SIG_SECURITY_CARD_ISSUE_REPLY {
    uint32_t account_id;      // +0x00
    char server_key[33];      // +0x04
    char phone[12];           // +0x25
    char cert_key[6];         // +0x31
    char card[39];            // +0x37
    char question[5];         // +0x5e
    char phone_hash[41];      // +0x63
    int32_t error;            // +0x8c
};

// Exact DB_SecurityCardRetransfer::_sendResult payload @0x842C7BE.  Its
// worker reserves 140 bytes, clears it, and writes the error dword at +0x88.
struct SIG_SECURITY_CARD_RETRANSFER_REPLY {
    uint32_t account_id;      // +0x00
    char server_key[33];      // +0x04
    char cert_key[6];         // +0x25
    char card[39];            // +0x2b
    char phone[12];           // +0x52
    char current_phone_hash[41]; // +0x5e
    uint8_t reserved_0x87;    // +0x87
    int32_t error;            // +0x88
};

// Inter_MonitorLoginOK @0x84C7E0C reads this monitor-specific return image.
// It is deliberately not SIG_LOGIN_DATA: the old dispatcher only consumes the
// unaligned mid dword at +0x0e, the error byte at +0x23, and the two dwords at
// +0x24/+0x28.  The unused bytes remain named as raw storage rather than being
// guessed as a DB-login structure.
struct SIG_MONITOR_LOGIN_OK_REPLY {
    uint8_t reserved_00_0d[0x0e];  // +0x00
    uint32_t member_id;            // +0x0e, old raw load is unaligned
    uint8_t reserved_12_22[0x11];  // +0x12
    uint8_t error_code;            // +0x23
    int32_t charac_no;             // +0x24
    int32_t channel_no;            // +0x28
};
#pragma pack(pop)

static_assert(sizeof(SIG_SECURITY_CARD_ACTIVATE_REPLY) == 0x0cu,
              "security-card activate reply ABI must stay 12 bytes");
static_assert(sizeof(SIG_SECURITY_CARD_ISSUE_REPLY) == 0x90u &&
                  offsetof(SIG_SECURITY_CARD_ISSUE_REPLY, server_key) == 0x04u &&
                  offsetof(SIG_SECURITY_CARD_ISSUE_REPLY, phone) == 0x25u &&
                  offsetof(SIG_SECURITY_CARD_ISSUE_REPLY, card) == 0x37u &&
                  offsetof(SIG_SECURITY_CARD_ISSUE_REPLY, question) == 0x5eu &&
                  offsetof(SIG_SECURITY_CARD_ISSUE_REPLY, error) == 0x8cu,
              "security-card issue reply ABI changed");
static_assert(sizeof(SIG_SECURITY_CARD_RETRANSFER_REPLY) == 0x8cu &&
                  offsetof(SIG_SECURITY_CARD_RETRANSFER_REPLY, server_key) == 0x04u &&
                  offsetof(SIG_SECURITY_CARD_RETRANSFER_REPLY, card) == 0x2bu &&
                  offsetof(SIG_SECURITY_CARD_RETRANSFER_REPLY, phone) == 0x52u &&
                  offsetof(SIG_SECURITY_CARD_RETRANSFER_REPLY, error) == 0x88u,
              "security-card retransfer reply ABI changed");
static_assert(sizeof(SIG_MONITOR_LOGIN_OK_REPLY) == 0x2cu &&
                  offsetof(SIG_MONITOR_LOGIN_OK_REPLY, member_id) == 0x0eu &&
                  offsetof(SIG_MONITOR_LOGIN_OK_REPLY, error_code) == 0x23u &&
                  offsetof(SIG_MONITOR_LOGIN_OK_REPLY, charac_no) == 0x24u &&
                  offsetof(SIG_MONITOR_LOGIN_OK_REPLY, channel_no) == 0x28u,
              "Inter_MonitorLoginOK monitor reply ABI changed");

const char kInterMonitorLoginOKFunction[] =
    "virtual int Inter_MonitorLoginOK::dispatch_sig(CUser*, char*, int)";

}  // namespace


// IDA_EA: 0x84C7AF0
// IDA_NAME: _ZN18Inter_SessionClose12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SessionClose::dispatch_sig(CUser *,char *,int)
int Inter_SessionClose::dispatch_sig(CUser* /*user*/,
                                     char* payload,
                                     int /*payload_size*/) {
    // The inter payload begins with the account id.  The original resolves
    // the currently connected user from that id rather than the dispatch
    // queue's optional user index.
    const unsigned int account_id =
        *reinterpret_cast<const unsigned int*>(payload);
    CUser* const current_user = G_CGameManager()->GetUserByAccId(account_id);
    if (!current_user) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, 1);
    packet.put_byte(0);
    packet.put_byte(18);
    packet.finalize(true);
    current_user->Send(packet);
    current_user->DisConnSig(static_cast<DISCONN_SIG>(15), true, 0);
    return 0;
}

// IDA_EA: 0x84EA006
// IDA_NAME: _ZN18Inter_SessionCloseC2Ev
// DEMANGLED: Inter_SessionClose::Inter_SessionClose(void)
Inter_SessionClose::Inter_SessionClose() {}

// IDA_EA: 0x84EA022 / 0x84EA052
// IDA_NAME: _ZN18Inter_SessionCloseD2Ev / _ZN18Inter_SessionCloseD0Ev
// DEMANGLED: Inter_SessionClose::~Inter_SessionClose()
Inter_SessionClose::~Inter_SessionClose() {}


// IDA_EA: 0x84D3D56
// IDA_NAME: _ZN26Inter_SecurityCardActivate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SecurityCardActivate::dispatch_sig(CUser *,char *,int)
int Inter_SecurityCardActivate::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_SECURITY_CARD_ACTIVATE_REPLY* const reply =
        reinterpret_cast<const SIG_SECURITY_CARD_ACTIVATE_REPLY*>(payload);
    if (reply->activated) {
        user->getSecurityCard()->setActivate(true);
        user->getSecurityCard()->setCertFlag(1);

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_CERT_KEY);
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_CERT_KEY, 1);
    }
    return 0;
}

// IDA_EA: 0x84EDB44
// IDA_NAME: _ZN26Inter_SecurityCardActivateC2Ev
// DEMANGLED: Inter_SecurityCardActivate::Inter_SecurityCardActivate(void)
Inter_SecurityCardActivate::Inter_SecurityCardActivate() {}

// IDA_EA: 0x84F8FEE
// IDA_NAME: _ZN26Inter_SecurityCardActivateD2Ev
// DEMANGLED: Inter_SecurityCardActivate::~Inter_SecurityCardActivate()
// IDA_EA: 0x84F901E
// IDA_NAME: _ZN26Inter_SecurityCardActivateD0Ev
// DEMANGLED: Inter_SecurityCardActivate::~Inter_SecurityCardActivate()
Inter_SecurityCardActivate::~Inter_SecurityCardActivate() {}

// IDA_EA: 0x84D3A22
// IDA_NAME: _ZN23Inter_SecurityCardIssue12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SecurityCardIssue::dispatch_sig(CUser *,char *,int)
int Inter_SecurityCardIssue::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    SIG_SECURITY_CARD_ISSUE_REPLY& reply =
        *reinterpret_cast<SIG_SECURITY_CARD_ISSUE_REPLY*>(payload);
    if (reply.error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_SECURITY_CARD_ISSUE,
            static_cast<unsigned char>(reply.error));
        return 0;
    }

    // IDA uses a 66-byte server-key output, a 48-byte account-id buffer,
    // and a second 66-byte encrypted phone buffer.
    char encrypted_server_key[66] = {};
    char encrypted_account_id[48] = {};
    char encrypted_phone[66] = {};
    std::snprintf(encrypted_account_id,
                  sizeof(encrypted_account_id),
                  "%s",
                  NumberToString(
                      static_cast<unsigned int>(user->get_acc_id()), 0));
    GlobalData::s_securityCardCenter->encryptString(
        reply.server_key, encrypted_server_key);
    GlobalData::s_securityCardCenter->encryptString(
        encrypted_account_id, encrypted_account_id);
    GlobalData::s_securityCardCenter->encryptString(reply.phone,
                                                      encrypted_phone);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_ISSUE);
    // CLIENT_IDA_EVIDENCE: CMD1 handler sub_55F2D0 @ 0x55F2D0 consumes
    // this success branch through the final Time Gate NPC u16. Its reads at
    // 0x5600BF, 0x56016B, and 0x5601CE belong only to error branches.
    packet.put_byte(1);
    packet.put_int(static_cast<int>(std::strlen(encrypted_account_id)));
    packet.put_str(encrypted_account_id,
                   static_cast<int>(std::strlen(encrypted_account_id)));
    packet.put_int(static_cast<int>(std::strlen(encrypted_server_key)));
    packet.put_str(encrypted_server_key,
                   static_cast<int>(std::strlen(encrypted_server_key)));
    packet.put_int(static_cast<int>(std::strlen(encrypted_phone)));
    packet.put_str(encrypted_phone,
                   static_cast<int>(std::strlen(encrypted_phone)));
    packet.finalize(true);
    user->Send(packet);

    user->getSecurityCard()->init();
    user->getSecurityCard()->setSecurityCardInfo(reply.card,
                                                  false,
                                                  false,
                                                  0,
                                                  0,
                                                  0,
                                                  reply.question);
    return 0;
}

// IDA_EA: 0x84EDB28
// IDA_NAME: _ZN23Inter_SecurityCardIssueC2Ev
// DEMANGLED: Inter_SecurityCardIssue::Inter_SecurityCardIssue(void)
Inter_SecurityCardIssue::Inter_SecurityCardIssue() {}

// IDA_EA: 0x84F903C
// IDA_NAME: _ZN23Inter_SecurityCardIssueD2Ev
// DEMANGLED: Inter_SecurityCardIssue::~Inter_SecurityCardIssue()
// IDA_EA: 0x84F906C
// IDA_NAME: _ZN23Inter_SecurityCardIssueD0Ev
// DEMANGLED: Inter_SecurityCardIssue::~Inter_SecurityCardIssue()
Inter_SecurityCardIssue::~Inter_SecurityCardIssue() {}

// IDA_EA: 0x84D3E78
// IDA_NAME: _ZN28Inter_SecurityCardRetransfer12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SecurityCardRetransfer::dispatch_sig(CUser *,char *,int)
int Inter_SecurityCardRetransfer::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    SIG_SECURITY_CARD_RETRANSFER_REPLY& reply =
        *reinterpret_cast<SIG_SECURITY_CARD_RETRANSFER_REPLY*>(payload);
    if (reply.error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_SECURITY_CARD_RETRANSFER,
            static_cast<unsigned char>(reply.error));
        return 0;
    }

    char encrypted_server_key[66] = {};
    char encrypted_account_id[48] = {};
    char encrypted_phone[66] = {};
    std::snprintf(encrypted_account_id,
                  sizeof(encrypted_account_id),
                  "%s",
                  NumberToString(
                      static_cast<unsigned int>(user->get_acc_id()), 0));
    GlobalData::s_securityCardCenter->encryptString(
        reply.server_key, encrypted_server_key);
    GlobalData::s_securityCardCenter->encryptString(
        encrypted_account_id, encrypted_account_id);
    GlobalData::s_securityCardCenter->encryptString(reply.phone,
                                                      encrypted_phone);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_RETRANSFER);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(std::strlen(encrypted_account_id)));
    packet.put_str(encrypted_account_id,
                   static_cast<int>(std::strlen(encrypted_account_id)));
    packet.put_int(static_cast<int>(std::strlen(encrypted_server_key)));
    packet.put_str(encrypted_server_key,
                   static_cast<int>(std::strlen(encrypted_server_key)));
    packet.put_int(static_cast<int>(std::strlen(encrypted_phone)));
    packet.put_str(encrypted_phone,
                   static_cast<int>(std::strlen(encrypted_phone)));
    packet.finalize(true);
    user->Send(packet);

    user->getSecurityCard()->init();
    user->getSecurityCard()->setSecurityCardInfo(reply.card,
                                                  true,
                                                  false,
                                                  0,
                                                  0,
                                                  0,
                                                  0);
    return 0;
}

// IDA_EA: 0x84EDB60
// IDA_NAME: _ZN28Inter_SecurityCardRetransferC2Ev
// DEMANGLED: Inter_SecurityCardRetransfer::Inter_SecurityCardRetransfer(void)
Inter_SecurityCardRetransfer::Inter_SecurityCardRetransfer() {}

// IDA_EA: 0x84F8FA0
// IDA_NAME: _ZN28Inter_SecurityCardRetransferD2Ev
// DEMANGLED: Inter_SecurityCardRetransfer::~Inter_SecurityCardRetransfer()
// IDA_EA: 0x84F8FD0
// IDA_NAME: _ZN28Inter_SecurityCardRetransferD0Ev
// DEMANGLED: Inter_SecurityCardRetransfer::~Inter_SecurityCardRetransfer()
Inter_SecurityCardRetransfer::~Inter_SecurityCardRetransfer() {}

// IDA_EA: 0x84E08FC
// IDA_NAME: _ZN25Inter_Hack_CheckMyAccount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Hack_CheckMyAccount::dispatch_sig(CUser *,char *,int)
int Inter_Hack_CheckMyAccount::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;

    const SIG_CHECK_MY_ACCOUNT_OK* const reply =
        reinterpret_cast<const SIG_CHECK_MY_ACCOUNT_OK*>(payload);
    if (!reply->uid) {
        const unsigned int old_owner_account_id = static_cast<unsigned int>(
            user->SetETC(709, static_cast<int>(reply->owner_account_id)));
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x2c5u),
            1u,
            old_owner_account_id,
            0u);
    }
    return 0;
}

// IDA_EA: 0x84EE5FC
// IDA_NAME: _ZN25Inter_Hack_CheckMyAccountC2Ev
// DEMANGLED: Inter_Hack_CheckMyAccount::Inter_Hack_CheckMyAccount(void)
Inter_Hack_CheckMyAccount::Inter_Hack_CheckMyAccount() {}

// IDA_EA: 0x84F71C4
// IDA_NAME: _ZN25Inter_Hack_CheckMyAccountD2Ev
// DEMANGLED: Inter_Hack_CheckMyAccount::~Inter_Hack_CheckMyAccount()
// IDA_EA: 0x84F71F4
// IDA_NAME: _ZN25Inter_Hack_CheckMyAccountD0Ev
// DEMANGLED: Inter_Hack_CheckMyAccount::~Inter_Hack_CheckMyAccount()
Inter_Hack_CheckMyAccount::~Inter_Hack_CheckMyAccount() {}


namespace {

#pragma pack(push, 1)
struct SIG_TIME_SYNC_LOGIN {
    PacketHeader header;             // +0x00
    uint8_t reserved_0a_11[8];       // +0x0a
    uint16_t first_time_value;       // +0x12
    uint16_t second_time_value;      // +0x14
};

struct SIG_PUNISH_CANCEL {
    PacketHeader header;             // +0x00
    uint8_t reserved_0a_0d[4];       // +0x0a
    uint16_t cancel_kind;            // +0x0e
    uint16_t punish_type;            // +0x10
};

// The monitor reply has a PacketHeader prefix, then its actual PC-room event
// fields.  The final textual reason is a NUL-terminated variable tail; it is
// not an opaque copied packet buffer.
struct SIG_PC_ROOM_LOGIN_OUT {
    PacketHeader header;             // +0x00
    uint8_t event_kind;              // +0x0a
    uint8_t reserved_0b_0e[4];       // +0x0b
    uint32_t charac_no;              // +0x0f
    uint8_t logout_kind;             // +0x13
    char text[1];                    // +0x14, NUL-terminated tail
};

// The monitor packet carries the standard ten-byte PacketHeader before the
// flag/count and its variable account-ID tail.  The old receiver consumes
// precisely +0x0a, +0x0b, and +0x0d onward; it performs no local length gate.
struct SIG_MONITOR_PUNISH_USER_REQ {
    PacketHeader header;             // +0x00
    uint8_t monitor_flag;            // +0x0a
    uint16_t user_count;             // +0x0b
    uint32_t account_ids[1];         // +0x0d, variable tail
};
#pragma pack(pop)

static_assert(sizeof(SIG_TIME_SYNC_LOGIN) == 0x16u &&
                  offsetof(SIG_TIME_SYNC_LOGIN, first_time_value) == 0x12u &&
                  offsetof(SIG_TIME_SYNC_LOGIN, second_time_value) == 0x14u,
              "time-sync reply ABI changed");
static_assert(sizeof(SIG_PUNISH_CANCEL) == 0x12u &&
                  offsetof(SIG_PUNISH_CANCEL, cancel_kind) == 0x0eu &&
                  offsetof(SIG_PUNISH_CANCEL, punish_type) == 0x10u,
              "punish-cancel reply ABI changed");
static_assert(offsetof(SIG_PC_ROOM_LOGIN_OUT, event_kind) == 0x0au &&
                  offsetof(SIG_PC_ROOM_LOGIN_OUT, charac_no) == 0x0fu &&
                  offsetof(SIG_PC_ROOM_LOGIN_OUT, logout_kind) == 0x13u &&
                  offsetof(SIG_PC_ROOM_LOGIN_OUT, text) == 0x14u,
              "PC-room login/logout reply offsets changed");
static_assert(offsetof(SIG_MONITOR_PUNISH_USER_REQ, monitor_flag) == 0x0au &&
                  offsetof(SIG_MONITOR_PUNISH_USER_REQ, user_count) == 0x0bu &&
                  offsetof(SIG_MONITOR_PUNISH_USER_REQ, account_ids) == 0x0du,
              "monitor-punish reply offsets changed");

}  // namespace


// IDA_EA: 0x84E33EC
// IDA_NAME: _ZN23Inter_SendTimeSyncLogin12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SendTimeSyncLogin::dispatch_sig(CUser *,char *,int)
int Inter_SendTimeSyncLogin::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;

    if (!user) {
        return 21460;
    }

    const SIG_TIME_SYNC_LOGIN& reply =
        *reinterpret_cast<const SIG_TIME_SYNC_LOGIN*>(payload);
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_TIME_SYNC);
    packet.put_short(reply.first_time_value);
    packet.put_short(reply.second_time_value);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE998
// IDA_NAME: _ZN23Inter_SendTimeSyncLoginC2Ev
// DEMANGLED: Inter_SendTimeSyncLogin::Inter_SendTimeSyncLogin(void)
Inter_SendTimeSyncLogin::Inter_SendTimeSyncLogin() {}

// IDA_EA: 0x84F6852
// IDA_NAME: _ZN23Inter_SendTimeSyncLoginD2Ev
// DEMANGLED: Inter_SendTimeSyncLogin::~Inter_SendTimeSyncLogin()
// IDA_EA: 0x84F6882
// IDA_NAME: _ZN23Inter_SendTimeSyncLoginD0Ev
// DEMANGLED: Inter_SendTimeSyncLogin::~Inter_SendTimeSyncLogin()
Inter_SendTimeSyncLogin::~Inter_SendTimeSyncLogin() {}

// IDA_EA: 0x84E4B02
// IDA_NAME: _ZN18Inter_PunishCancel12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PunishCancel::dispatch_sig(CUser *,char *,int)
int Inter_PunishCancel::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    (void)payload_size;

    const SIG_PUNISH_CANCEL& reply =
        *reinterpret_cast<const SIG_PUNISH_CANCEL*>(payload);
    if (user && reply.cancel_kind == 2 && reply.punish_type == 32 &&
        user->isTradePunishType(32)) {
        user->DelTradePunishType(reply.punish_type);
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_PUNISH_CANCEL);
        packet.put_short(reply.cancel_kind);
        packet.put_short(reply.punish_type);
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EEB20
// IDA_NAME: _ZN18Inter_PunishCancelC2Ev
// DEMANGLED: Inter_PunishCancel::Inter_PunishCancel(void)
Inter_PunishCancel::Inter_PunishCancel() {}

// IDA_EA: 0x84F6324
// IDA_NAME: _ZN18Inter_PunishCancelD2Ev
// DEMANGLED: Inter_PunishCancel::~Inter_PunishCancel()
// IDA_EA: 0x84F6354
// IDA_NAME: _ZN18Inter_PunishCancelD0Ev
// DEMANGLED: Inter_PunishCancel::~Inter_PunishCancel()
Inter_PunishCancel::~Inter_PunishCancel() {}

// IDA_EA: 0x84DA1FA
// IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOut12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::dispatch_sig(CUser *,char *,int)
int Inter_NoticePCRoomPlayerLoginOut::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    (void)payload_size;

    if (user->get_state() > 2) {
        const SIG_PC_ROOM_LOGIN_OUT& reply =
            *reinterpret_cast<const SIG_PC_ROOM_LOGIN_OUT*>(payload);
        if (user->get_charac_no(-1) == static_cast<int>(reply.charac_no)) {
            PacketGuard packet;
            packet.put_header(0, ENUM_NOTIPACKET_PCROOM_PLAYER_LOGIN_LOGOUT);
            packet.put_byte(reply.event_kind);
            packet.put_byte(reply.logout_kind);
            const int text_length = static_cast<int>(std::strlen(reply.text));
            packet.put_int(text_length);
            packet.put_str(reply.text, text_length);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_NoticePCRoomPlayerLoginOut::dispatch_sig(CUser*, char*, int)",
                16875,
                "PCRoom : Inter_NoticePCRoomPlayerLoginOut::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                reply.charac_no);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE068
// IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutC2Ev
// DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::Inter_NoticePCRoomPlayerLoginOut(void)
Inter_NoticePCRoomPlayerLoginOut::Inter_NoticePCRoomPlayerLoginOut() {}

// IDA_EA: 0x84F8100
// IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutD2Ev
// DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::~Inter_NoticePCRoomPlayerLoginOut()
// IDA_EA: 0x84F8130
// IDA_NAME: _ZN32Inter_NoticePCRoomPlayerLoginOutD0Ev
// DEMANGLED: Inter_NoticePCRoomPlayerLoginOut::~Inter_NoticePCRoomPlayerLoginOut()
Inter_NoticePCRoomPlayerLoginOut::~Inter_NoticePCRoomPlayerLoginOut() {}

// IDA_EA: 0x84DB294
// IDA_NAME: _ZN26Inter_MonitorPunishUserReq12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorPunishUserReq::dispatch_sig(CUser *,char *,int)
int Inter_MonitorPunishUserReq::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_MONITOR_PUNISH_USER_REQ& request =
        *reinterpret_cast<const SIG_MONITOR_PUNISH_USER_REQ*>(payload);
    cMyTrace request_trace(
        "virtual int Inter_MonitorPunishUserReq::dispatch_sig(CUser*, char*, int)",
        17340,
        0);
    request_trace("Monitor Punish User / m_flag = %d / user_count",
                  request.monitor_flag);

    if (request.monitor_flag == 0) {
        for (uint32_t index = 0; index < request.user_count; ++index) {
            const uint32_t account_id = request.account_ids[index];
            if (account_id > 0x63u &&
                G_CGameManager()->isConnectUserByAccId(account_id)) {
                CUser* const target =
                    G_CGameManager()->GetUserByAccId(account_id);
                target->DisConnSig(static_cast<DISCONN_SIG>(44), true, 0);

                cMyTrace kick_trace(
                    "virtual int Inter_MonitorPunishUserReq::dispatch_sig(CUser*, char*, int)",
                    17364,
                    0);
                kick_trace("System Kick User : %s", NumberToString(account_id, 0));
            }
        }
    }
    return 0;
}

// IDA_EA: 0x84EE1F0
// IDA_NAME: _ZN26Inter_MonitorPunishUserReqC2Ev
// DEMANGLED: Inter_MonitorPunishUserReq::Inter_MonitorPunishUserReq(void)
Inter_MonitorPunishUserReq::Inter_MonitorPunishUserReq() {}

// IDA_EA: 0x84F7D0A
// IDA_NAME: _ZN26Inter_MonitorPunishUserReqD2Ev
// DEMANGLED: Inter_MonitorPunishUserReq::~Inter_MonitorPunishUserReq()
// IDA_EA: 0x84F7D3A
// IDA_NAME: _ZN26Inter_MonitorPunishUserReqD0Ev
// DEMANGLED: Inter_MonitorPunishUserReq::~Inter_MonitorPunishUserReq()
Inter_MonitorPunishUserReq::~Inter_MonitorPunishUserReq() {}

// IDA_EA: 0x84C7E0C
// IDA_NAME: _ZN20Inter_MonitorLoginOK12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorLoginOK::dispatch_sig(CUser *,char *,int)
int Inter_MonitorLoginOK::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    // The raw dispatcher has this third ABI argument but does not read it.
    (void)payload_size;

    if (!user->get_state()) {
        return 8298;
    }

    const SIG_MONITOR_LOGIN_OK_REPLY& reply =
        *reinterpret_cast<const SIG_MONITOR_LOGIN_OK_REPLY*>(payload);

    user->SetLastLoginCharacNo(0);
    user->SetLastLoginChannelNo(0);
    user->SetValidLastLoginCharac(false);

    if (G_GameWorld()->GetChannelType() == 8 && reply.charac_no == 0 &&
        reply.channel_no == 0) {
        cMyTrace trace(kInterMonitorLoginOKFunction, 8309, 0);
        trace("Inter_MonitorLoginOK() mid(%s), charac_no(%d), channel_no(%d)",
              NumberToString(reply.member_id, 0),
              reply.charac_no,
              reply.channel_no);
        return 8310;
    }

    cMyTrace trace(kInterMonitorLoginOKFunction, 8314, 0);
    trace("Inter_MonitorLoginOK() mid(%s), charac_no(%d), channel_no(%d)",
          NumberToString(reply.member_id, 0),
          reply.charac_no,
          reply.channel_no);

    PacketGuard packet;
    packet.put_header(1, 1);

    switch (reply.error_code) {
    case 2:
        packet.put_byte(0);
        packet.put_byte(18);
        packet.finalize(true);
        user->Send(packet);
        user->DisConnSig(static_cast<DISCONN_SIG>(12), true, 0);
        {
            cMyTrace already_trace(kInterMonitorLoginOKFunction, 8328, 0);
            already_trace("info.err_code == SIG_LOGIN_DATA::ERROR_ALREADY");
        }
        return 0;

    case 3:
        packet.put_byte(0);
        packet.put_byte(65);
        packet.finalize(true);
        user->Send(packet);
        user->DisConnSig(static_cast<DISCONN_SIG>(21), true, 0);
        {
            cMyTrace denied_trace(kInterMonitorLoginOKFunction, 8341, 0);
            denied_trace("info.err_code == SIG_LOGIN_DATA::ERROR_ALREADY");
        }
        return 0;

    case 1:
        break;

    default:
        {
            cMyTrace invalid_trace(kInterMonitorLoginOKFunction, 8713, 5);
            invalid_trace("[%s][%d]", kInterMonitorLoginOKFunction, 8713);
        }
        return 8717;
    }

    packet.put_byte(1);

    const int account_name_length =
        static_cast<int>(::strnlen(user->get_acc_name(), 0x14u));
    const bool pad_activated = user->getPad()->isActivate();
    packet.put_int(account_name_length);
    packet.put_str(user->get_acc_name(), account_name_length);
    packet.put_byte(pad_activated);
    packet.put_byte(0);

    if (G_CEnvironment()->get_server_group() == 3) {
        packet.put_byte(1);
    } else {
        packet.put_byte(user->getGarenaAuthData()->getMobileAuth());
    }

    packet.put_byte(user->GetAge());
    packet.put_byte(user->IsPCRoomUser());
    packet.put_byte(G_GameWorld()->GetChannelType());
    packet.put_byte(G_CDataManager()->GetUpgradeRevisionPvPChannel());
    packet.put_byte(user->isTournamentAccount());
    packet.put_int(user->get_acc_id());

    packet.put_int(static_cast<int>(
        std::strlen(G_CEnvironment()->server_envir_.relay_ip)));
    packet.put_str(G_CEnvironment()->server_envir_.relay_ip, 16);
    packet.put_int(G_CEnvironment()->server_envir_.relay_tcp_port);
    packet.put_int(G_CEnvironment()->server_envir_.relay_udp_port);

    int sex = user->getSex();
    if (sex == -1) {
        sex = 1;
    }
    packet.put_byte(static_cast<signed char>(sex));
    packet.put_byte(user->GetReliablePerson());
    packet.put_byte(user->getSecurityCard()->isActivate());
    packet.put_byte(user->getSecurityCard()->isCertified());
    packet.put_byte(user->getPad()->getVersionInfo());
    packet.put_byte(user->getPad()->isCertified());
    packet.put_byte(user->isRestingUser());

    uint8_t antibot_output[0x4c];
    std::memset(antibot_output, 0, sizeof(antibot_output));
    sockaddr peer_address;
    socklen_t peer_address_length = 16;
    if (::getpeername(user->network_.get_socket(),
                      &peer_address,
                      &peer_address_length)) {
        return 8468;
    }

    // The old body loads addr.sa_data+2 before this call.  It has no later
    // data dependency, but getpeername remains the required transport gate.
    const int client_version = user->getClientVersion();
    const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
    const int process_sequence = G_CEnvironment()->getProcessSequence();
    const int antibot_key = AntibotCheckerAddClient(
        GlobalData::antibot_checker(),
        process_sequence,
        0,
        account_id,
        client_version,
        antibot_output);
    if (antibot_key < 0) {
        const int logged_client_version = user->getClientVersion();
        const uint32_t logged_account_id =
            static_cast<uint32_t>(user->get_acc_id());
        const int logged_process_sequence =
            G_CEnvironment()->getProcessSequence();
        cMyTrace antibot_trace(kInterMonitorLoginOKFunction, 8475, 0);
        antibot_trace(
            "AddClient ProcessSequence : %d, qq_id : %s, ClientVersion : %d, return : %d",
            logged_process_sequence,
            NumberToString(logged_account_id, 0),
            logged_client_version,
            antibot_key);
    }

    user->antibot_packet_state_ = 0;
    user->antibot_send_buffer_count_ = 0;
    user->antibot_dproto_callback_count_ = 0;

    const int p2p_client_version = user->getClientVersion();
    const uint32_t p2p_account_id = static_cast<uint32_t>(user->get_acc_id());
    if (AntibotCheckerP2PAddClient(GlobalData::antibot_checker(),
                                   p2p_account_id,
                                   -1,
                                   p2p_client_version) == -1) {
        const int logged_client_version = user->getClientVersion();
        const uint32_t logged_account_id =
            static_cast<uint32_t>(user->get_acc_id());
        const int logged_process_sequence =
            G_CEnvironment()->getProcessSequence();
        cMyTrace p2p_trace(kInterMonitorLoginOKFunction, 8492, 0);
        p2p_trace(
            "P2PAddClient ProcessSequence : %d, qq_id : %s, ClientVersion : %d, return : -1",
            logged_process_sequence,
            NumberToString(logged_account_id, 0),
            logged_client_version);
    }
    user->setAntibotKey(antibot_key);

    packet.put_byte(user->getUserEventCharacterFlag());
    packet.put_byte(user->getOnlinePreliminaryPlayType());
    packet.put_byte(
        online_preliminary::COnlinePreliminary::GetPlayCountPerOneTeam());
    packet.put_byte(
        GlobalData::s_pGMAccounts_->isGM(
            static_cast<uint32_t>(user->get_acc_id())));
    if (GlobalData::s_timeGate_->isOpen()) {
        packet.put_short(GlobalData::s_timeGate_->getNpcIndex());
    } else {
        packet.put_short(0);
    }
    packet.finalize(true);
    user->Send(packet);

    user->history_stream_ = 0;
    G_TimerQueue()->InsertTimer(
        0,
        user->GetUID(),
        0,
        300,
        static_cast<unsigned int>(
            static_cast<uint16_t>(user->get_unique_id())),
        0);
    _NS_NonClient_Request(user);
    GlobalData::s_pIPGHelper->IPGQuery(user);
    user->SetLastLoginCharacNo(reply.charac_no);
    user->SetLastLoginChannelNo(reply.channel_no);
    DB_LoadDnfHackLog::makeRequest(
        user->GetUID(), static_cast<uint32_t>(user->get_acc_id()));
    return 0;
}

// IDA_EA: 0x84EA1AE
// IDA_NAME: _ZN20Inter_MonitorLoginOKC2Ev
// DEMANGLED: Inter_MonitorLoginOK::Inter_MonitorLoginOK(void)
Inter_MonitorLoginOK::Inter_MonitorLoginOK() {}

// IDA_EA: 0x84EA1CA
// IDA_NAME: _ZN20Inter_MonitorLoginOKD2Ev
// DEMANGLED: Inter_MonitorLoginOK::~Inter_MonitorLoginOK()
// IDA_EA: 0x84EA1FA
// IDA_NAME: _ZN20Inter_MonitorLoginOKD0Ev
// DEMANGLED: Inter_MonitorLoginOK::~Inter_MonitorLoginOK()
Inter_MonitorLoginOK::~Inter_MonitorLoginOK() {}
