#include "old_data/db/inventory/db_save_account_cargo.h"
#include "old_data/db/inventory/account_cargo_payload.h"
#include "old_data/db/inventory/db_item_growth_state.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>
#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);

// IDA_EA: 0x844F43E
// IDA_NAME: _ZN19DB_SaveAccountCargoC2Ev
// DEMANGLED: DB_SaveAccountCargo::DB_SaveAccountCargo(void)
DB_SaveAccountCargo::DB_SaveAccountCargo() {}

// IDA_EA: 0x845C6D0
// IDA_NAME: _ZN19DB_SaveAccountCargoD2Ev
// DEMANGLED: DB_SaveAccountCargo::~DB_SaveAccountCargo()
// IDA_EA: 0x845C700
// IDA_NAME: _ZN19DB_SaveAccountCargoD0Ev
// DEMANGLED: DB_SaveAccountCargo::~DB_SaveAccountCargo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveAccountCargo::~DB_SaveAccountCargo() {}

// IDA_EA: 0x843B7C2
// IDA_NAME: _ZN19DB_SaveAccountCargo8dispatchEiiP6Stream
// DEMANGLED: DB_SaveAccountCargo::dispatch(int,int,Stream *)
bool DB_SaveAccountCargo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1) {
        return false;
    }

    SIG_ACCOUNT_CARGO_DATA payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    item_growth_db::LegacySaveProofTrailer proof;
    memset(&proof, 0, sizeof(proof));
    bool has_proof = false;
    if (stream->length() != 0) {
        if (stream->length() != static_cast<int>(sizeof(proof)) ||
            stream->get_binary(&proof, sizeof(proof)) != 1 ||
            stream->length() != 0 ||
            proof.magic != item_growth_db::kLegacyProofMagic ||
            proof.version != item_growth_db::kMessageVersion ||
            proof.proof_kind !=
                item_growth_db::kSagaProofAccountCargo ||
            proof.reserved != 0u || proof.server_group < 0 ||
            proof.server_group >= 0x12 || proof.account_id != account_id ||
            proof.charac_no == 0u) {
            return false;
        }
        has_proof = true;
    }

    char zip_buffer[4111];
    memset(zip_buffer, 0, sizeof(zip_buffer));
    unsigned int zip_length = sizeof(zip_buffer);
    if (compress_zip(zip_buffer,
                     zip_length,
                     reinterpret_cast<const char*>(payload.items),
                     sizeof(payload.items)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id_text = NumberToString(account_id, 0);
    char* cargo_blob = db_handle->blob_to_str(0, zip_buffer, zip_length);
    db_handle->set_query(
        "upDate account_cargo set capacity=%u, money=%u, cargo='%s' where m_id = %s",
        payload.capacity,
        payload.money,
        cargo_blob,
        account_id_text);
    if (db_handle->exec(true) != true) return false;
    return !has_proof || item_growth_db::QueuePersistedProof(
                             user_idx,
                             proof.server_group,
                             proof.account_id,
                             proof.charac_no,
                             proof.proof_kind);
}

// IDA_EA: 0x843B946
// IDA_NAME: _ZN19DB_SaveAccountCargo11makeRequestEijP13CAccountCargo
// DEMANGLED: DB_SaveAccountCargo::makeRequest(int,unsigned int,CAccountCargo *)
bool DB_SaveAccountCargo::makeRequest(int user_idx,
                                      unsigned int account_id,
                                      CAccountCargo* account_cargo,
                                      int server_group,
                                      unsigned int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35999);
    CStreamGuard guard(stream, true);
    *guard << 497;
    *guard << user_idx;
    *guard << account_id;

    void* payload_buffer =
        guard->GetInBufferUntyped(sizeof(SIG_ACCOUNT_CARGO_DATA));
    if (payload_buffer) {
        memset(payload_buffer, 0, sizeof(SIG_ACCOUNT_CARGO_DATA));
        if (account_cargo) {
            SIG_ACCOUNT_CARGO_DATA* payload =
                reinterpret_cast<SIG_ACCOUNT_CARGO_DATA*>(payload_buffer);
            payload->capacity = account_cargo->capacity_;
            payload->money = account_cargo->money_;
            memcpy(payload->items,
                   account_cargo->items_,
                   sizeof(payload->items));
        }
    }
    item_growth_db::LegacySaveProofTrailer* const proof =
        reinterpret_cast<item_growth_db::LegacySaveProofTrailer*>(
            guard.GetInBufferUntyped(
                sizeof(item_growth_db::LegacySaveProofTrailer)));
    if (!payload_buffer || !proof) return false;
    item_growth_db::InitLegacySaveProofTrailer(
        proof,
        server_group,
        account_id,
        charac_no,
        item_growth_db::kSagaProofAccountCargo);
    const bool queued =
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard) == 1;
    if (queued && account_cargo) {
        account_cargo->SetStable();
    }
    return queued;
}
