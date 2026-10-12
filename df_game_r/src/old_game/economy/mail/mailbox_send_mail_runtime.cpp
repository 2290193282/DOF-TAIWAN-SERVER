#include "old_game/economy/mail/mail_trade_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace WongWork {

// IDA_EA: 0x8555756
// IDA_NAME: _ZN8WongWork14CMailBoxHelper17_makeSendMailDataEP25SIG_MAILBOX_SEND_NEW_MAILPKciRK10Inven_ItemjjS4_iji
// DEMANGLED: WongWork::CMailBoxHelper::_makeSendMailData(SIG_MAILBOX_SEND_NEW_MAIL *,char const*,int,Inven_Item const&,unsigned int,unsigned int,char const*,int,unsigned int,int)
SIG_MAILBOX_SEND_NEW_MAIL* CMailBoxHelper::_makeSendMailData(
    SIG_MAILBOX_SEND_NEW_MAIL* payload,
    const char* title,
    int sender_charac_no,
    const Inven_Item& item,
    unsigned int money,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    int unknown) {
    SIG_MAILBOX_SEND_NEW_MAIL::Request& request = payload->request;
    InvenItemMailRecord mail_item;
    item.ExportMailRecord(mail_item);

    std::strncpy(request.sender_charac_name,
                 title,
                 sizeof(request.sender_charac_name) - 1u);
    request.sender_charac_no = sender_charac_no;
    request.receiver_charac_no = static_cast<int32_t>(charac_no);
    request.gold = static_cast<int32_t>(money);
    request.seal_flag = mail_item.seal_flag;
    request.item_id = mail_item.item_id;
    request.add_info = mail_item.add_info;
    request.upgrade = mail_item.item_attr;
    request.endurance = mail_item.endurance;
    request.extend_info = mail_item.extend_info;
    std::memcpy(request.item_guid,
                mail_item.item_guid,
                sizeof(request.item_guid));
    request.amplify_option = mail_item.amplify_option;
    request.amplify_value = mail_item.amplify_value;
    request.chronicle_value = mail_item.chronicle_value;
    std::memcpy(request.random_option,
                mail_item.random_option,
                sizeof(request.random_option));
    request.separate_upgrade = mail_item.separate_upgrade;
    request.avatar_flag = mail_item.avatar_flag ? 1 : 0;
    request.creature_flag = mail_item.creature_flag ? 1 : 0;
    request.letter_text_length = 0;
    std::memset(request.letter_text, 0, sizeof(request.letter_text));
    if (text && text_len) {
        const uint16_t requested = static_cast<uint16_t>(text_len);
        const uint16_t capped = requested < 255u ? requested : 255u;
        request.letter_text_length = capped;
        std::strncpy(request.letter_text, text, capped);
    }
    request.occ_time = static_cast<int32_t>(occ_time);
    request.old_letter_id = unknown;
    request.db_work = 0;
    return payload;
}

// IDA_EA: 0x8556422
// IDA_NAME: _ZN8WongWork14CMailBoxHelper24ReqDBSendNewCreatureMailEP5CUseriRK10Inven_ItemPKci
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewCreatureMail(CUser *,int,Inven_Item const&,char const*,int)
void CMailBoxHelper::ReqDBSendNewCreatureMail(CUser* user,
                                              int receiver_charac_no,
                                              const Inven_Item& item,
                                              const char* text,
                                              int text_length) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("MailBox.cpp", 2111), true);
    *guard << 197;
    *guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();
    payload->package_type = 1;
    std::strncpy(payload->sender_charac_name,
                 user->getCurCharacName(),
                 sizeof(payload->sender_charac_name) - 1u);
    payload->receiver_charac_no = receiver_charac_no;
    payload->sender_charac_no = user->getCurCharacNo();
    std::memset(&payload->package, 0, sizeof(payload->package));

    Inven_Item_Compatible compatible;
    std::memset(&compatible, 0, sizeof(compatible));
    compatible.setCopy(item);
    std::memcpy(&payload->package.creature.inven_item_copy,
                &compatible,
                sizeof(compatible));
    payload->package.creature.detail.slot = 0;
    payload->package.creature.detail.charac_no =
        static_cast<uint32_t>(receiver_charac_no);
    payload->package.creature.detail.stomach = 100;
    payload->package.creature.detail.creature_flag = 32;
    payload->package.creature.detail.state = 1;
    payload->package.creature.current_creature_flag = 0;

    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(payload->package.creature.inven_item_copy.item_id));
    if (!prototype || prototype->is_stackable()) {
        return;
    }

    const CEquipItem* const equipment = static_cast<const CEquipItem*>(
        prototype);
    payload->package.creature.detail.creature_type =
        equipment->get_sub_type() ? 0 : 1;
    payload->letter_text_length = 0;
    std::memset(payload->letter_text, 0, sizeof(payload->letter_text));
    if (text && text_length) {
        payload->letter_text_length =
            static_cast<uint16_t>(text_length);
        std::strncpy(payload->letter_text,
                     text,
                     payload->letter_text_length);
    }
    payload->package.creature.detail.expiration_value =
        prototype->getUsablePeriod();
    payload->package.creature.detail.delete_tick =
        prototype->getExpirationDate();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x85571A0
// IDA_NAME: _ZN8WongWork14CMailBoxHelper32ReqDBSendNewCreatureMailCashShopEP5CUseriRK10Inven_ItemPKci
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewCreatureMailCashShop(CUser *,int,Inven_Item const&,char const*,int)
void CMailBoxHelper::ReqDBSendNewCreatureMailCashShop(
    CUser* user,
    int receiver_charac_no,
    const Inven_Item& item,
    const char* text,
    int text_length) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("MailBox.cpp", 2460), true);
    *guard << 197;
    *guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();
    payload->package_type = 1;
    std::strncpy(payload->sender_charac_name,
                 user->getCurCharacName(),
                 sizeof(payload->sender_charac_name) - 1u);
    payload->receiver_charac_no = receiver_charac_no;
    payload->sender_charac_no = user->getCurCharacNo();
    std::memset(&payload->package, 0, sizeof(payload->package));

    Inven_Item_Compatible compatible;
    std::memset(&compatible, 0, sizeof(compatible));
    compatible.setCopy(item);
    std::memcpy(&payload->package.creature.inven_item_copy,
                &compatible,
                sizeof(compatible));
    payload->package.creature.detail.slot = 0;
    payload->package.creature.detail.charac_no =
        static_cast<uint32_t>(receiver_charac_no);
    payload->package.creature.detail.stomach = 100;
    payload->package.creature.detail.creature_flag = 32;
    payload->package.creature.detail.state = 1;
    payload->package.creature.current_creature_flag = 0;
    payload->unlimit_flag = 1;

    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(payload->package.creature.inven_item_copy.item_id));
    if (!prototype || prototype->is_stackable()) {
        return;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(prototype);
    payload->package.creature.detail.creature_type =
        equipment->get_sub_type() ? 0 : 1;
    payload->letter_text_length = 0;
    std::memset(payload->letter_text, 0, sizeof(payload->letter_text));
    if (text && text_length) {
        payload->letter_text_length =
            static_cast<uint16_t>(text_length);
        std::strncpy(payload->letter_text,
                     text,
                     payload->letter_text_length);
    }
    payload->package.creature.detail.expiration_value =
        prototype->getUsablePeriod();
    payload->package.creature.detail.delete_tick =
        prototype->getExpirationDate();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

}  // namespace WongWork
