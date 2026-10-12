#include "old_data/db/auth_security/db_secu_service_update_validity_time.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



// IDA_EA: 0x844F1D6
// IDA_NAME: _ZN32DB_SecuServiceUpdateValidityTimeC2Ev
// DEMANGLED: DB_SecuServiceUpdateValidityTime::DB_SecuServiceUpdateValidityTime(void)
DB_SecuServiceUpdateValidityTime::DB_SecuServiceUpdateValidityTime() {}

// IDA_EA: 0x845CB14
// IDA_NAME: _ZN32DB_SecuServiceUpdateValidityTimeD2Ev
// DEMANGLED: DB_SecuServiceUpdateValidityTime::~DB_SecuServiceUpdateValidityTime()
// IDA_EA: 0x845CB44
// IDA_NAME: _ZN32DB_SecuServiceUpdateValidityTimeD0Ev
// DEMANGLED: DB_SecuServiceUpdateValidityTime::~DB_SecuServiceUpdateValidityTime() deleting destructor
DB_SecuServiceUpdateValidityTime::~DB_SecuServiceUpdateValidityTime() {}

// IDA_EA: 0x843A264
// IDA_NAME: _ZN32DB_SecuServiceUpdateValidityTime8dispatchEiiP6Stream
// DEMANGLED: DB_SecuServiceUpdateValidityTime::dispatch(int,int,Stream *)
bool DB_SecuServiceUpdateValidityTime::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SECURITY_CARD_SERVICE_VALIDITYTIME payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    payload.ip[sizeof(payload.ip) - 1] = '\0';

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload.service_type == 8) {
        db_handle->set_query(
            "upDate member_security_grade set security_card_validity_time\t=unix_timestamp(now())+%d , validity_ip = '%s' where m_id=%s",
            payload.seconds,
            payload.ip,
            NumberToString(payload.account_id, 0));
    } else if (payload.service_type == 16) {
        db_handle->set_query(
            "upDate member_security_grade set goblin_validity_time\t\t=unix_timestamp(now())+%d , validity_ip = '%s' where m_id=%s",
            payload.seconds,
            payload.ip,
            NumberToString(payload.account_id, 0));
    } else {
        return true;
    }
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x843A34E
// IDA_NAME: _ZN32DB_SecuServiceUpdateValidityTime11makeRequestEjjtPc
// DEMANGLED: DB_SecuServiceUpdateValidityTime::makeRequest(unsigned int,unsigned int,unsigned short,char *)
void DB_SecuServiceUpdateValidityTime::makeRequest(unsigned int account_id,
                                                   unsigned int seconds,
                                                   unsigned short service_type,
                                                   char* ip) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35028);
    CStreamGuard guard(stream, true);
    *guard << 457;
    *guard << -1;

    SIG_SECURITY_CARD_SERVICE_VALIDITYTIME* payload =
        static_cast<SIG_SECURITY_CARD_SERVICE_VALIDITYTIME*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_SECURITY_CARD_SERVICE_VALIDITYTIME)));
    memset(payload, 0, sizeof(SIG_SECURITY_CARD_SERVICE_VALIDITYTIME));
    payload->account_id = account_id;
    payload->seconds = seconds;
    payload->service_type = service_type;
    if (ip) {
        strncpy(payload->ip, ip, sizeof(payload->ip) - 1);
        payload->ip[sizeof(payload->ip) - 1] = '\0';
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
