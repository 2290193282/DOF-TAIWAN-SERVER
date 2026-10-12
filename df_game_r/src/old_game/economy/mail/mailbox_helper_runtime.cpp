#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/config.h"

#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_data/db/mail/letter_stat_payload.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "old_game/economy/trade/trade_limit_item_abi.h"
#include "old_data/db/avatar_aura/db_avatar_change_owner.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>
#include <cstring>
#include <list>

namespace {

const size_t kMailboxPackageLackCount = 20u;

void MakeMailboxPackageItem(CUser* user,
                            const SIG_MAILBOX_MAIL_ENTRY& record,
                            uint32_t chronicle_value,
                            Inven_Item& package_item) {
    const int item_id = static_cast<int>(record.item_id);
    if (!item_id) {
        return;
    }

    if (item_id == 1) {
        package_item.item_id_ = item_id;
        package_item.add_info_ = record.add_info;
        package_item.avatar_select_ = record.endurance;
        return;
    }

    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) {
        return;
    }

    // CItem's old virtual slot +0x08 initializes a physical Inven_Item from
    // the PVF prototype. The 0627 mail payload restores the fields it owns,
    // including the GUID bytes omitted by Inven_Item::setCopy. The appended
    // 0828 DB signal field restores Chronicle value_ independently from the
    // legacy 14-byte random-option record.
    const CItemVTable* const vtable =
        *reinterpret_cast<const CItemVTable* const*>(item);
    vtable->make_item(item, package_item);

    package_item.seal_or_state_ = record.seal_flag;
    package_item.item_id_ = item_id;
    package_item.add_info_ = record.add_info;
    package_item.SetItemAttr(record.upgrade);
    package_item.avatar_select_ = record.endurance;
    package_item.extend_info_ = static_cast<int32_t>(record.extend_info);
    std::memcpy(&package_item.globally_unique_id_,
                record.item_guid,
                sizeof(package_item.globally_unique_id_));
    package_item.amplify_option_.assign(record.amplify_option,
                                        record.amplify_value);
    std::memcpy(&package_item.chronicle_random_option_.random_option_,
                &record.random_option,
                sizeof(package_item.chronicle_random_option_.random_option_));
    package_item.chronicle_random_option_.value_ = chronicle_value;
    package_item.upgrade_separate_.SetUpgradeSeparateAndTradeRestriction(
        record.separate_upgrade);

    if (record.type == 4u && IsTradeLimitAttachTypeItem(package_item)) {
        package_item.SetTradeLimitCount(static_cast<uint8_t>(
            package_item.GetTradeLimitCount() - 1));
        user->history_log().ChangeItemLimitCount(
            1, -1, package_item.item_id_, -1, 4);
    }
}

void PutMailboxLetter(PacketGuard* packet,
                      const SIG_MAILBOX_LIST* list,
                      const SIG_LETTER_INFO& letter) {
    packet->put_int(static_cast<int>(letter.letter_id));
    packet->put_int(letter.send_charac_no);
    packet->put_int(static_cast<int>(std::strlen(letter.send_charac_name)));
    packet->put_str(letter.send_charac_name,
                    static_cast<int>(std::strlen(letter.send_charac_name)));
    packet->put_int(static_cast<int>(std::strlen(letter.letter_text)));
    packet->put_str(letter.letter_text,
                    static_cast<int>(std::strlen(letter.letter_text)));
    packet->put_int(isUnlimitMail(list, static_cast<int>(letter.letter_id))
                        ? 0
                        : letter.reg_time);
    packet->put_short(letter.stat);
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x8554E90
// IDA_NAME: _ZN8WongWork14CMailBoxHelper16CheckMailTimeoutEP5CUserP11PacketGuard
// DEMANGLED: WongWork::CMailBoxHelper::CheckMailTimeout(CUser *,PacketGuard *)
int CMailBoxHelper::CheckMailTimeout(CUser* user, PacketGuard* packet) {
    CMailBox* mailbox = user->GetMailBox();
    if (!mailbox) {
        return 0;
    }

    // The old stack buffer is exactly one count plus the twenty entries
    // permitted by the typed inner StaticPool<CMail,20>.
    unsigned int postal_ids[21] = {};
    const long current_sec = GlobalData::s_systemTime_.getCurSec();

    for (std::map<unsigned int, CMailBox::CMail*>::iterator it =
             mailbox->mails.begin();
         it != mailbox->mails.end();) {
        CMailBox::CMail* mail = it->second;
        if (!mail->isUnlimit() && mail->IsTimeOver(current_sec)) {
            const unsigned int postal_id = it->first;
            packet->put_int(postal_id);
            postal_ids[postal_ids[0] + 1] = postal_id;

            std::map<unsigned int, CMailBox::CMail*>::iterator erase_it = it++;
            mailbox->mails.erase(erase_it);
            ++postal_ids[0];

            if (mail->GetLetterId()) {
                bool same_letter_exists = false;
                for (std::map<unsigned int, CMailBox::CMail*>::iterator check =
                         mailbox->mails.begin();
                     check != mailbox->mails.end();
                     ++check) {
                    if (check->second->GetLetterId() == mail->GetLetterId()) {
                        same_letter_exists = true;
                    }
                }
                if (!same_letter_exists) {
                    ++mailbox->loaded_letter_count;
                }
            }

            mail->ResetPackage();
            mailbox->mail_pool.Free(mail);
        } else {
            ++it;
        }
    }

    ReqDBRemoveMail(user, postal_ids, true);
    return static_cast<int>(postal_ids[0]);
}

// IDA_EA: 0x85552AC
// IDA_NAME: _ZN8WongWork14CMailBoxHelper13ReqDBLoadMailEP5CUserj
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBLoadMail(CUser *,unsigned int)
void CMailBoxHelper::ReqDBLoadMail(CUser* user, unsigned int charac_no) {
    CMailBox* mailbox = user->GetMailBox();
    if (!mailbox) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("MailBox.cpp", 1248);
    CStreamGuard guard(stream, true);
    guard << 127;
    guard << user->GetUID();

    // Three-byte private routing prefix already recognized by the restored
    // signal-127 owner.  The official SIG_MAILBOX_LIST ABI that follows is
    // unchanged and the prefix is never included in the response.
    guard << static_cast<uint16_t>(0x8247u);
    guard << static_cast<uint8_t>(user->GetServerGroup());

    SIG_MAILBOX_LIST* const payload =
        guard.GetInBuffer<SIG_MAILBOX_LIST>();
    std::memset(payload, 0, sizeof(*payload));
    payload->request.charac_no = static_cast<int32_t>(charac_no);
    payload->request.last_postal_id = mailbox->GetLastLoadIdx();
    if (mailbox->GetLoadedLetterCount() > 20) {
        mailbox->SetLoadedLetterCount(20);
    }
    payload->request.remain_count =
        mailbox->GetRemainSize() - mailbox->GetLoadedLetterCount();
    payload->request.last_letter_id =
        mailbox->GetLastLoadLetterIdx();
    payload->request.mail_load_count = mailbox->getMailLoadCount();
    mailbox->GetPackageLoadLack(
        payload->request.package_load_lack,
        0x14u);

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8555464
// IDA_NAME: _ZN8WongWork14CMailBoxHelper18ReqDBCheckReceiverEP5CUserPKcP23MSG_CHECK_MAIL_RECEIVER
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBCheckReceiver(CUser *,char const*,MSG_CHECK_MAIL_RECEIVER *)
int CMailBoxHelper::ReqDBCheckReceiver(
    CUser* user,
    const char* receiver,
    MSG_CHECK_MAIL_RECEIVER* message) {
    return ReqDBCheckReceiver_forchina(user, receiver, message);
}

// IDA_EA: 0x8555486
// IDA_NAME: _ZN8WongWork14CMailBoxHelper27ReqDBCheckReceiver_forchinaEP5CUserPKcP23MSG_CHECK_MAIL_RECEIVER
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBCheckReceiver_forchina(CUser *,char const*,MSG_CHECK_MAIL_RECEIVER *)
int CMailBoxHelper::ReqDBCheckReceiver_forchina(
    CUser* user,
    const char* receiver,
    MSG_CHECK_MAIL_RECEIVER* message) {
    const char* charac_name = user->getCurCharacName();
    if (std::strcmp(charac_name, receiver) == 0) {
        return 7;
    }

    Stream* stream = GlobalData::big_stream_pool()->Acquire("MailBox.cpp",
                                                            1326);
    CStreamGuard guard(stream, true);
    guard << 126;
    // IDA 0x8555486 writes the full int UID.  This must match both
    // DBTask::DBProcess and DB_MailBox_Req_CheckReceiver's return frame.
    guard << user->GetUID();

    SIG_MAILBOX_CHECK_RECEIVER* const payload =
        guard.GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>();
    std::strncpy(payload->receiver_name,
                 receiver,
                 sizeof(payload->receiver_name));
    payload->message = *message;
    payload->sender_charac_no = user->get_charac_no(-1);

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x85555E8
// IDA_NAME: _ZN8WongWork14CMailBoxHelper22ReqDBSendNewSystemMailEPKcRK10Inven_ItemjjS2_ij17ENUM_SERVER_GROUPbb
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(char const*,Inven_Item const&,unsigned int,unsigned int,char const*,int,unsigned int,ENUM_SERVER_GROUP,bool,bool)
int CMailBoxHelper::ReqDBSendNewSystemMail(
    const char* title,
    const Inven_Item& item,
    unsigned int money,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    ENUM_SERVER_GROUP server_group,
    bool unlimit,
    bool db_work) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return -1;
    }

    Stream* stream = pool->Acquire("MailBox.cpp", 1359);
    if (!stream) {
        return -1;
    }
    CStreamGuard guard(stream, true);
    guard << 128;
    guard << -1;

    SIG_MAILBOX_SEND_NEW_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    std::memset(payload, 0, sizeof(*payload));
    _makeSendMailData(payload,
                      title,
                      0,
                      item,
                      money,
                      charac_no,
                      text,
                      text_len,
                      occ_time,
                      0);
    payload->request.server_group = server_group;
    payload->request.unlimit_flag = unlimit ? 1 : 0;
    payload->request.db_work = db_work ? 1 : 0;

    // IDA_EVIDENCE: the legacy helper ignores DB_Q::put and always returns
    // zero.  Keep that ABI for its many existing callers; only the additive
    // tracked helper below exposes submission failure to its explicit 0828
    // persistence consumers (CMD723, CMD725 and decide-level signal 844).
    (void)queues->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

int CMailBoxHelper::ReqDBSendNewSystemMailTracked(
    const char* title,
    const Inven_Item& item,
    unsigned int money,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    ENUM_SERVER_GROUP server_group,
    bool unlimit,
    bool db_work,
    const SIG_MAILBOX_PERSISTENCE_CONTEXT& persistence) {
    InvenItemMailRecord record;
    std::memset(&record, 0, sizeof(record));
    item.ExportMailRecord(record);
    return ReqDBSendNewSystemMailTrackedRecord(
        title, record, money, charac_no, text, text_len, occ_time,
        server_group, unlimit, db_work, persistence);
}

int CMailBoxHelper::ReqDBSendNewSystemMailTrackedRecord(
    const char* title,
    const InvenItemMailRecord& item,
    unsigned int money,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    ENUM_SERVER_GROUP server_group,
    bool unlimit,
    bool db_work,
    const SIG_MAILBOX_PERSISTENCE_CONTEXT& persistence) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    const bool known_consumer =
        persistence.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP ||
        persistence.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE ||
        persistence.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP;
    if (!pool || !queues ||
        !MailboxPersistenceCorrelationValid(persistence) ||
        !title || text_len < 0 || (text_len != 0 && !text) ||
        charac_no == 0u || item.item_id == 0u || !known_consumer) {
        return -1;
    }

    Stream* const stream = pool->Acquire("MailBox.cpp", 1407);
    if (!stream) {
        return -1;
    }
    CStreamGuard guard(stream, true);
    guard << 128;
    guard << -1;

    SIG_MAILBOX_SEND_NEW_MAIL_TRACKED* const tracked =
        reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL_TRACKED*>(
            guard.GetInBufferUntyped(
                static_cast<int>(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED))));
    if (!tracked) {
        return -1;
    }
    std::memset(tracked, 0, sizeof(*tracked));
    SIG_MAILBOX_SEND_NEW_MAIL::Request& request = tracked->mail.request;
    std::strncpy(request.sender_charac_name, title,
                 sizeof(request.sender_charac_name) - 1u);
    request.sender_charac_no = 0;
    request.receiver_charac_no = static_cast<int32_t>(charac_no);
    request.gold = static_cast<int32_t>(money);
    request.seal_flag = item.seal_flag;
    request.item_id = item.item_id;
    request.add_info = item.add_info;
    request.upgrade = item.item_attr;
    request.endurance = item.endurance;
    request.extend_info = item.extend_info;
    std::memcpy(request.item_guid, item.item_guid,
                sizeof(request.item_guid));
    request.amplify_option = item.amplify_option;
    request.amplify_value = item.amplify_value;
    std::memcpy(request.random_option, item.random_option,
                sizeof(request.random_option));
    request.separate_upgrade = item.separate_upgrade;
    request.avatar_flag = item.avatar_flag ? 1 : 0;
    request.creature_flag = item.creature_flag ? 1 : 0;
    request.letter_text_length = 0u;
    if (text_len != 0) {
        const size_t available = std::strlen(text);
        size_t requested = static_cast<size_t>(text_len);
        if (requested > available) requested = available;
        if (requested > 255u) requested = 255u;
        request.letter_text_length = static_cast<uint16_t>(requested);
        std::memcpy(request.letter_text, text, requested);
    }
    request.occ_time = static_cast<int32_t>(occ_time);
    request.server_group = server_group;
    request.old_letter_id = 0;
    request.db_work = db_work ? 1 : 0;
    request.unlimit_flag = unlimit ? 1 : 0;
    request.chronicle_value = item.chronicle_value;

    // Bind the acknowledgement to what was actually serialized, rather than
    // trusting duplicate identity fields supplied by the feature caller.
    tracked->context = persistence;
    tracked->context.magic = MAILBOX_PERSISTENCE_CONTEXT_MAGIC;
    tracked->context.version = MAILBOX_PERSISTENCE_CONTEXT_VERSION;
    tracked->context.receiver_charac_no = charac_no;
    tracked->context.reward_item_id =
        static_cast<uint32_t>(request.item_id);
    tracked->context.server_group = static_cast<int32_t>(server_group);

    return queues->put(MsgQueueMgr::DB_Q, guard) == 1 ? 0 : -1;
}

// IDA_EA: 0x8555916
// IDA_NAME: _ZN8WongWork14CMailBoxHelper14_DoSendNewMailEP5CUserRK10Inven_ItemjjbPKcii
// DEMANGLED: WongWork::CMailBoxHelper::_DoSendNewMail(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int,int)
int CMailBoxHelper::_DoSendNewMail(CUser* user,
                                   const Inven_Item& item,
                                   unsigned int money,
                                   unsigned int charac_no,
                                   bool send_now,
                                   const char* text,
                                   int text_len,
                                   int unknown) {
    Stream* stream = GlobalData::stream_pool()->Acquire("MailBox.cpp", 1435);
    CStreamGuard guard(stream, false);
    guard << 128;
    // Signal 128 is not a byte-UID exception in DBTask.
    guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    std::memset(payload, 0, sizeof(*payload));

    const int sender_charac_no = user->getCurCharacNo();
    const char* sender_name = user->getCurCharacName();
    _makeSendMailData(payload,
                      sender_name,
                      sender_charac_no,
                      item,
                      money,
                      charac_no,
                      text,
                      text_len,
                      0,
                      unknown);
    payload->request.server_group = user->GetServerGroup();

    if (send_now) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        CMailBox* mailbox = user->GetMailBox();
        Stream* stored_stream = guard.Get();
        mailbox->pending_streams.push_back(stored_stream);
    }
    return 0;
}

// IDA_EA: 0x8555AC8
// IDA_NAME: _ZN8WongWork14CMailBoxHelper16ReqDBSendNewMailEP5CUserRK10Inven_ItemjjbPKci
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMail(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int)
int CMailBoxHelper::ReqDBSendNewMail(CUser* user,
                                     const Inven_Item& item,
                                     unsigned int money,
                                     unsigned int charac_no,
                                     bool send_now,
                                     const char* text,
                                     int text_len) {
    return _DoSendNewMail(user,
                          item,
                          money,
                          charac_no,
                          send_now,
                          text,
                          text_len,
                          0);
}

// IDA_EA: 0x8555B14
// IDA_NAME: _ZN8WongWork14CMailBoxHelper16ReqDBSendNewMailEP5CUserP16MSG_MAILBOX_SENDjbji
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMail(CUser *,MSG_MAILBOX_SEND *,unsigned int,bool,unsigned int,int)
int CMailBoxHelper::ReqDBSendNewMail(CUser* user,
                                     MSG_MAILBOX_SEND* message,
                                     unsigned int receiver_charac_no,
                                     bool send_now,
                                     unsigned int receiver_account_id,
                                     int multi_mail_text_length) {
    if (!user || !message) {
        return 1;
    }
    if (message->item_add_info < 0) {
        return 17;
    }

    int postage = message->item_id ? 1000 : 100;
    if (message->money) {
        postage += static_cast<int>(message->money * 5u / 100u);
        const int postage_limit = message->item_id ? 11000 : 10100;
        if (postage > postage_limit) {
            postage = postage_limit;
        }
    }
    if (user->getCurCharacMoney() <
        static_cast<int>(message->money + static_cast<unsigned int>(postage))) {
        return 10;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        return 17;
    }

    int inventory_type = 6;
    Inven_Item source_item;
    Inven_Item mail_item;
    if (message->item_id) {
        switch (message->inventory_type) {
            case 0:
                inventory_type = 1;
                break;
            case 1:
                inventory_type = 2;
                break;
            case 7:
                inventory_type = 3;
                break;
            default:
                return 23;
        }

        source_item = inventory->GetInvenSlot(inventory_type,
                                               message->inventory_slot);
        if (static_cast<uint32_t>(source_item.item_id_) != message->item_id) {
            return 17;
        }
        if (source_item.item_attr_ == 4u) {
            return 23;
        }
        if (!source_item.isEquipableItemType() &&
            source_item.get_add_info() < message->item_add_info) {
            return 17;
        }

        CItem* const item = G_CDataManager()->find_item(source_item.item_id_);
        if (!item) {
            return 1;
        }
        if (!item->isPackagable()) {
            return 23;
        }

        switch (item->GetAttachType()) {
            case 1:
            case 2:
                return 23;
            case 3:
                if (source_item.seal_or_state_ != 1u) {
                    return 23;
                }
                break;
            case 7:
                if (!DecreaseTradeLimitCount(source_item)) {
                    return 23;
                }
                user->history_log().ChangeItemLimitCount(
                    inventory_type,
                    message->inventory_slot,
                    source_item.item_id_,
                    -1,
                    3);
                break;
            default:
                break;
        }
        if (source_item.upgrade_separate_.IsTradeRestriction()) {
            return 23;
        }

        mail_item.setCopy(source_item);
        if (!mail_item.isEquipableItemType()) {
            mail_item.set_add_info(message->item_add_info);
        }

        if (item->vtable_ && item->vtable_->is_avatar_item &&
            item->vtable_->is_avatar_item(item)) {
            const int avatar_key = source_item.get_add_info();
            if (CAvatarItemMgr::IsTempKey(avatar_key)) {
                return 17;
            }
            const CAvatarItemMgr* const avatar_manager =
                inventory->GetAvatarItemMgrR();
            if (!avatar_manager || avatar_manager->GetExpireDate(avatar_key)) {
                return 17;
            }
        }
    }

    if (inventory->use_money(postage,
                             static_cast<eMoneySubReason>(16),
                             true) != 1) {
        return 10;
    }
    if (message->money) {
        Secu_GoldControl* const gold_control =
            static_cast<Secu_GoldControl*>(user->GetGoldControl());
        if (gold_control) {
            gold_control->SetMailCharName(message->receiver_name);
        }
        if (inventory->use_money(static_cast<int>(message->money),
                                 static_cast<eMoneySubReason>(17),
                                 true) != 1) {
            return 10;
        }
    }

    if (message->item_id) {
        if (source_item.item_category_ == 8u) {
            if (inventory->delete_item(static_cast<INVEN_TYPE>(2),
                                       message->inventory_slot,
                                       1,
                                       static_cast<eItemDelReason>(2),
                                       true) != 1) {
                return 17;
            }
            const int avatar_key = source_item.get_add_info();
            inventory->SendAvatarEvent(54,
                                       avatar_key,
                                       2,
                                       static_cast<int>(message->inventory_slot) +
                                           oldsrv::config::
                                               kAvatarInventorySlotBase,
                                       0);
            DB_AvatarChangeOwner::makeRequest(
                static_cast<unsigned int>(avatar_key),
                static_cast<unsigned int>(user->getCurCharacNo()),
                receiver_charac_no,
                0);
            CAvatarItemMgr* const avatar_manager =
                inventory->GetAvatarItemMgrW();
            if (avatar_manager) {
                avatar_manager->UnRegistItem(avatar_key);
            }
        } else if (inventory->delete_item(
                       static_cast<INVEN_TYPE>(inventory_type),
                       message->inventory_slot,
                       message->item_add_info,
                       static_cast<eItemDelReason>(15),
                       true) != 1) {
            return 17;
        }

        if (source_item.IsCreatureItemType()) {
            user_creature::CCreatureMgr* const creature_manager =
                inventory->GetCreatureMgrW();
            if (creature_manager) {
                creature_manager->SendMail(&source_item,
                                           message->inventory_slot,
                                           15);
            }
        }
    }

    user->history_log().SendMail(message, receiver_account_id);
    return _DoSendNewMail(user,
                          mail_item,
                          message->money,
                          receiver_charac_no,
                          send_now,
                          message->contents,
                          static_cast<int>(message->contents_length),
                          multi_mail_text_length);
}

// IDA_EA: 0x8556A14
// IDA_NAME: _ZN8WongWork14CMailBoxHelper25MakeSystemMultiMailPostalERSt6vectorISt4pairIiiESaIS3_EEP10Inven_Itemj
// DEMANGLED: WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,Inven_Item *,unsigned int)
int CMailBoxHelper::MakeSystemMultiMailPostal(
    std::vector<std::pair<int, int> >& rewards,
    Inven_Item* items,
    unsigned int max_count) {
    if (rewards.size() > 0x0au || rewards.empty()) {
        return 0;
    }

    unsigned int item_index = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             rewards.begin();
         it != rewards.end() && item_index < max_count;
         ++it) {
        Inven_Item item;
        item.reset();
        item.item_id_ = it->first;
        const int item_count = it->second;
        CDataManager* data_manager = G_CDataManager();
        CItemList* item_list = data_manager->item_list_;
        item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT,
                               item,
                               item_count);
        items[item_index].setCopy(item);
        ++item_index;
    }

    return 1;
}

// IDA_EA: 0x85566AE
// IDA_NAME: _ZN8WongWork14CMailBoxHelper19ReqDBSendStoredMailEP5CUser
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendStoredMail(CUser *)
int CMailBoxHelper::ReqDBSendStoredMail(CUser* user) {
    CMailBox* mailbox = user->GetMailBox();
    if (!mailbox) {
        return 0;
    }

    if (mailbox->pending_streams.empty()) {
        return 1;
    }

    for (std::list<Stream*>::iterator it = mailbox->pending_streams.begin();
         it != mailbox->pending_streams.end();
         ++it) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, *it);
    }
    mailbox->pending_streams.clear();
    return 0;
}

// IDA_EA: 0x8556772
// IDA_NAME: _ZN8WongWork14CMailBoxHelper15ReqDBRemoveMailEPK5CUserPjb
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBRemoveMail(CUser const*,unsigned int *,bool)
void CMailBoxHelper::ReqDBRemoveMail(const CUser* user,
                                     unsigned int* postal_ids,
                                     bool permanent_delete) {
    Stream* stream = GlobalData::stream_pool()->Acquire("MailBox.cpp", 2254);
    CStreamGuard guard(stream, true);
    guard << 129;
    guard << user->GetUID();

    unsigned int* payload = reinterpret_cast<unsigned int*>(
        guard.GetInBuffer<SIG_MAILBOX_DEL_MAIL>());
    payload[0] = postal_ids[0];
    for (unsigned int index = 1; index <= postal_ids[0]; ++index) {
        payload[index] = postal_ids[index];
    }
    payload[21] = permanent_delete ? 2u : 1u;

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x85568A6
// IDA_NAME: _ZN8WongWork14CMailBoxHelper19ReqChangeLetterStatEP5CUserii
// DEMANGLED: WongWork::CMailBoxHelper::ReqChangeLetterStat(CUser *,int,int)
int CMailBoxHelper::ReqChangeLetterStat(CUser* user,
                                        int letter_id,
                                        int stat) {
    switch (stat) {
        case 2:
        case 0:
            break;
        case 3: {
            CMailBox* mailbox = user->GetMailBox();
            if (!mailbox) {
                return 21;
            }
            if (mailbox->GetLetterKeepCount() + 1 > 10) {
                return 22;
            }
            break;
        }
        default:
            return 19;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("MailBox.cpp", 2288);
    CStreamGuard guard(stream, true);
    guard << 194;
    guard << user->GetUID();

    SIG_LETTER_STAT* const payload =
        guard.GetInBuffer<SIG_LETTER_STAT>();
    payload->charac_no = user->getCurCharacNo();
    payload->letter_id = letter_id;
    payload->letter_stat = static_cast<int16_t>(stat);
    payload->reserved_0a[0] = 0;
    payload->reserved_0a[1] = 0;

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x8556D5C
// IDA_NAME: _ZN8WongWork14CMailBoxHelper24ReqDBSendNewMailCashShopEP5CUserRK10Inven_ItemjjbPKci
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int)
int CMailBoxHelper::ReqDBSendNewMailCashShop(CUser* user,
                                             const Inven_Item& item,
                                             unsigned int money,
                                             unsigned int charac_no,
                                             bool send_now,
                                             const char* text,
                                             int text_len) {
    if (G_GameWorld()->GetChannelType() == 7) {
        return 0;
    }

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("MailBox.cpp", 2397), true);
    guard << 128;
    // Signal 128 is not a byte-UID exception in DBTask.
    guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    _makeSendMailData(payload,
                      user->getCurCharacName(),
                      user->getCurCharacNo(),
                      item,
                      money,
                      charac_no,
                      text,
                      text_len,
                      0,
                      0);
    payload->request.server_group = 0;
    payload->request.unlimit_flag = 1;

    if (send_now) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        Stream* const stream = guard.Get();
        user->GetMailBox()->pending_streams.push_back(stream);
    }
    return 0;
}

// IDA_EA: 0x85523E2
// IDA_NAME: _ZN8WongWork14CMailBoxHelper19SetPeriodCheckTimerEP5CUser
// DEMANGLED: WongWork::CMailBoxHelper::SetPeriodCheckTimer(CUser *)
int CMailBoxHelper::SetPeriodCheckTimer(CUser* user) {
    CMailBox* const mailbox = user->GetMailBox();
    if (!mailbox) {
        return 0;
    }

    const int current_sec = GlobalData::s_systemTime_.getCurSec();
    const int fallback_period = current_sec + 1382400;
    int earliest_period = fallback_period;
    for (std::map<unsigned int, CMailBox::CMail*>::const_iterator it =
             mailbox->mails.begin();
         it != mailbox->mails.end();
         ++it) {
        CMailBox::CMail* const mail = it->second;
        if (!mail->isUnlimit()) {
            earliest_period = std::min(
                earliest_period, static_cast<int>(mail->GetPeriod()));
        }
    }

    if (earliest_period != fallback_period) {
        return G_TimerQueue()->InsertTimer(
            0,
            user->GetUID(),
            8,
            user->GetLoginTick(),
            static_cast<unsigned int>(user->getCurCharacNo()),
            0);
    }
    return earliest_period;
}

// IDA_EA: 0x8555118
// IDA_NAME: _ZN8WongWork14CMailBoxHelper21MakeMailBoxListPacketEP5CUserP11PacketGuard
// DEMANGLED: WongWork::CMailBoxHelper::MakeMailBoxListPacket(CUser *,PacketGuard *)
CMailBox* CMailBoxHelper::MakeMailBoxListPacket(CUser* user,
                                                PacketGuard* packet) {
    CMailBox* const mailbox = user->GetMailBox();
    if (!mailbox) {
        return 0;
    }

    const int current_sec = GlobalData::s_systemTime_.getCurSec();
    packet->put_header(0, ENUM_NOTIPACKET_MAILBOX_MAIL_LIST);
    int mail_count_index = packet->get_index();
    int mail_count = 0;
    packet->put_byte(mailbox->GetRecvSize());
    packet->put_byte(0);

    for (std::map<unsigned int, CMailBox::CMail*>::const_iterator it =
             mailbox->mails.begin();
         it != mailbox->mails.end();
         ++it) {
        CMailBox::CMail* const mail = it->second;
        if (!mailbox->FindPackageLoadLack(
                static_cast<unsigned int>(mail->GetLetterId()))) {
            ++mail_count;
            mail->MakePacket(packet, current_sec, user);
        }
    }

    packet->put_byte(mail_count_index, mail_count);
    // The declared old return type is CMailBox*, but 0x8555118 returns the
    // final InterfacePacketBuf::put_short EAX value verbatim.  Its known
    // caller ignores it; preserve that ABI-visible return rather than making
    // up a convenient mailbox return value.
    return reinterpret_cast<CMailBox*>(
        static_cast<uintptr_t>(
            packet->put_short(mailbox->GetNotLoadedMailCount())));
}

// IDA_EA: 0x85525A6
// IDA_NAME: _ZN8WongWork14CMailBoxHelper14SetMailBoxInfoEP5CUserPK16SIG_MAILBOX_LIST
// DEMANGLED: WongWork::CMailBoxHelper::SetMailBoxInfo(CUser *,SIG_MAILBOX_LIST const*)
int CMailBoxHelper::SetMailBoxInfo(CUser* user, const SIG_MAILBOX_LIST* list) {
    CMailBox* const mailbox = user->GetMailBox();
    if (!mailbox) {
        return 0;
    }

    mailbox->incMailLoadCount();
    const int was_unloaded = mailbox->IsLoaded() ^ 1;
    PacketGuard packet;
    const int current_sec = GlobalData::s_systemTime_.getCurSec();

    std::set<unsigned int> changed_package_ids;
    mailbox->SetPackageLoadLack(list->response.changed_package_ids,
                                static_cast<int>(kMailboxPackageLackCount),
                                changed_package_ids);

    int mail_update_count = 0;
    int mail_update_count_index = 0;
    const int mail_count = list->response.mail_count;
    if (was_unloaded != 1) {
        packet.put_header(0, ENUM_NOTIPACKET_MAILBOX_MAIL_LIST);
        mail_update_count_index = packet.get_index();
        packet.put_byte(mail_count);
        packet.put_byte(1);

        for (std::map<unsigned int, CMailBox::CMail*>::const_iterator it =
                 mailbox->mails.begin();
             it != mailbox->mails.end();
             ++it) {
            CMailBox::CMail* const mail = it->second;
            if (changed_package_ids.find(
                    static_cast<unsigned int>(mail->GetLetterId())) !=
                changed_package_ids.end()) {
                mail->MakePacket(&packet, current_sec, user);
                ++mail_update_count;
            }
        }
    }

    const int avatar_count = list->response.avatar_count;
    for (int index = 0; avatar_count > index; ++index) {
        const SIG_MAILBOX_AVATAR_ENTRY& avatar =
            list->response.avatars[index];
        stAvatarEmblemInfo_t emblem;
        stAvatarExpansionInfo_t expansion;
        for (size_t socket = 0; socket != 5u; ++socket) {
            emblem.sockets[socket].socket_type =
                avatar.emblem_info.sockets[socket].socket_type;
            emblem.sockets[socket].emblem_item_index =
                avatar.emblem_info.sockets[socket].emblem_item_index;
        }
        expansion.avatar_expansion[0] = avatar.avatar_expansion[0];
        expansion.avatar_expansion[1] = avatar.avatar_expansion[1];

        CInventory* const inventory = user->getCurCharacInvenW();
        CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrW();
        avatar_manager->RegistItem(
            avatar.ui_id,
            avatar.expire_time,
            static_cast<int>(avatar.reg_time),
            avatar.ipg_agency_no,
            emblem,
            expansion,
            false);
    }

    const int creature_count = list->response.creature_count;
    for (int index = 0; creature_count > index; ++index) {
        const SIG_MAILBOX_CREATURE_ENTRY& creature =
            list->response.creatures[index];
        CInventory* const inventory = user->getCurCharacInvenW();
        user_creature::CCreatureMgr* const creature_manager =
            inventory->GetCreatureMgrW();
        creature_manager->RegistTempPeriod(creature.ui_id,
                                           creature.expire_time);
    }

    mailbox->SetLoadState(true,
                          static_cast<int>(list->response.last_postal_id));
    for (int index = 0; mail_count > index; ++index) {
        const SIG_MAILBOX_MAIL_ENTRY& mail = list->response.mails[index];
        Inven_Item package_item;
        MakeMailboxPackageItem(user,
                               mail,
                               list->response.chronicle_values[index],
                               package_item);

        int package_type = 0;
        if (mail.avatar_flag) {
            package_type = 1;
        } else if (mail.creature_flag) {
            package_type = 2;
        }

        CMailBox::stAddNewMailInput input;
        input.mail_idx = mail.postal_id;
        input.sender_charac_no = mail.send_charac_no;
        input.package_money = mail.gold;
        input.period = static_cast<int>(mail.letter_id);
        input.letter_id = mail.type;
        input.package_type = package_type;
        input.package_period = mail.expire_time;
        input.unlimit = mail.unlimit_flag;
        input.sender_name = mail.send_charac_name;
        input.package_item = &package_item;

        CMailBox::CMail* const new_mail = mailbox->AddNewMail(input);
        if (new_mail && !mailbox->FindPackageLoadLack(
                            static_cast<unsigned int>(input.period))) {
            ++mail_update_count;
            if (was_unloaded != 1) {
                new_mail->MakePacket(&packet, current_sec, user);
            }
        }
    }

    mailbox->SetNotLoadedMailCount(list->response.not_loaded_mail_count);
    if (was_unloaded) {
        MakeMailBoxListPacket(user, &packet);
        if (mail_count > 0) {
            SetPeriodCheckTimer(user);
        }
        mailbox->ClearLetterKeepCount();
        mailbox->SetLoadedLetterCount(0);
    } else {
        packet.put_byte(mail_update_count_index, mail_update_count);
        packet.put_short(mailbox->GetNotLoadedMailCount());
    }

    mailbox->SetLoadedLetterCount(list->response.loaded_letter_adjusted +
                                  mailbox->GetLoadedLetterCount());

    int letter_output_count = 0;
    int letter_count_index = packet.get_index();
    const int letter_count = list->response.letter_count;
    packet.put_short(letter_count);

    for (std::set<unsigned int>::const_iterator changed =
             changed_package_ids.begin();
         changed != changed_package_ids.end();
         ++changed) {
        std::map<unsigned int, SIG_LETTER_INFO>::iterator found =
            mailbox->letter_infos_map.find(*changed);
        if (found != mailbox->letter_infos_map.end()) {
            PutMailboxLetter(&packet, list, found->second);
            ++letter_output_count;
            mailbox->letter_infos_map.erase(found);
        }
    }

    for (int index = 0; letter_count > index; ++index) {
        const SIG_LETTER_INFO& letter = list->response.letters[index];

        if (mailbox->FindPackageLoadLack(letter.letter_id)) {
            mailbox->letter_infos_map.insert(
                std::make_pair(letter.letter_id, letter));
        } else {
            PutMailboxLetter(&packet, list, letter);
            ++letter_output_count;
        }

        mailbox->SetLastLoadLetterIdx(std::max(
            static_cast<unsigned int>(mailbox->GetLastLoadLetterIdx()),
            letter.letter_id));
        if (letter.stat == 3) {
            mailbox->InsertLetterKeepCount(letter.letter_id);
        }
    }

    packet.put_short(letter_count_index, letter_output_count);
    packet.finalize(true);
    user->Send(packet);

    if (was_unloaded != 1) {
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_MAILBOX_OPEN);
        packet.put_byte(1);
        packet.put_short(mailbox->GetNotLoadedMailCount());
        packet.finalize(true);
        user->Send(packet);
    }
    return 1;
}

}  // namespace WongWork
