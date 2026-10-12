#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mailbox_system_multi_mail_payload.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

namespace WongWork {

// IDA_EA: 0x8556B68
// IDA_NAME: _ZN8WongWork14CMailBoxHelper27ReqDBSendNewSystemMultiMailEPKcPK10Inven_ItemjjjS2_ij17ENUM_SERVER_GROUPb
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(char const*, Inven_Item const*, unsigned int, unsigned int, unsigned int, char const*, int, unsigned int, ENUM_SERVER_GROUP, bool)
int CMailBoxHelper::ReqDBSendNewSystemMultiMail(
    const char* title,
    const Inven_Item* items,
    unsigned int item_count,
    unsigned int sender,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    ENUM_SERVER_GROUP server_group,
    bool unlimit) {
    if (item_count > 10u || item_count == 0u) {
        return 17;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("MailBox.cpp", 2327);
    CStreamGuard guard(stream, true);
    guard << 676;
    guard << -1;

    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>();
    std::memset(payload, 0, sizeof(*payload));
    payload->sender_charac_no = 0;
    std::strcpy(payload->title, title);
    for (unsigned int index = 0; index < item_count; ++index) {
        payload->items[index].setCopy(items[index]);
    }
    payload->item_count = item_count;
    payload->sender = sender;
    payload->charac_no = charac_no;
    std::strcpy(payload->letter_text, text);
    payload->text_length = text_len;
    payload->occurrence_days = occ_time;
    payload->server_group = server_group;
    payload->unlimit_flag = unlimit ? 1 : 0;
    payload->letter_stat = 1;

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

}  // namespace WongWork
