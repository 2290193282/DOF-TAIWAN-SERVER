#include "old_data/db/mail/db_mailbox_req_delete.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



// IDA_EA: 0x83FF968.
// IDA_NAME: _Z15getList2inQueryPjPc
// DEMANGLED: getList2inQuery(unsigned int *,char *)
extern char* getList2inQuery(unsigned int* values, char* dest)
    __asm__("_Z15getList2inQueryPjPc");

namespace {

const ENUM_DB_HANDLE_IDX kMailboxDeleteDbHandle = DB_HANDLE_3;
const ENUM_SERVER_GROUP kMailboxDeleteDefaultServerGroup =
    SERVER_GROUP_DEFAULT;

}  // namespace

// IDA_EA: 0x844E312
// IDA_NAME: _ZN21DB_MailBox_Req_DeleteC2Ev
// DEMANGLED: DB_MailBox_Req_Delete::DB_MailBox_Req_Delete(void)
DB_MailBox_Req_Delete::DB_MailBox_Req_Delete() {}

// IDA_EA: 0x845F4D2
// IDA_NAME: _ZN21DB_MailBox_Req_DeleteD2Ev
// DEMANGLED: DB_MailBox_Req_Delete::~DB_MailBox_Req_Delete()
// IDA_EA: 0x845F502
// IDA_NAME: _ZN21DB_MailBox_Req_DeleteD0Ev
// DEMANGLED: DB_MailBox_Req_Delete::~DB_MailBox_Req_Delete()
DB_MailBox_Req_Delete::~DB_MailBox_Req_Delete() {}

// IDA_EA: 0x841F084
// IDA_NAME: _ZN21DB_MailBox_Req_Delete8dispatchEiiP6Stream
// DEMANGLED: DB_MailBox_Req_Delete::dispatch(int,int,Stream *)
bool DB_MailBox_Req_Delete::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream->enable_out(sizeof(SIG_MAILBOX_DEL_MAIL))) {
        return false;
    }
    SIG_MAILBOX_DEL_MAIL* payload =
        reinterpret_cast<SIG_MAILBOX_DEL_MAIL*>(stream->get_buffer());
    stream->out_ptr(sizeof(*payload));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kMailboxDeleteDbHandle,
        kMailboxDeleteDefaultServerGroup);
    if (!payload->count) {
        return false;
    }

    if (payload->count == 1) {
        db_handle->set_query(
            "upDate postal set delete_flag=%d,receive_time=now() where postal_id=%d",
            payload->delete_flag,
            payload->postal_ids[0]);
    } else {
        static char str_query[512];
        const char* list_query = getList2inQuery(
            reinterpret_cast<unsigned int*>(payload),
            str_query);
        db_handle->set_query(
            "upDate postal set delete_flag=%d,receive_time=now() where postal_id %s",
            payload->delete_flag,
            list_query);
    }

    return db_handle->exec(true) == true;
}
