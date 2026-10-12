#include "old_game/player/core/cuser.h"

#include "old_data/PVF/data/datamanager/captcha_data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/server_utility.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/account/db_force_save_simple_sso.h"
#include "old_data/db/auth_security/db_count_of_human_certify.h"
#include "old_data/db/pvp_party/db_save_user_handicap.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>
#include <cstring>

// IDA_EA: 0x8695038
// IDA_NAME: _ZN8WongWork12CMCAPManager15incExposedCountEv
// DEMANGLED: WongWork::CMCAPManager::incExposedCount(void)
WongWork::CMCAPManager* WongWork::CMCAPManager::incExposedCount() {
    ++exposed_count_;
    return this;
}

// IDA_EA: 0x86853E6
// IDA_NAME: _ZN15cUserHistoryLog15RequestCleanPadEjc
// DEMANGLED: cUserHistoryLog::RequestCleanPad(unsigned int,char)
int cUserHistoryLog::RequestCleanPad(unsigned int point, char request_type) {
    return owner_->LogHistory("CleanPad,%d,%d", point, request_type);
}

// IDA_EA: 0x8649AC4
// IDA_NAME: _ZN5CUser24GenerateCleanPadVeriDataERtRhS0_
// DEMANGLED: CUser::GenerateCleanPadVeriData(unsigned short &,unsigned char &,unsigned short &)
uint16_t* CUser::GenerateCleanPadVeriData(uint16_t& first,
                                          uint8_t& operation,
                                          uint16_t& second) {
    clean_pad_verification_first_ =
        static_cast<uint16_t>(get_rand_int(0xFFFF) + 1u);
    clean_pad_verification_operation_ =
        static_cast<uint8_t>(get_rand_int(6));
    clean_pad_verification_second_ =
        static_cast<uint16_t>(get_rand_int(0xFFFF) + 1u);
    first = clean_pad_verification_first_;
    operation = clean_pad_verification_operation_;
    second = clean_pad_verification_second_;
    return &second;
}

// IDA_EA: 0x867F4C8
// IDA_NAME: _ZN5CUser17checkHumanCertifyEiPt
// DEMANGLED: CUser::checkHumanCertify(int,unsigned short *)
bool CUser::checkHumanCertify(int answer_size, unsigned short* answer) {
    // CMD145 bounds this C2S value to the ten-byte CAPTCHA image before this
    // comparison.  The count request intentionally precedes the comparison,
    // exactly as in the original anti-bot audit transaction.
    DB_CountOfHumanCertify::makeRequest(
        static_cast<unsigned int>(get_acc_id()));

    if (std::memcmp(answer,
                    clean_pad_captcha_text_,
                    static_cast<size_t>(answer_size)) == 0) {
        if (human_certify_error_count_ != 0u) {
            DB_ForceSaveSimpleSSO::makeRequest(
                static_cast<unsigned int>(get_acc_id()), 0);
        }
        human_certify_error_count_ = 0u;
        setHumanCertified(true);
        return true;
    }

    ++human_certify_error_count_;
    if (isCleanPadVerifyLimit() && mail_send_certified_ != 0u) {
        DB_ForceSaveSimpleSSO::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            static_cast<int>(human_certify_error_count_));
        SendCmdErrorPacket(ENUM_CMDPACKET_RPY_HUMAN_CERTIFY, 8);
    }
    setHumanCertified(true);
    return false;
}

namespace {

bool IsLowValueMailItem(const uint32_t item_id) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    return item && item->is_stackable() != 1 && item->get_rarity() <= 1;
}

void QueueHumanCertifyMailAudit(CUser& user,
                                const MSG_MAILBOX_SEND& request) {
    if (!user.getCurCharacR()) {
        return;
    }

    const int current_money = user.getCurCharacInvenR()->get_money();
    const int random_percent = get_rand_int(25);
    const long double threshold =
        static_cast<long double>(current_money) *
        (0.7L + static_cast<long double>(random_percent) * 0.01L);
    if (static_cast<unsigned int>(static_cast<long long>(threshold)) >
        request.money) {
        return;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire("user.cpp", 41259),
                       true);
    guard << 394;
    guard << user.GetUID();
    SIG_FIND_FACTORY_HUB* const payload =
        guard.GetInBuffer<SIG_FIND_FACTORY_HUB>();
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = user.get_acc_id();
    std::strncpy(payload->charac_name,
                 request.receiver_name,
                 request.receiver_name_length);
    payload->fatigue_check =
        user.getCurCharacUsedFatigue() >= get_rand_int(20) + 120 ? 1u : 0u;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

}  // namespace

// IDA_EA: 0x8689F12
// IDA_NAME: _ZN5CUser8SendMailEb
// DEMANGLED: CUser::SendMail(bool)
void* CUser::SendMail(bool db_receiver_verified) {
    mail_send_certified_ = 1u;
    if (pending_mail_is_multi_ != 0u) {
        return SendMultiMail(db_receiver_verified);
    }

    MSG_MAILBOX_SEND& request =
        *reinterpret_cast<MSG_MAILBOX_SEND*>(pending_mail_message_);
    if (db_receiver_verified) {
        MSG_CHECK_MAIL_RECEIVER check;
        check.set<MSG_CHECK_MAIL_RECEIVER::kMailboxSend>(request);
        const int error = WongWork::CMailBoxHelper::ReqDBCheckReceiver(
            this, request.receiver_name, &check);
        if (error) {
            SendCmdErrorPacket(ENUM_CMDPACKET_MAILBOX_SEND,
                               static_cast<unsigned char>(error));
        } else {
            IncrementTradeGold(request.money,
                               (!request.item_id ||
                                IsLowValueMailItem(request.item_id))
                                   ? 1
                                   : 0);
        }
        QueueHumanCertifyMailAudit(*this, request);
    } else {
        SendCmdErrorPacket(ENUM_CMDPACKET_MAILBOX_SEND, 0x87u);
    }
    return std::memset(pending_mail_message_, 0,
                       sizeof(pending_mail_message_));
}

// IDA_EA: 0x868A2DC
// IDA_NAME: _ZN5CUser13SendMultiMailEb
// DEMANGLED: CUser::SendMultiMail(bool)
void* CUser::SendMultiMail(bool db_receiver_verified) {
    MSG_MULTI_MAILBOX_SEND& request =
        *reinterpret_cast<MSG_MULTI_MAILBOX_SEND*>(pending_multi_mail_message_);
    if (db_receiver_verified) {
        MSG_CHECK_MAIL_RECEIVER check;
        check.set<MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend>(request);
        const int error = WongWork::CMailBoxHelper::ReqDBCheckReceiver(
            this, request.receiver_name, &check);
        if (error) {
            SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND,
                               static_cast<unsigned char>(error));
        } else {
            for (uint8_t index = 0; index < request.item_count; ++index) {
                const uint32_t item_id = request.items[index].item_id;
                IncrementTradeGold(request.money,
                                   (!item_id || IsLowValueMailItem(item_id))
                                       ? 1
                                       : 0);
            }
        }
    } else {
        SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 0x87u);
    }
    return std::memset(pending_multi_mail_message_, 0,
                       sizeof(pending_multi_mail_message_));
}

// IDA_EA: 0x868A4B6
// IDA_NAME: _ZN5CUser15set_MailMsgDataEP16MSG_MAILBOX_SEND
// DEMANGLED: CUser::set_MailMsgData(MSG_MAILBOX_SEND *)
void* CUser::set_MailMsgData(MSG_MAILBOX_SEND* message) {
    pending_mail_is_multi_ = 0u;
    return std::memcpy(pending_mail_message_, message,
                       sizeof(pending_mail_message_));
}

// IDA_EA: 0x868A4E8
// IDA_NAME: _ZN5CUser20set_MultimailMsgDataEP22MSG_MULTI_MAILBOX_SEND
// DEMANGLED: CUser::set_MultimailMsgData(MSG_MULTI_MAILBOX_SEND *)
void* CUser::set_MultimailMsgData(MSG_MULTI_MAILBOX_SEND* message) {
    pending_mail_is_multi_ = 1u;
    return std::memcpy(pending_multi_mail_message_, message,
                       sizeof(pending_multi_mail_message_));
}

// IDA_EA: 0x868A51A
// IDA_NAME: _ZN5CUser18reqSendMailCertifyEv
// DEMANGLED: CUser::reqSendMailCertify(void)
int CUser::reqSendMailCertify() {
    if (GlobalData::s_capchaDataManager.isEmpty()) {
        return 0;
    }

    MSG_UDP_CAPCHA_CERTIFY* const captcha =
        GlobalData::s_capchaDataManager.peekData();
    std::memcpy(clean_pad_captcha_text_, captcha,
                sizeof(clean_pad_captcha_text_));

    PacketGuard packet;
    packet.put_header(0, 125);
    packet.put_byte(5);
    packet.put_int(static_cast<int>(captcha->image_size));
    packet.put_binary(reinterpret_cast<const char*>(captcha->image),
                      static_cast<int>(captcha->image_size));
    packet.put_byte(0);

    uint16_t first = 0;
    uint8_t operation = 0;
    uint16_t second = 0;
    GenerateCleanPadVeriData(first, operation, second);
    packet.put_short(first);
    packet.put_byte(operation);
    packet.put_short(second);
    packet.put_byte(static_cast<unsigned char>(human_certify_error_count_));
    packet.put_byte(1);
    packet.finalize(true);
    Send(packet);

    setHumanCertified(false);
    mail_send_certified_ = 0u;
    if (human_certify_timer_key_ > 0x7ffffffeu) {
        human_certify_timer_key_ = 0u;
    }
    const unsigned int timer_key = ++human_certify_timer_key_;
    G_TimerQueue()->InsertTimer(0, GetUID(), 9, 60, timer_key, -1);
    return 1;
}

// IDA_EA: 0x867F26E
// IDA_NAME: _ZN5CUser15reqHumanCertifyEv
// DEMANGLED: CUser::reqHumanCertify(void)
int CUser::reqHumanCertify() {
    if (isHumanCertified() != 1) {
        return 0;
    }

    history_log_.RequestCleanPad(mcap_manager_.getPoint(), 0);
    mcap_manager_.incExposedCount();
    if (GlobalData::s_capchaDataManager.isEmpty()) {
        return 0;
    }

    MSG_UDP_CAPCHA_CERTIFY* const captcha =
        GlobalData::s_capchaDataManager.peekData();
    std::memcpy(clean_pad_captcha_text_,
                captcha->challenge,
                sizeof(clean_pad_captcha_text_));

    PacketGuard packet;
    packet.put_header(0, 125);
    packet.put_byte(5);
    packet.put_int(static_cast<int>(captcha->image_size));
    packet.put_binary(reinterpret_cast<const char*>(captcha->image),
                      static_cast<int>(captcha->image_size));
    packet.put_byte(mcap_manager_.getPoint() > 0x251Bu);

    uint16_t first = 0;
    uint8_t operation = 0;
    uint16_t second = 0;
    GenerateCleanPadVeriData(first, operation, second);
    packet.put_short(first);
    packet.put_byte(operation);
    packet.put_short(second);
    packet.put_byte(static_cast<unsigned char>(human_certify_error_count_));
    packet.put_byte(0);
    packet.finalize(true);
    Send(packet);
    clean_pad_request_tick_ =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurTickCount());
    return 1;
}

// IDA_EA: 0x86802B8
// IDA_NAME: _ZN5CUser24reqHumanCertify4ClearMapEb
// DEMANGLED: CUser::reqHumanCertify4ClearMap(bool)
int CUser::reqHumanCertify4ClearMap(bool probability_gate) {
    if (probability_gate &&
        mcap_manager_.getPoint() <= get_rand_int(10000)) {
        return 0;
    }

    if (mcap_manager_.getPoint() >= 100u) {
        const int reduced_rate = std::max(
            0, 100 - static_cast<int>(mcap_manager_.getPoint() / 100u));
        if (getStdDropRate() > reduced_rate &&
            ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                RestrictCategory::Security, 1)) {
            setStdDropRate(reduced_rate);
            DB_SaveUserHandicap::makeRequest(get_acc_id(),
                                             40001,
                                             101,
                                             getStdDropRate(),
                                             1,
                                             0);
        }
    }

    const int result = reqHumanCertify() ^ 1;
    if (result) {
        return result;
    }

    setHumanCertified(false);
    if (human_certify_timer_key_ > 0x7FFFFFFEu) {
        human_certify_timer_key_ = 0;
    }
    const unsigned int timer_key = ++human_certify_timer_key_;
    return G_TimerQueue()->InsertTimer(
        0, GetUID(), 9, 60, timer_key, -1);
}
