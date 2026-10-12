#include "old_data/db/taiwan/taiwan_restrict_dispatchers.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/Login/simple_sso.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x815DFF8
// IDA_NAME: _ZN25LoadServiceRestrictPolicyC2Ev
// DEMANGLED: LoadServiceRestrictPolicy::LoadServiceRestrictPolicy(void)
LoadServiceRestrictPolicy::LoadServiceRestrictPolicy()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::LOAD_SERVICE_RESTRICT_POLICY) {
}

// IDA_EA: 0x815E01C
// IDA_NAME: _ZN25LoadServiceRestrictPolicyD2Ev
// IDA_EA: 0x815E04C
// IDA_NAME: _ZN25LoadServiceRestrictPolicyD0Ev
// DEMANGLED: LoadServiceRestrictPolicy::~LoadServiceRestrictPolicy()
LoadServiceRestrictPolicy::~LoadServiceRestrictPolicy() {
}

// IDA_EA: 0x815A8F2
// IDA_NAME: _ZN25LoadServiceRestrictPolicy8dispatchEiiP6Stream
// DEMANGLED: LoadServiceRestrictPolicy::dispatch(int,int,Stream *)
int LoadServiceRestrictPolicy::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        throw "handle null";
    }

    const int server_group = G_CEnvironment()->get_server_group();
    if (!db_handle->set_query(
            "seLect category, restrict_code, restrict_value from "
            "dnf_restrict_state where server_group=%d",
            server_group)) {
        throw "set_query";
    }
    if (!db_handle->exec(true)) {
        throw "exec";
    }

    Taiwan::SigLoadRestrictPolicy policy;
    policy.init();
    while (db_handle->fetch()) {
        int category = 0;
        int restrict_code = 0;
        char restrict_value = 0;
        int column = 0;
        if (!db_handle->get_int(column++, category)) {
            throw "category";
        }
        if (!db_handle->get_int(column++, restrict_code)) {
            throw "restrict";
        }
        if (!db_handle->get_char(column++, restrict_value)) {
            throw "values";
        }

        if (policy.code != 0 && policy.code != category) {
            sendRestrictData(policy);
            policy.init();
        }
        policy.set(category, restrict_code, restrict_value);
    }

    // The old receiver queues its final frame even when the result set was
    // empty; Inter code 1 owns the corresponding empty-policy behavior.
    sendRestrictData(policy);
    return 1;
}

// IDA_EA: 0x815AC40
// IDA_NAME: _ZN25LoadServiceRestrictPolicy16sendRestrictDataERN6Taiwan21SigLoadRestrictPolicyE
// DEMANGLED: LoadServiceRestrictPolicy::sendRestrictData(Taiwan::SigLoadRestrictPolicy &)
void LoadServiceRestrictPolicy::sendRestrictData(
    Taiwan::SigLoadRestrictPolicy& policy) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp", 217);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(
        guard, TaiwanInternalPack::LOAD_SERVICE_RESTRICT_POLICY, -1);

    Taiwan::SigLoadRestrictPolicy* const destination =
        guard.GetInBuffer<Taiwan::SigLoadRestrictPolicy>();
    if (!destination) {
        throw "stream";
    }
    destination->set(policy);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
}


// IDA_EA: 0x815E06A
// IDA_NAME: _ZN25SaveServiceRestrictPolicyC2Ev
// DEMANGLED: SaveServiceRestrictPolicy::SaveServiceRestrictPolicy(void)
SaveServiceRestrictPolicy::SaveServiceRestrictPolicy()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SAVE_SERVICE_RESTRICT_POLICY) {
}

// IDA_EA: 0x815E08E
// IDA_NAME: _ZN25SaveServiceRestrictPolicyD2Ev
// IDA_EA: 0x815E0BE
// IDA_NAME: _ZN25SaveServiceRestrictPolicyD0Ev
// DEMANGLED: SaveServiceRestrictPolicy::~SaveServiceRestrictPolicy()
SaveServiceRestrictPolicy::~SaveServiceRestrictPolicy() {
}

// IDA_EA: 0x815AD3C
// IDA_NAME: _ZN25SaveServiceRestrictPolicy8dispatchEiiP6Stream
// DEMANGLED: SaveServiceRestrictPolicy::dispatch(int,int,Stream *)
int SaveServiceRestrictPolicy::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    Taiwan::SigLoadRestrictPolicy* const policy =
        stream->GetOutBuffer<Taiwan::SigLoadRestrictPolicy>();
    if (!policy) {
        throw "context null";
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        throw "handle null";
    }

    const int server_group = G_CEnvironment()->get_server_group();
    for (int index = 1; policy->count >= index; ++index) {
        if (!db_handle->set_query(
                "inSert ignore into dnf_restrict_state (server_group , "
                "category , restrict_code , restrict_value , mod_date , "
                "reg_date) values(%d, %d, %d, '%d', now(), now())",
                server_group,
                policy->code,
                index,
                policy->values[index])) {
            throw "set_query";
        }
        if (!db_handle->exec(true)) {
            throw "insert_exec";
        }
    }

    for (int index = 1; policy->count >= index; ++index) {
        if (!db_handle->set_query(
                "inSert ignore into dnf_restrict_info (category , "
                "restrict_code , restrict_str , reg_date) values(%d, %d, "
                "'%s', now())",
                policy->code,
                index,
                policy->names + 45 * index)) {
            throw "set_query";
        }
        if (!db_handle->exec(true)) {
            throw "insert_exec";
        }
    }
    return 1;
}


// IDA_EA: 0x815E0DC
// IDA_NAME: _ZN15LoadGeolocationC2Ev
// DEMANGLED: LoadGeolocation::LoadGeolocation(void)
LoadGeolocation::LoadGeolocation()
    : TaiwanInterfaceDBDispatcher(TaiwanInternalPack::LOAD_GEOLOCATION) {
}

// IDA_EA: 0x815E100
// IDA_NAME: _ZN15LoadGeolocationD2Ev
// IDA_EA: 0x815E130
// IDA_NAME: _ZN15LoadGeolocationD0Ev
// DEMANGLED: LoadGeolocation::~LoadGeolocation()
LoadGeolocation::~LoadGeolocation() {
}

// IDA_EA: 0x815B010
// IDA_NAME: _ZN15LoadGeolocation8dispatchEiiP6Stream
// DEMANGLED: LoadGeolocation::dispatch(int,int,Stream *)
int LoadGeolocation::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        throw "handle null";
    }

    const int server_group = G_CEnvironment()->get_server_group();
    // The old path intentionally does not test set_query's return value here.
    db_handle->set_query(
        "seLect country_code from geo_allow_country where server_group = %d",
        server_group);
    if (!db_handle->exec(true)) {
        throw "exec";
    }

    Taiwan::SigLoadGeolocation countries;
    countries.init();
    char country_code[16];
    std::memset(country_code, 0, sizeof(country_code));
    while (db_handle->fetch()) {
        std::memset(country_code, 0, sizeof(country_code));
        if (!db_handle->get_str(0, country_code, 16)) {
            throw "get_str";
        }
        countries.set(country_code);
        // Exact original cutoff: send after appending entry 44, rather than
        // filling the ABI's physical 45-entry storage limit.
        if (countries.count > 0x2B) {
            sendCountryData(countries);
            countries.init();
        }
    }

    sendCountryData(countries);
    return 1;
}

// IDA_EA: 0x815B266
// IDA_NAME: _ZN15LoadGeolocation15sendCountryDataERN6Taiwan18SigLoadGeolocationE
// DEMANGLED: LoadGeolocation::sendCountryData(Taiwan::SigLoadGeolocation &)
void LoadGeolocation::sendCountryData(Taiwan::SigLoadGeolocation& countries) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp", 324);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, TaiwanInternalPack::LOAD_GEOLOCATION, -1);

    Taiwan::SigLoadGeolocation* const destination =
        guard.GetInBuffer<Taiwan::SigLoadGeolocation>();
    if (!destination) {
        throw "stream";
    }
    destination->setData(countries);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
}


// IDA_EA: 0x815E232
// IDA_NAME: _ZN16DBUpdateExitTimeC2Ev
// DEMANGLED: DBUpdateExitTime::DBUpdateExitTime(void)
DBUpdateExitTime::DBUpdateExitTime()
    : TaiwanInterfaceDBDispatcher(TaiwanInternalPack::UPDATE_EXIT_TIME) {
}

// IDA_EA: 0x815E256
// IDA_NAME: _ZN16DBUpdateExitTimeD2Ev
// IDA_EA: 0x815E286
// IDA_NAME: _ZN16DBUpdateExitTimeD0Ev
// DEMANGLED: DBUpdateExitTime::~DBUpdateExitTime()
DBUpdateExitTime::~DBUpdateExitTime() {
}

// IDA_EA: 0x815B89A
// IDA_NAME: _ZN16DBUpdateExitTime11makeRequestEjij
// DEMANGLED: DBUpdateExitTime::makeRequest(unsigned int,int,unsigned int)
void DBUpdateExitTime::makeRequest(uint32_t account_id,
                                   int source_user_idx,
                                   uint32_t timestamp) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp", 441);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(
        guard, TaiwanInternalPack::UPDATE_EXIT_TIME, source_user_idx);

    Taiwan::SigUpdateExitTime* const update =
        guard.GetInBuffer<Taiwan::SigUpdateExitTime>();
    if (update) {
        update->init();
        update->set(account_id, timestamp);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x815B97C
// IDA_NAME: _ZN16DBUpdateExitTime8dispatchEiiP6Stream
// DEMANGLED: DBUpdateExitTime::dispatch(int,int,Stream *)
int DBUpdateExitTime::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    Taiwan::SigUpdateExitTime* const update =
        stream->GetOutBuffer<Taiwan::SigUpdateExitTime>();
    if (!update) {
        throw "handle null";
    }
    if (!GlobalData::s_psimpleSSO->updateSSOExpireTime(update->account_id,
                                                       update->expire_time)) {
        throw "exec";
    }
    return 1;
}
