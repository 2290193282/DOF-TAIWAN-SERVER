#include "old_data/db/inventory/db_load_account_cargo.h"
#include "old_data/db/inventory/account_cargo_payload.h"

#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>
#include <string.h>

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len);

// IDA_EA: 0x844F422
// IDA_NAME: _ZN19DB_LoadAccountCargoC2Ev
// DEMANGLED: DB_LoadAccountCargo::DB_LoadAccountCargo(void)
DB_LoadAccountCargo::DB_LoadAccountCargo() {}

// IDA_EA: 0x845C71E
// IDA_NAME: _ZN19DB_LoadAccountCargoD2Ev
// DEMANGLED: DB_LoadAccountCargo::~DB_LoadAccountCargo()
// IDA_EA: 0x845C74E
// IDA_NAME: _ZN19DB_LoadAccountCargoD0Ev
// DEMANGLED: DB_LoadAccountCargo::~DB_LoadAccountCargo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadAccountCargo::~DB_LoadAccountCargo() {}

// IDA_EA: 0x843B3B6
// IDA_NAME: _ZN19DB_LoadAccountCargo8dispatchEiiP6Stream
// DEMANGLED: DB_LoadAccountCargo::dispatch(int,int,Stream *)
bool DB_LoadAccountCargo::dispatch(int signal, int user_idx, Stream* stream) {
    unsigned int account_id = 0;
    int growth_server_group = -1;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1 ||
        (*stream >> growth_server_group) != 1 || stream->length() != 0 ||
        growth_server_group < 0 || growth_server_group >= 0x12) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "seLect capacity, money, cargo from account_cargo where m_id = %s",
        account_id_text);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            35923);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_ACCOUNT_CARGO_DATA* payload =
        guard.GetInBuffer<SIG_ACCOUNT_CARGO_DATA>();
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(*payload));
    if (db_handle->get_uint(0, payload->capacity) != true) {
        return false;
    }
    if (db_handle->get_uint(1, payload->money) != true) {
        return false;
    }

    char zip_buffer[4111];
    memset(zip_buffer, 0, sizeof(zip_buffer));
    if (db_handle->get_binary(2, zip_buffer, sizeof(zip_buffer)) != true) {
        return false;
    }

    const unsigned int binary_length =
        static_cast<unsigned int>(db_handle->get_binary_length(2));
    char* const cargo_bytes = reinterpret_cast<char*>(payload->items);
    unsigned int raw_length = sizeof(payload->items);
    if (uncompress_zip(cargo_bytes,
                       raw_length,
                       zip_buffer,
                       binary_length) != 1) {
        return false;
    }
    if (raw_length && raw_length % 0xD58u) {
        return false;
    }

    if (!item_growth_db::QueueAccountCargoPreload(
            user_idx, growth_server_group, account_id, *payload)) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadAccountCargo::dispatch(int, int, Stream*)",
            35922,
            "equipment growth continuation queue failed account_id=%u",
            account_id);
        return false;
    }
    return true;
}

// IDA_EA: 0x843B6EC
// IDA_NAME: _ZN19DB_LoadAccountCargo11makeRequestEij
// DEMANGLED: DB_LoadAccountCargo::makeRequest(int,unsigned int)
void DB_LoadAccountCargo::makeRequest(int user_idx,
                                      unsigned int account_id,
                                      int server_group) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35957);
    CStreamGuard guard(stream, true);
    *guard << 498;
    *guard << user_idx;
    *guard << account_id;
    *guard << server_group;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
