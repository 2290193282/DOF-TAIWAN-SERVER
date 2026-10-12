#include "old_data/db/mail/db_mailbox_req_check_receiver.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>



namespace {

enum {
    kDbSignalMailboxCheckReceiver = 126,
};

}  // namespace

// IDA_EA: 0x844E2DA
// IDA_NAME: _ZN28DB_MailBox_Req_CheckReceiverC2Ev
// DEMANGLED: DB_MailBox_Req_CheckReceiver::DB_MailBox_Req_CheckReceiver(void)
DB_MailBox_Req_CheckReceiver::DB_MailBox_Req_CheckReceiver() {}

// IDA_EA: 0x845F56E
// IDA_NAME: _ZN28DB_MailBox_Req_CheckReceiverD2Ev
// DEMANGLED: DB_MailBox_Req_CheckReceiver::~DB_MailBox_Req_CheckReceiver()
// IDA_EA: 0x845F59E
// IDA_NAME: _ZN28DB_MailBox_Req_CheckReceiverD0Ev
// DEMANGLED: DB_MailBox_Req_CheckReceiver::~DB_MailBox_Req_CheckReceiver()
DB_MailBox_Req_CheckReceiver::~DB_MailBox_Req_CheckReceiver() {}

// IDA_EA: 0x841C882
// IDA_NAME: _ZN28DB_MailBox_Req_CheckReceiver8dispatchEiiP6Stream
// DEMANGLED: DB_MailBox_Req_CheckReceiver::dispatch(int,int,Stream *)
bool DB_MailBox_Req_CheckReceiver::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;

    // DBTask consumed the original full-width int UID before this payload.
    // The former three-byte compensation was only masking a non-original
    // byte-UID decode and shifted the request body.
    if (!stream->enable_out(sizeof(SIG_MAILBOX_CHECK_RECEIVER))) {
        return false;
    }
    SIG_MAILBOX_CHECK_RECEIVER* payload =
        reinterpret_cast<SIG_MAILBOX_CHECK_RECEIVER*>(stream->get_buffer());
    stream->out_ptr(sizeof(*payload));

    int receiver_charac_no = 0;
    int receiver_level = 0;
    int delete_flag = 0;
    unsigned int receiver_account_id = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    MySQL* black_list_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);

    char escaped_name[59];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->receiver_name);
    db_handle->set_query(
        "seLect charac_no,lev,delete_flag,m_id from charac_info where charac_name='%s'",
        escaped_name);
    if (db_handle->exec(true) != true) {
        return false;
    }

    for (unsigned int row = 0; row < static_cast<unsigned int>(db_handle->get_n_rows());
         ++row) {
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, receiver_charac_no) != true ||
            db_handle->get_int(1, receiver_level) != true ||
            db_handle->get_int(2, delete_flag) != true ||
            db_handle->get_uint(3, receiver_account_id) != true) {
            return false;
        }
    }

    Stream* out_stream =
        GlobalData::big_stream_pool()->Acquire("DBThread.cpp", 20634);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalMailboxCheckReceiver;
    *guard << user_idx;
    SIG_MAILBOX_CHECK_RECEIVER* response =
        guard->GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>();

    response->receiver_charac_no = receiver_charac_no;
    response->receiver_level = receiver_level;
    response->receiver_delete_flag = delete_flag;
    response->receiver_account_id = receiver_account_id;
    response->message = payload->message;

    const int sender_charac_no = payload->sender_charac_no;
    if (black_list_db->set_query(
            "seLect count(*) from charac_black_list where m_id = %s and charac_no = %d",
            NumberToString(receiver_account_id, 0),
            sender_charac_no) == true) {
        int black_list_count = 0;
        response->black_list_result = 0;
        if (black_list_db->exec(true) == true &&
            black_list_db->fetch() == true &&
            black_list_db->get_int(0, black_list_count) == true &&
            black_list_count > 0) {
            response->black_list_result = 77;
        }
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
