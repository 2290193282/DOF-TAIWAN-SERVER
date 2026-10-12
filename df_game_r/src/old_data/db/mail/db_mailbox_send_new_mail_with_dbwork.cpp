#include "old_data/db/mail/db_mailbox_send_new_mail_with_dbwork.h"

#include <cstring>

#include "old_data/db/mail/mailbox_payload.h"
#include "common/manager/db_common_public_abi.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/avatar_aura/db_avatar_item_insert.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/creature/db_insert_creature_item.h"
#include "old_data/db/mail/db_mailbox_req_send.h"
#include "infra/network/packet/mailbox_dbwork_stream.h"

// IDA_EA: 0x844E676
// IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkC2Ev
// DEMANGLED: DB_MailBoxSendNewMailWithDBWork::DB_MailBoxSendNewMailWithDBWork(void)
DB_MailBoxSendNewMailWithDBWork::DB_MailBoxSendNewMailWithDBWork() {}

// IDA_EA: 0x845EA76
// IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkD2Ev
// DEMANGLED: DB_MailBoxSendNewMailWithDBWork::~DB_MailBoxSendNewMailWithDBWork()
// IDA_EA: 0x845EAA6
// IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkD0Ev
// DEMANGLED: DB_MailBoxSendNewMailWithDBWork::~DB_MailBoxSendNewMailWithDBWork()
DB_MailBoxSendNewMailWithDBWork::~DB_MailBoxSendNewMailWithDBWork() {}

// IDA_EA: 0x84272DE
// IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWork8dispatchEiiP6Stream
// DEMANGLED: DB_MailBoxSendNewMailWithDBWork::dispatch(int,int,Stream *)
bool DB_MailBoxSendNewMailWithDBWork::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    try {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const payload =
        stream->GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();

    SIG_MAILBOX_SEND_NEW_MAIL mail;
    std::memset(&mail, 0, sizeof(mail));
    SIG_MAILBOX_SEND_NEW_MAIL::Request& request = mail.request;
    std::strncpy(request.sender_charac_name,
                 payload->sender_charac_name,
                 29u);
    request.receiver_charac_no = payload->receiver_charac_no;
    request.sender_charac_no = payload->sender_charac_no;
    request.old_letter_id = 0;
    request.unlimit_flag = payload->unlimit_flag;
    if (payload->letter_text_length) {
        request.letter_text_length = payload->letter_text_length;
        std::strncpy(request.letter_text,
                     payload->letter_text,
                     request.letter_text_length);
    }

    if (payload->package_type == 0) {
        if (!DB_AvatarItemInsert::InsertAvatarItem(
                &payload->package.avatar)) {
            throw 26737;
        }
        const unsigned int identity =
            WongWork::DBCommon::GetIdentity(db_handle);
        if (!identity) {
            throw 26740;
        }
        request.seal_flag = 0;
        request.item_id = payload->package.avatar.item_id;
        request.add_info = identity;
        request.upgrade = 0;
        request.endurance = static_cast<uint8_t>(
            payload->package.avatar.ability_no);
        request.extend_info = payload->package.avatar.add_info;
        request.avatar_flag = 1;
    } else if (payload->package_type == 1) {
        if (!DB_InsertCreatureItem::insertCreatureItem(
                db_handle, &payload->package.creature)) {
            throw 26755;
        }
        const unsigned int identity =
            WongWork::DBCommon::GetIdentity(db_handle);
        if (!identity) {
            throw 26758;
        }
        request.seal_flag = 0;
        request.item_id =
            payload->package.creature.inven_item_copy.item_id;
        request.add_info = identity;
        request.upgrade = 0;
        request.endurance =
            payload->package.creature.inven_item_copy.endurance;
        request.creature_flag = 1;
    }

    if (!DB_MailBox_Req_Send::insertMailData(db_handle, &mail)) {
        throw 26769;
    }
    if (!DB_MailBox_Req_Send::sendResult(
            user_idx,
            0,
            payload->receiver_charac_no,
            static_cast<ENUM_SERVER_GROUP>(0))) {
        throw 26772;
    }
    } catch (int error) {
        cMyTrace trace(__PRETTY_FUNCTION__, 26777, 5);
        trace("%d", error);
    }
    return true;
}
