#include "old_game/economy/mail/arad_mail_runtime.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const char* AradMailString(uint32_t address) {
    return address
               ? reinterpret_cast<const char*>(
                     static_cast<uintptr_t>(address))
               : 0;
}

}  // namespace

namespace ARAD {

// IDA_EA: 0x8196B03
// IDA_NAME: _ZN4ARAD9send_mailERN10ARAD_DATAS12AradMailInfoE
// DEMANGLED: ARAD::send_mail(ARAD_DATAS::AradMailInfo &)
int send_mail(ARAD_DATAS::AradMailInfo& info) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localjapan/Arad_Function_Set.cpp", 58);
    CStreamGuard guard(stream, true);
    *guard << 128;
    *guard << -1;

    SIG_MAILBOX_SEND_NEW_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    SIG_MAILBOX_SEND_NEW_MAIL::Request& request = payload->request;

    // These stores intentionally mirror the old writer rather than calling
    // the newer helper, whose extra normalization changes the DB payload.
    request.receiver_charac_no = info.receiver_charac_no;
    request.sender_charac_no = info.sender_charac_no;
    request.server_group = static_cast<int32_t>(info.server_group);
    request.occ_time = info.occ_time;
    request.gold = info.gold;
    request.letter_text_length =
        static_cast<uint16_t>(info.letter_text_length);
    std::memset(request.letter_text, 0, sizeof(request.letter_text));

    const char* const title = AradMailString(info.title_address);
    std::strncpy(request.sender_charac_name, title, 0x1du);
    const char* const content = AradMailString(info.content_address);
    if (content) {
        const size_t content_length = std::strlen(content);
        std::strncpy(request.letter_text, content, content_length);
    }

    request.seal_flag = info.item_info.seal_flag;
    request.item_id = info.item_info.item_index;
    request.add_info = info.item_info.item_count;
    request.endurance = info.item_info.endurance;
    request.upgrade = static_cast<uint8_t>(info.item_info.GetItemAttr());
    request.extend_info = info.item_info.extend_info;
    request.amplify_option = 0;
    request.amplify_value = 0;
    request.chronicle_value = 0;
    request.creature_flag = 0;
    request.avatar_flag = 0;

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

}  // namespace ARAD
