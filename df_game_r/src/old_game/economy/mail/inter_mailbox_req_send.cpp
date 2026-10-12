#include "old_game/economy/mail/inter_mailbox_req_send.h"

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_data/db/event/db_take_magic_lamp.h"
#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "infra/log/format/trace.h"

#include <cstring>

// IDA_EA: 0x84ED63C
Inter_MailBox_Req_Send::Inter_MailBox_Req_Send() : ISigDispatcher() {}

// IDA_EA: 0x84F9C6C / 0x84F9C9C
Inter_MailBox_Req_Send::~Inter_MailBox_Req_Send() {}

// IDA_EA: 0x84D0130
// IDA_NAME: _ZN22Inter_MailBox_Req_Send12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MailBox_Req_Send::dispatch_sig(CUser *,char *,int)
int Inter_MailBox_Req_Send::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)user;

    if (!payload) {
        return 0;
    }

    const bool tracked_reply =
        payload_size ==
        static_cast<int>(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED));
    if (payload_size != static_cast<int>(sizeof(SIG_MAILBOX_SEND_NEW_MAIL)) &&
        !tracked_reply) {
        return 0;
    }

    const SIG_MAILBOX_SEND_NEW_MAIL::Result& reply =
        reinterpret_cast<const SIG_MAILBOX_SEND_NEW_MAIL*>(payload)->result;
    if (tracked_reply) {
        SIG_MAILBOX_PERSISTENCE_CONTEXT context;
        std::memcpy(&context,
                    payload + sizeof(SIG_MAILBOX_SEND_NEW_MAIL),
                    sizeof(context));
        const bool known_consumer =
            context.consumer ==
                MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP ||
            context.consumer ==
                MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE ||
            context.consumer ==
                MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP;
        const bool valid_context =
            context.magic == MAILBOX_PERSISTENCE_CONTEXT_MAGIC &&
            context.version == MAILBOX_PERSISTENCE_CONTEXT_VERSION &&
            known_consumer &&
            MailboxPersistenceCorrelationValid(context) &&
            context.receiver_charac_no != 0u &&
            context.reward_item_id != 0u &&
            context.receiver_charac_no ==
                static_cast<uint32_t>(reply.receiver_charac_no) &&
            context.server_group == reply.server_group;
        if (!valid_context) {
            return 0;
        }

        if (context.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP) {
            if (reply.result == 0 &&
                !magic_lamp::DB_TakeMagicLamp::
                    makeMarkMailPersistedRequest(
                        context.account_id,
                        context.receiver_charac_no,
                        context.reward_item_id,
                        context.server_group)) {
                cMyTrace trace(
                    "Inter_MailBox_Req_Send::dispatch_sig tracked persistence",
                    __LINE__,
                    5);
                trace("[0828 MagicLamp] postal persisted but claim-state "
                      "DB_Q submission failed account(%u) charac(%u)",
                      static_cast<unsigned int>(context.account_id),
                      static_cast<unsigned int>(context.receiver_charac_no));
            }
        } else if (context.consumer ==
                   MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE) {
            // Facebook Like owns a lease-backed per-character claim row.
            // Feed it both positive and negative postal outcomes so only a
            // real exec(true) ACK advances the state to mailed.
            facebook_like::OnTrackedMailResult(context,
                                                     reply.result == 0);
        } else {
            decide_levelup_event::OnTrackedMailResult(context,
                                                       reply.result == 0);
        }
    }
    if (reply.result == 0 && GlobalData::s_monitor_proxy_mgr) {
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(reply.server_group));
        if (proxy) {
            proxy->SendNotifyNewMail(
                static_cast<uint32_t>(reply.receiver_charac_no));
        }
    }
    return 0;
}
