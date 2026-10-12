#include "old_data/db/inventory/db_item_growth_state.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/equipment_growth.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/account_cargo_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/security/policy/checksum.h"

#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <zlib.h>


namespace item_growth_db {
namespace {

const char kStateTable[] = "item_growth_state_0828";
const char kSagaTable[] = "item_growth_reset_saga_0828";
const char kPendingTable[] = "item_growth_reset_pending_0828";
const unsigned int kQueryChunkSize = 256u;
const uint32_t kCompressedBlobRawSizeMask = 0x3fffffffu;
volatile uint32_t s_next_correlation = 0u;

typedef std::map<std::string, item_growth_state::State> LoadedStateMap;

bool ExecuteLiteral(MySQL* db, const char* sql) {
    return db && sql && db->set_query("%s", sql) && db->exec(true);
}

void Rollback(MySQL* db) {
    if (db) (void)ExecuteLiteral(db, "rollback");
}

bool Commit(MySQL* db) {
    return ExecuteLiteral(db, "commit");
}

bool HasNonZeroDigest(const uint8_t* digest) {
    if (!digest) return false;
    uint8_t merged = 0u;
    for (unsigned int index = 0; index < 32u; ++index) {
        merged = static_cast<uint8_t>(merged | digest[index]);
    }
    return merged != 0u;
}

std::string HexBytes(const void* raw, unsigned int length) {
    static const char digits[] = "0123456789abcdef";
    const uint8_t* const bytes = static_cast<const uint8_t*>(raw);
    std::string text(length * 2u, '0');
    for (unsigned int index = 0; index < length; ++index) {
        text[2u * index] = digits[bytes[index] >> 4];
        text[2u * index + 1u] = digits[bytes[index] & 0x0fu];
    }
    return text;
}

std::string MakeKey(const void* guid_bytes) {
    return std::string(static_cast<const char*>(guid_bytes),
                       item_growth_state::kGuidBytes);
}

std::string HexGuid(const void* guid_bytes) {
    static const char digits[] = "0123456789abcdef";
    const uint8_t* const bytes = static_cast<const uint8_t*>(guid_bytes);
    std::string text;
    text.resize(item_growth_state::kGuidBytes * 2u);
    for (int index = 0; index < item_growth_state::kGuidBytes; ++index) {
        text[2 * index] = digits[bytes[index] >> 4];
        text[2 * index + 1] = digits[bytes[index] & 0x0fu];
    }
    return text;
}

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < 0x12;
}

bool IsValidWriteMode(uint8_t mode, uint32_t correlation) {
    if (mode == kWriteAsync) return correlation == 0u;
    return correlation != 0u &&
           (mode == kWriteStageReset || mode == kWriteStageCompensate ||
            mode == kWriteCargoReset || mode == kWriteCargoCompensate);
}

MySQL* GetGrowthDB(int server_group) {
    return GlobalData::s_db_mgr && IsValidServerGroup(server_group)
               ? GlobalData::s_db_mgr->GetDBHandleExact(
                     DB_HANDLE_FEATURE_DB3,
                     static_cast<ENUM_SERVER_GROUP>(server_group))
               : 0;
}

MySQL* GetAccountCargoDB(int server_group) {
    return GlobalData::s_db_mgr && IsValidServerGroup(server_group)
               ? GlobalData::s_db_mgr->GetDBHandleExact(
                     DB_HANDLE_FEATURE_DB2,
                     static_cast<ENUM_SERVER_GROUP>(server_group))
               : 0;
}

bool ReadExactBlob(MySQL* db,
                   int column,
                   unsigned int raw_bytes,
                   std::vector<uint8_t>* raw) {
    if (!db || !raw || raw_bytes == 0u) return false;
    const int encoded_bytes = db->get_binary_length(column);
    if (encoded_bytes < 4 ||
        static_cast<uLong>(encoded_bytes) > compressBound(raw_bytes) + 4u) {
        return false;
    }
    std::vector<uint8_t> encoded(static_cast<size_t>(encoded_bytes), 0u);
    if (!db->get_binary(column, &encoded[0], encoded_bytes)) return false;
    uint32_t declared = 0u;
    memcpy(&declared, &encoded[0], sizeof(declared));
    if ((declared & kCompressedBlobRawSizeMask) != raw_bytes) return false;
    raw->assign(raw_bytes, 0u);
    uLongf decoded_bytes = static_cast<uLongf>(raw_bytes);
    return ::uncompress(reinterpret_cast<Bytef*>(&(*raw)[0]),
                        &decoded_bytes,
                        reinterpret_cast<const Bytef*>(&encoded[4]),
                        static_cast<uLong>(encoded.size() - 4u)) == Z_OK &&
           decoded_bytes == static_cast<uLongf>(raw_bytes);
}

bool HashBytes(const std::vector<uint8_t>& raw, uint8_t digest[32]) {
    if (raw.empty() || !digest ||
        raw.size() > static_cast<size_t>(0x7fffffffu)) {
        return false;
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

bool ReadInventoryDigest(MySQL* db,
                         uint32_t charac_no,
                         uint8_t digest[32]) {
    if (!db || !charac_no || !digest ||
        !db->set_query(
            "seLect i.inventory,e.jewel from inventory i join "
            "charac_inven_expand e on e.charac_no=i.charac_no where "
            "i.charac_no=%u limit 2 for update",
            static_cast<unsigned int>(charac_no)) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }
    const unsigned int primary_bytes =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int expanded_bytes =
        oldsrv::config::kMainInventoryExpandedItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    std::vector<uint8_t> primary;
    std::vector<uint8_t> expanded;
    if (!ReadExactBlob(db, 0, primary_bytes, &primary) ||
        !ReadExactBlob(db, 1, expanded_bytes, &expanded)) {
        return false;
    }
    primary.insert(primary.end(), expanded.begin(), expanded.end());
    return HashBytes(primary, digest);
}

bool ReadAccountCargoDigest(MySQL* db,
                            uint32_t account_id,
                            uint8_t digest[32]) {
    if (!db || !account_id || !digest ||
        !db->set_query(
            "seLect cargo from account_cargo where m_id=%u limit 2 for update",
            static_cast<unsigned int>(account_id)) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }
    std::vector<uint8_t> raw;
    return ReadExactBlob(db,
                         0,
                         56u * static_cast<unsigned int>(sizeof(Inven_Item)),
                         &raw) &&
           HashBytes(raw, digest);
}

bool ValidateResetProof(const Message& message, int user_idx) {
    const bool stage = message.write_mode == kWriteStageReset;
    const bool cargo = message.write_mode == kWriteCargoReset;
    if ((!stage && !cargo) || message.correlation == 0u ||
        message.owner_user_idx != user_idx || message.owner_user_idx < 0 ||
        message.account_id == 0u || message.charac_no == 0u ||
        message.source_item_id == 0u || message.target_item_id == 0u ||
        message.growth_progress != 0u || message.fatigue_state != 0u ||
        message.reserved != 0u ||
        !HasNonZeroDigest(message.pre_inventory_sha256) ||
        !HasNonZeroDigest(message.post_inventory_sha256) ||
        memcmp(message.pre_inventory_sha256,
               message.post_inventory_sha256,
               32u) == 0) {
        return false;
    }
    if (stage) {
        static const uint8_t zeros[32] = {0};
        return message.saga_owner == kSagaOwnerStage &&
               message.proof_flags == kSagaProofInventory &&
               message.source_space == kSagaSpaceInventory &&
               message.target_space == kSagaSpaceInventory &&
               message.source_slot == message.target_slot &&
               message.source_item_id != message.target_item_id &&
               memcmp(message.pre_account_cargo_sha256, zeros, 32u) == 0 &&
               memcmp(message.post_account_cargo_sha256, zeros, 32u) == 0;
    }
    return message.saga_owner == kSagaOwnerCargo &&
           message.proof_flags ==
               (kSagaProofInventory | kSagaProofAccountCargo) &&
           message.source_space == kSagaSpaceInventory &&
           message.target_space == kSagaSpaceAccountCargo &&
           message.source_item_id == message.target_item_id &&
           HasNonZeroDigest(message.pre_account_cargo_sha256) &&
           HasNonZeroDigest(message.post_account_cargo_sha256) &&
           memcmp(message.pre_account_cargo_sha256,
                  message.post_account_cargo_sha256,
                  32u) != 0;
}

bool BeginResetSaga(MySQL* db, const Message& message, int user_idx) {
    if (!db || !ValidateResetProof(message, user_idx) ||
        !ExecuteLiteral(db, "start transaction")) {
        return false;
    }
    const std::string guid = HexGuid(message.item_guid);
    if (!db->set_query(
            "seLect growth_progress,fatigue_state from %s where "
            "item_guid=X'%s' for update",
            kStateTable,
            guid.c_str()) ||
        !db->exec(true) || db->get_n_rows() > 1) {
        Rollback(db);
        return false;
    }
    unsigned int stored_progress = 0u;
    unsigned int stored_fatigue = 0u;
    if (db->get_n_rows() == 1) {
        if (!db->fetch() || !db->get_uint(0, stored_progress) ||
            !db->get_uint(1, stored_fatigue)) {
            Rollback(db);
            return false;
        }
    }
    if (stored_progress != message.baseline_progress ||
        stored_fatigue != message.baseline_fatigue) {
        Rollback(db);
        return false;
    }

    const std::string pre_inventory =
        HexBytes(message.pre_inventory_sha256, 32u);
    const std::string post_inventory =
        HexBytes(message.post_inventory_sha256, 32u);
    const std::string pre_cargo =
        HexBytes(message.pre_account_cargo_sha256, 32u);
    const std::string post_cargo =
        HexBytes(message.post_account_cargo_sha256, 32u);
    if (!db->set_query(
            "inSert into %s(server_group,correlation,owner_user_idx,m_id,"
            "charac_no,item_guid,operation,saga_state,source_space,"
            "target_space,source_slot,target_slot,source_item_id,"
            "target_item_id,baseline_progress,baseline_fatigue,"
            "pre_inventory_sha256,post_inventory_sha256,"
            "pre_account_cargo_sha256,post_account_cargo_sha256,created_at) "
            "values(%u,%u,%d,%u,%u,X'%s',%u,1,%u,%u,%u,%u,%u,%u,%u,%u,"
            "X'%s',X'%s',X'%s',X'%s',now())",
            kSagaTable,
            static_cast<unsigned int>(message.server_group),
            static_cast<unsigned int>(message.correlation),
            message.owner_user_idx,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(message.charac_no),
            guid.c_str(),
            static_cast<unsigned int>(message.saga_owner),
            static_cast<unsigned int>(message.source_space),
            static_cast<unsigned int>(message.target_space),
            static_cast<unsigned int>(message.source_slot),
            static_cast<unsigned int>(message.target_slot),
            static_cast<unsigned int>(message.source_item_id),
            static_cast<unsigned int>(message.target_item_id),
            static_cast<unsigned int>(message.baseline_progress),
            static_cast<unsigned int>(message.baseline_fatigue),
            pre_inventory.c_str(),
            post_inventory.c_str(),
            pre_cargo.c_str(),
            post_cargo.c_str()) ||
        !db->exec(true) ||
        !db->set_query(
            "inSert into %s(server_group,item_guid,correlation,charac_no,m_id,"
            "created_at) values(%u,X'%s',%u,%u,%u,now())",
            kPendingTable,
            static_cast<unsigned int>(message.server_group),
            guid.c_str(),
            static_cast<unsigned int>(message.correlation),
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !db->exec(true) ||
        !db->set_query(
            "inSert into %s(item_guid,growth_progress,fatigue_state) "
            "values(X'%s',0,0) on duplicate key update "
            "growth_progress=0,fatigue_state=0",
            kStateTable,
            guid.c_str()) ||
        !db->exec(true) || !Commit(db)) {
        Rollback(db);
        return false;
    }
    return true;
}

bool CompensateResetSaga(MySQL* db, const Message& message) {
    if (!db || message.correlation == 0u ||
        !ExecuteLiteral(db, "start transaction")) {
        return false;
    }
    const std::string guid = HexGuid(message.item_guid);
    if (!db->set_query(
            "seLect operation,saga_state,baseline_progress,baseline_fatigue,"
            "owner_user_idx "
            "from %s where server_group=%u and correlation=%u and "
            "item_guid=X'%s' for update",
            kSagaTable,
            static_cast<unsigned int>(message.server_group),
            static_cast<unsigned int>(message.correlation),
            guid.c_str()) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        Rollback(db);
        return false;
    }
    unsigned int owner = 0u;
    unsigned int state = 0u;
    unsigned int progress = 0u;
    unsigned int fatigue = 0u;
    int owner_user_idx = -1;
    if (!db->get_uint(0, owner) || !db->get_uint(1, state) ||
        !db->get_uint(2, progress) || !db->get_uint(3, fatigue) ||
        !db->get_int(4, owner_user_idx) ||
        owner_user_idx != message.owner_user_idx ||
        owner != static_cast<unsigned int>(
                     message.write_mode == kWriteStageCompensate
                         ? kSagaOwnerStage
                         : kSagaOwnerCargo) ||
        progress > 0xffffu || fatigue > 0xffu) {
        Rollback(db);
        return false;
    }
    if (state == 3u) return Commit(db);
    if (state != 1u ||
        !db->set_query(
            "inSert into %s(item_guid,growth_progress,fatigue_state) "
            "values(X'%s',%u,%u) on duplicate key update "
            "growth_progress=values(growth_progress),"
            "fatigue_state=values(fatigue_state)",
            kStateTable,
            guid.c_str(),
            progress,
            fatigue) ||
        !db->exec(true) ||
        !db->set_query(
            "upDate %s set saga_state=3,finalized_at=now() where "
            "server_group=%u and correlation=%u and saga_state=1",
            kSagaTable,
            static_cast<unsigned int>(message.server_group),
            static_cast<unsigned int>(message.correlation)) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u ||
        !db->set_query(
            "deLete from %s where server_group=%u and item_guid=X'%s' and "
            "correlation=%u",
            kPendingTable,
            static_cast<unsigned int>(message.server_group),
            guid.c_str(),
            static_cast<unsigned int>(message.correlation)) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u || !Commit(db)) {
        Rollback(db);
        return false;
    }
    return true;
}

bool LoadOpenSagas(MySQL* db,
                   int server_group,
                   uint32_t account_id,
                   uint32_t charac_no,
                   std::vector<Message>* rows) {
    if (!db || !rows || (!account_id && !charac_no)) return false;
    bool query_set = false;
    if (account_id && charac_no) {
        query_set = db->set_query(
            "seLect correlation,owner_user_idx,m_id,charac_no,item_guid,"
            "operation,source_space,target_space,source_slot,target_slot,"
            "source_item_id,target_item_id,baseline_progress,baseline_fatigue,"
            "pre_inventory_sha256,post_inventory_sha256,"
            "pre_account_cargo_sha256,post_account_cargo_sha256 from %s "
            "where server_group=%u and saga_state=1 and "
            "(m_id=%u or charac_no=%u)",
            kSagaTable,
            static_cast<unsigned int>(server_group),
            static_cast<unsigned int>(account_id),
            static_cast<unsigned int>(charac_no));
    } else if (account_id) {
        query_set = db->set_query(
            "seLect correlation,owner_user_idx,m_id,charac_no,item_guid,"
            "operation,source_space,target_space,source_slot,target_slot,"
            "source_item_id,target_item_id,baseline_progress,baseline_fatigue,"
            "pre_inventory_sha256,post_inventory_sha256,"
            "pre_account_cargo_sha256,post_account_cargo_sha256 from %s "
            "where server_group=%u and saga_state=1 and m_id=%u",
            kSagaTable,
            static_cast<unsigned int>(server_group),
            static_cast<unsigned int>(account_id));
    } else {
        query_set = db->set_query(
            "seLect correlation,owner_user_idx,m_id,charac_no,item_guid,"
            "operation,source_space,target_space,source_slot,target_slot,"
            "source_item_id,target_item_id,baseline_progress,baseline_fatigue,"
            "pre_inventory_sha256,post_inventory_sha256,"
            "pre_account_cargo_sha256,post_account_cargo_sha256 from %s "
            "where server_group=%u and saga_state=1 and charac_no=%u",
            kSagaTable,
            static_cast<unsigned int>(server_group),
            static_cast<unsigned int>(charac_no));
    }
    if (!query_set || !db->exec(true)) return false;
    const int count = db->get_n_rows();
    for (int index = 0; index < count; ++index) {
        Message row;
        memset(&row, 0, sizeof(row));
        row.magic = kMessageMagic;
        row.version = kMessageVersion;
        row.operation = kOperationSave;
        row.server_group = server_group;
        row.result = kResultFailed;
        unsigned int correlation = 0u;
        int owner_user_idx = -1;
        unsigned int owner = 0u;
        unsigned int source_space = 0u;
        unsigned int target_space = 0u;
        unsigned int source_slot = 0u;
        unsigned int target_slot = 0u;
        unsigned int baseline_progress = 0u;
        unsigned int baseline_fatigue = 0u;
        if (!db->fetch() || !db->get_uint(0, correlation) ||
            !db->get_int(1, owner_user_idx) ||
            !db->get_uint(2, row.account_id) ||
            !db->get_uint(3, row.charac_no) ||
            !db->get_binary(4, row.item_guid, sizeof(row.item_guid)) ||
            !db->get_uint(5, owner) ||
            !db->get_uint(6, source_space) ||
            !db->get_uint(7, target_space) ||
            !db->get_uint(8, source_slot) ||
            !db->get_uint(9, target_slot) ||
            !db->get_uint(10, row.source_item_id) ||
            !db->get_uint(11, row.target_item_id) ||
            !db->get_uint(12, baseline_progress) ||
            !db->get_uint(13, baseline_fatigue) ||
            !db->get_binary(14, row.pre_inventory_sha256, 32) ||
            !db->get_binary(15, row.post_inventory_sha256, 32) ||
            !db->get_binary(16, row.pre_account_cargo_sha256, 32) ||
            !db->get_binary(17, row.post_account_cargo_sha256, 32) ||
            correlation == 0u || owner_user_idx < 0 ||
            (owner != kSagaOwnerStage && owner != kSagaOwnerCargo) ||
            source_space > 0xffu || target_space > 0xffu ||
            source_slot > 0xffffu || target_slot > 0xffffu ||
            baseline_progress > 0xffffu || baseline_fatigue > 0xffu ||
            !item_growth_state::IsValidGuid(row.item_guid)) {
            return false;
        }
        row.correlation = correlation;
        row.owner_user_idx = owner_user_idx;
        row.saga_owner = static_cast<uint8_t>(owner);
        row.source_space = static_cast<uint8_t>(source_space);
        row.target_space = static_cast<uint8_t>(target_space);
        row.source_slot = static_cast<uint16_t>(source_slot);
        row.target_slot = static_cast<uint16_t>(target_slot);
        row.baseline_progress = static_cast<uint16_t>(baseline_progress);
        row.baseline_fatigue = static_cast<uint8_t>(baseline_fatigue);
        rows->push_back(row);
    }
    return true;
}

bool FinishRecoveredSaga(MySQL* db, const Message& row, bool post_image) {
    if (!db) return false;
    const std::string guid = HexGuid(row.item_guid);
    if (!post_image &&
        (!db->set_query(
             "inSert into %s(item_guid,growth_progress,fatigue_state) "
             "values(X'%s',%u,%u) on duplicate key update "
             "growth_progress=values(growth_progress),"
             "fatigue_state=values(fatigue_state)",
             kStateTable,
             guid.c_str(),
             static_cast<unsigned int>(row.baseline_progress),
             static_cast<unsigned int>(row.baseline_fatigue)) ||
         !db->exec(true))) {
        Rollback(db);
        return false;
    }
    if (!db->set_query(
            "upDate %s set saga_state=%u,finalized_at=now() where "
            "server_group=%u and correlation=%u and saga_state=1",
            kSagaTable,
            post_image ? 2u : 3u,
            static_cast<unsigned int>(row.server_group),
            static_cast<unsigned int>(row.correlation)) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u ||
        !db->set_query(
            "deLete from %s where server_group=%u and item_guid=X'%s' and "
            "correlation=%u",
            kPendingTable,
            static_cast<unsigned int>(row.server_group),
            guid.c_str(),
            static_cast<unsigned int>(row.correlation)) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u || !Commit(db)) {
        Rollback(db);
        return false;
    }
    return true;
}

bool RecoverOpenSagas(MySQL* db,
                      int server_group,
                      uint32_t account_id,
                      uint32_t charac_no,
                      int notify_user_idx,
                      std::vector<Message>* notifications) {
    std::vector<Message> rows;
    if (!LoadOpenSagas(db, server_group, account_id, charac_no, &rows)) {
        return false;
    }
    bool unresolved = false;
    for (std::vector<Message>::iterator it = rows.begin(); it != rows.end();
         ++it) {
        if (!ExecuteLiteral(db, "start transaction")) return false;
        const std::string guid = HexGuid(it->item_guid);
        if (!db->set_query(
                "seLect saga_state from %s where server_group=%u and "
                "correlation=%u and item_guid=X'%s' for update",
                kSagaTable,
                static_cast<unsigned int>(it->server_group),
                static_cast<unsigned int>(it->correlation),
                guid.c_str()) ||
            !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
            Rollback(db);
            return false;
        }
        unsigned int saga_state = 0u;
        if (!db->get_uint(0, saga_state) || saga_state != 1u) {
            Rollback(db);
            return false;
        }
        uint8_t inventory_digest[32];
        memset(inventory_digest, 0, sizeof(inventory_digest));
        if (!ReadInventoryDigest(db, it->charac_no, inventory_digest)) {
            Rollback(db);
            unresolved = true;
            continue;
        }
        bool pre_image =
            memcmp(inventory_digest, it->pre_inventory_sha256, 32u) == 0;
        bool post_image =
            memcmp(inventory_digest, it->post_inventory_sha256, 32u) == 0;
        MySQL* cargo_db = 0;
        if (it->saga_owner == kSagaOwnerCargo) {
            cargo_db = GetAccountCargoDB(server_group);
            if (!cargo_db || !ExecuteLiteral(cargo_db, "start transaction")) {
                Rollback(db);
                unresolved = true;
                continue;
            }
            uint8_t cargo_digest[32];
            memset(cargo_digest, 0, sizeof(cargo_digest));
            if (!ReadAccountCargoDigest(cargo_db,
                                        it->account_id,
                                        cargo_digest)) {
                Rollback(cargo_db);
                Rollback(db);
                unresolved = true;
                continue;
            }
            pre_image = pre_image &&
                memcmp(cargo_digest,
                       it->pre_account_cargo_sha256,
                       32u) == 0;
            post_image = post_image &&
                memcmp(cargo_digest,
                       it->post_account_cargo_sha256,
                       32u) == 0;
        }
        if (pre_image == post_image) {
            if (cargo_db) Rollback(cargo_db);
            Rollback(db);
            unresolved = true;
            LogManager::logFormat(
                1,
                "db_item_growth_state_0828.cpp",
                "RecoverOpenSagas",
                0,
                "SQL036 unresolved authoritative image sg=%d account=%u charac=%u corr=%u owner=%u",
                server_group,
                it->account_id,
                it->charac_no,
                it->correlation,
                static_cast<unsigned int>(it->saga_owner));
            continue;
        }
        if (!FinishRecoveredSaga(db, *it, post_image)) {
            if (cargo_db) Rollback(cargo_db);
            return false;
        }
        // The DB2 transaction is read-only.  Its row lock is held until the
        // DB3 journal transition is durable, closing the proof/finalize race.
        // A failed read-only COMMIT cannot invalidate the already-committed
        // authoritative proof, so it must not manufacture a false failure.
        if (cargo_db && !Commit(cargo_db)) Rollback(cargo_db);
        if (post_image && notifications &&
            notify_user_idx == it->owner_user_idx) {
            it->write_mode = static_cast<uint8_t>(
                it->saga_owner == kSagaOwnerStage
                    ? kWriteStageFinalize
                    : kWriteCargoFinalize);
            it->growth_progress = 0u;
            it->fatigue_state = 0u;
            it->result = kResultOk;
            notifications->push_back(*it);
        }
    }
    return !unresolved;
}

bool SendResult(int user_idx, const Message& source) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const result = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!result) return false;
    *result = source;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool SendForwardedResult(int response_signal,
                         int user_idx,
                         const void* payload,
                         unsigned int payload_bytes) {
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!queues || !payload || payload_bytes == 0u) return false;

    Stream* const stream = payload_bytes > 0x3fffu
                               ? (GlobalData::big_stream_pool()
                                      ? GlobalData::big_stream_pool()->Acquire(
                                            __FILE__, __LINE__)
                                      : 0)
                               : (GlobalData::stream_pool()
                                      ? GlobalData::stream_pool()->Acquire(
                                            __FILE__, __LINE__)
                                      : 0);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << response_signal;
    *guard << user_idx;
    void* const copy =
        guard.GetInBufferUntyped(static_cast<int>(payload_bytes));
    if (!copy) return false;
    memcpy(copy, payload, payload_bytes);
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool QueueContinuation(int user_idx,
                       int server_group,
                       uint8_t operation,
                       int response_signal,
                       uint32_t account_id,
                       uint32_t charac_no,
                       const void* payload,
                       unsigned int payload_bytes) {
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || !IsValidServerGroup(server_group) || !queues ||
        !payload || payload_bytes == 0u) {
        return false;
    }

    Stream* const stream =
        payload_bytes + sizeof(ContinuationHeader) > 0x3fffu
            ? (GlobalData::big_stream_pool()
                   ? GlobalData::big_stream_pool()->Acquire(__FILE__, __LINE__)
                   : 0)
            : (GlobalData::stream_pool()
                   ? GlobalData::stream_pool()->Acquire(__FILE__, __LINE__)
                   : 0);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    ContinuationHeader* const header =
        reinterpret_cast<ContinuationHeader*>(guard.GetInBufferUntyped(
            static_cast<int>(sizeof(ContinuationHeader))));
    if (!header) return false;
    header->magic = kMessageMagic;
    header->version = kMessageVersion;
    header->operation = operation;
    header->reserved = 0u;
    header->server_group = server_group;
    header->response_signal = response_signal;
    header->payload_bytes = payload_bytes;
    header->account_id = account_id;
    header->charac_no = charac_no;
    void* const copy = guard.GetInBufferUntyped(
        static_cast<int>(payload_bytes));
    if (!copy) return false;
    memcpy(copy, payload, payload_bytes);
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

void AppendInventoryGuids(GuidList* guids,
                          const SIG_LOAD_INVENTORY& payload) {
    AppendItems(guids,
                payload.tag_inventory.avatar_inventory,
                sizeof(payload.tag_inventory.avatar_inventory) /
                    sizeof(payload.tag_inventory.avatar_inventory[0]));
    AppendItems(guids,
                payload.tag_inventory.avatar_equipment,
                sizeof(payload.tag_inventory.avatar_equipment) /
                    sizeof(payload.tag_inventory.avatar_equipment[0]));
    AppendItems(guids,
                payload.avatar_equipment,
                sizeof(payload.avatar_equipment) /
                    sizeof(payload.avatar_equipment[0]));
    AppendItems(guids,
                payload.inventory,
                sizeof(payload.inventory) / sizeof(payload.inventory[0]));
    AppendItems(guids,
                payload.avatar_inventory,
                sizeof(payload.avatar_inventory) /
                    sizeof(payload.avatar_inventory[0]));
    AppendItems(guids,
                payload.equipment,
                sizeof(payload.equipment) / sizeof(payload.equipment[0]));
    AppendItems(guids,
                payload.cargo,
                sizeof(payload.cargo) / sizeof(payload.cargo[0]));
    AppendItems(guids,
                payload.creature_inventory,
                sizeof(payload.creature_inventory) /
                    sizeof(payload.creature_inventory[0]));
    AppendItems(guids,
                payload.expanded_equipment.expand_equipslot_items,
                sizeof(payload.expanded_equipment.expand_equipslot_items) /
                    sizeof(payload.expanded_equipment.expand_equipslot_items[0]));
}

bool HandleContinuation(int user_idx,
                        const ContinuationHeader& header,
                        const void* payload) {
    LogManager::logFormat(
        1,
        "db_item_growth_state.cpp",
        "item_growth_db::HandleContinuation",
        760,
        "[CHARVIEW][GROWTH_CONT_ENTER] operation=%u signal=%d payload_bytes=%u server_group=%d",
        static_cast<unsigned int>(header.operation),
        header.response_signal,
        header.payload_bytes,
        header.server_group);
    if (header.magic != kMessageMagic ||
        header.version != kMessageVersion || header.reserved != 0u ||
        !IsValidServerGroup(header.server_group) || !payload) {
        return false;
    }

    GuidList guids;
    if (header.operation == kOperationPreloadInventory &&
        header.response_signal == 8 &&
        header.payload_bytes == sizeof(SIG_LOAD_INVENTORY)) {
        const SIG_LOAD_INVENTORY& inventory =
            *static_cast<const SIG_LOAD_INVENTORY*>(payload);
        if (!header.account_id || !header.charac_no ||
            header.account_id != inventory.account_id ||
            header.charac_no != inventory.charac_no) {
            return false;
        }
        AppendInventoryGuids(
            &guids, inventory);
    } else if (header.operation == kOperationPreloadAccountCargo &&
               header.response_signal == 498 &&
               header.payload_bytes == sizeof(SIG_ACCOUNT_CARGO_DATA)) {
        if (!header.account_id || header.charac_no != 0u) return false;
        const SIG_ACCOUNT_CARGO_DATA& cargo =
            *static_cast<const SIG_ACCOUNT_CARGO_DATA*>(payload);
        AppendItems(&guids,
                    cargo.items,
                    sizeof(cargo.items) / sizeof(cargo.items[0]));
    } else if (header.operation == kOperationPreloadMailbox &&
               header.response_signal == 127 &&
               header.payload_bytes == sizeof(SIG_MAILBOX_LIST)) {
        if (header.account_id != 0u || !header.charac_no) return false;
        const SIG_MAILBOX_LIST& mailbox =
            *static_cast<const SIG_MAILBOX_LIST*>(payload);
        if (mailbox.response.charac_no !=
            static_cast<int32_t>(header.charac_no)) {
            return false;
        }
        if (mailbox.response.mail_count < 0 ||
            mailbox.response.mail_count >
                static_cast<int>(sizeof(mailbox.response.mails) /
                                 sizeof(mailbox.response.mails[0]))) {
            return false;
        }
        for (int index = 0; index < mailbox.response.mail_count; ++index) {
            const SIG_MAILBOX_MAIL_ENTRY& mail =
                mailbox.response.mails[index];
            if (equipment_growth::FindPrototype(
                    static_cast<int>(mail.item_id), 0)) {
                AppendGuid(&guids, mail.item_guid);
            }
        }
    } else {
        return false;
    }

    MySQL* const db = GetGrowthDB(header.server_group);
    const bool recovered =
        db && RecoverOpenSagas(db,
                               header.server_group,
                               header.account_id,
                               header.charac_no,
                               -1,
                               0);
    const bool preloaded = recovered && Preload(db, guids);
    const bool forwarded =
        preloaded && SendForwardedResult(header.response_signal,
                                         user_idx,
                                         payload,
                                         header.payload_bytes);
    LogManager::logFormat(
        1,
        "db_item_growth_state.cpp",
        "item_growth_db::HandleContinuation",
        850,
        "[CHARVIEW][GROWTH_CONT_RESULT] db=%d recover=%d preload=%d forward=%d guid_count=%u",
        db ? 1 : 0,
        recovered ? 1 : 0,
        preloaded ? 1 : 0,
        forwarded ? 1 : 0,
        static_cast<unsigned int>(guids.size()));
    return forwarded;
}

bool LoadChunk(MySQL* db,
               const std::vector<std::string>& keys,
               unsigned int begin,
               unsigned int end) {
    if (!db || begin >= end || end > keys.size()) return false;

    std::string query(
        "seLect item_guid,growth_progress,fatigue_state from ");
    query += kStateTable;
    query += " where item_guid in (";
    for (unsigned int index = begin; index < end; ++index) {
        if (index != begin) query += ',';
        query += "X'";
        query += HexGuid(keys[index].data());
        query += '\'';
    }
    query += ')';
    if (!db->set_query("%s", query.c_str()) || !db->exec(true)) {
        return false;
    }

    LoadedStateMap loaded;
    const int row_count = db->get_n_rows();
    for (int row = 0; row < row_count; ++row) {
        uint8_t guid[item_growth_state::kGuidBytes];
        unsigned int progress = 0;
        unsigned int fatigue = 0;
        memset(guid, 0, sizeof(guid));
        if (!db->fetch() ||
            !db->get_binary(0, guid, sizeof(guid)) ||
            !db->get_uint(1, progress) ||
            !db->get_uint(2, fatigue) || progress > 0xffffu ||
            fatigue > 0xffu ||
            !item_growth_state::IsValidGuid(guid)) {
            return false;
        }
        loaded[MakeKey(guid)] = item_growth_state::State(
            static_cast<uint16_t>(progress),
            static_cast<uint8_t>(fatigue));
    }

    for (unsigned int index = begin; index < end; ++index) {
        const LoadedStateMap::const_iterator found = loaded.find(keys[index]);
        const item_growth_state::State state =
            found == loaded.end() ? item_growth_state::State()
                                  : found->second;
        if (!item_growth_state::InstallBytes(keys[index].data(), state)) {
            return false;
        }
    }
    return true;
}

}  // namespace


bool AppendGuid(GuidList* list, const void* guid_bytes) {
    if (!list || !item_growth_state::IsValidGuid(guid_bytes)) {
        return false;
    }
    Guid value;
    memcpy(value.bytes, guid_bytes, sizeof(value.bytes));
    list->push_back(value);
    return true;
}

void AppendItems(GuidList* list,
                 const Inven_Item* items,
                 unsigned int count) {
    if (!list || (!items && count != 0u)) return;
    for (unsigned int index = 0; index < count; ++index) {
        if (equipment_growth::FindPrototype(items[index].item_id_, 0)) {
            AppendGuid(list, &items[index].globally_unique_id_);
        }
    }
}

bool Preload(MySQL* db, const GuidList& guids) {
    if (!db) return false;
    std::set<std::string> unique;
    for (GuidList::const_iterator it = guids.begin(); it != guids.end(); ++it) {
        if (item_growth_state::IsValidGuid(it->bytes)) {
            unique.insert(MakeKey(it->bytes));
        }
    }
    if (unique.empty()) return true;

    const std::vector<std::string> keys(unique.begin(), unique.end());
    for (unsigned int begin = 0; begin < keys.size();
         begin += kQueryChunkSize) {
        const unsigned int end =
            begin + kQueryChunkSize < keys.size()
                ? begin + kQueryChunkSize
                : static_cast<unsigned int>(keys.size());
        if (!LoadChunk(db, keys, begin, end)) return false;
    }
    return true;
}

bool QueueInventoryPreload(int user_idx,
                           int server_group,
                           const SIG_LOAD_INVENTORY& payload) {
    return QueueContinuation(user_idx,
                             server_group,
                             kOperationPreloadInventory,
                             8,
                             payload.account_id,
                             payload.charac_no,
                             &payload,
                             sizeof(payload));
}

bool QueueAccountCargoPreload(int user_idx,
                              int server_group,
                              uint32_t account_id,
                              const SIG_ACCOUNT_CARGO_DATA& payload) {
    return QueueContinuation(user_idx,
                             server_group,
                             kOperationPreloadAccountCargo,
                             498,
                             account_id,
                             0u,
                             &payload,
                             sizeof(payload));
}

bool QueueMailboxPreload(int user_idx,
                         int server_group,
                         uint32_t charac_no,
                         const SIG_MAILBOX_LIST& payload) {
    return QueueContinuation(user_idx,
                             server_group,
                             kOperationPreloadMailbox,
                             127,
                             0u,
                             charac_no,
                             &payload,
                             sizeof(payload));
}

void InitLegacySaveProofTrailer(LegacySaveProofTrailer* trailer,
                                int server_group,
                                uint32_t account_id,
                                uint32_t charac_no,
                                uint8_t proof_kind) {
    if (!trailer) return;
    memset(trailer, 0, sizeof(*trailer));
    trailer->magic = kLegacyProofMagic;
    trailer->version = kMessageVersion;
    trailer->proof_kind = proof_kind;
    trailer->server_group = server_group;
    trailer->account_id = account_id;
    trailer->charac_no = charac_no;
}

bool QueuePersistedProof(int user_idx,
                         int server_group,
                         uint32_t account_id,
                         uint32_t charac_no,
                         uint8_t proof_kind) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx < 0 || !IsValidServerGroup(server_group) || !account_id ||
        !charac_no ||
        (proof_kind != kSagaProofInventory &&
         proof_kind != kSagaProofAccountCargo) ||
        !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const message = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!message) return false;
    memset(message, 0, sizeof(*message));
    message->magic = kMessageMagic;
    message->version = kMessageVersion;
    message->operation = kOperationReconcilePersisted;
    message->proof_flags = proof_kind;
    message->server_group = server_group;
    message->owner_user_idx = user_idx;
    message->account_id = account_id;
    message->charac_no = charac_no;
    message->result = kResultFailed;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

uint32_t NextCorrelation() {
    uint32_t value = __sync_add_and_fetch(&s_next_correlation, 1u);
    if (value == 0u) {
        value = __sync_add_and_fetch(&s_next_correlation, 1u);
    }
    return value;
}

DB_ItemGrowthState::DB_ItemGrowthState() : DBDispatcher() {}
DB_ItemGrowthState::~DB_ItemGrowthState() {}

bool DB_ItemGrowthState::QueueSave(
    int user_idx,
    int server_group,
    const itemGloballyUniqueIdentifier_t& guid,
    const item_growth_state::State& state,
    uint8_t write_mode,
    uint32_t correlation,
    const SagaProof* proof) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    const bool reset = write_mode == kWriteStageReset ||
                       write_mode == kWriteCargoReset;
    if (user_idx == -1 || !IsValidServerGroup(server_group) ||
        !IsValidWriteMode(write_mode, correlation) ||
        (reset != (proof != 0)) ||
        !item_growth_state::IsValidGuid(&guid) || !pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const message = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!message) return false;
    memset(message, 0, sizeof(*message));
    message->magic = kMessageMagic;
    message->version = kMessageVersion;
    message->operation = kOperationSave;
    message->write_mode = write_mode;
    message->correlation = correlation;
    message->server_group = server_group;
    message->owner_user_idx = user_idx;
    memcpy(message->item_guid, &guid, sizeof(message->item_guid));
    message->growth_progress = state.progress;
    message->fatigue_state = state.fatigue_state;
    message->result = static_cast<uint8_t>(kResultFailed);
    if (proof) {
        memcpy(&message->saga_owner, proof, sizeof(*proof));
    }
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_ItemGrowthState::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() < 8) {
        return false;
    }
    const uint8_t operation =
        static_cast<uint8_t>(stream->get_buffer()[6]);
    if (operation == kOperationReconcilePersisted) {
        if (stream->length() != static_cast<int>(sizeof(Message))) return false;
        const Message* const request =
            reinterpret_cast<const Message*>(stream->get_buffer());
        if (!request || request->magic != kMessageMagic ||
            request->version != kMessageVersion ||
            request->operation != kOperationReconcilePersisted ||
            !IsValidServerGroup(request->server_group) ||
            request->owner_user_idx != user_idx || request->account_id == 0u ||
            request->charac_no == 0u ||
            (request->proof_flags != kSagaProofInventory &&
             request->proof_flags != kSagaProofAccountCargo)) {
            return false;
        }
        MySQL* const db = GetGrowthDB(request->server_group);
        std::vector<Message> notifications;
        if (!db || !RecoverOpenSagas(db,
                                     request->server_group,
                                     request->account_id,
                                     request->charac_no,
                                     user_idx,
                                     &notifications)) {
            return false;
        }
        for (std::vector<Message>::const_iterator it = notifications.begin();
             it != notifications.end(); ++it) {
            if (!SendResult(user_idx, *it)) return false;
        }
        return true;
    }
    if (operation != kOperationSave) {
        if (stream->length() < static_cast<int>(sizeof(ContinuationHeader))) {
            return false;
        }
        const ContinuationHeader* const header =
            reinterpret_cast<const ContinuationHeader*>(stream->get_buffer());
        if (!header ||
            stream->length() !=
                static_cast<int>(sizeof(*header) + header->payload_bytes)) {
            return false;
        }
        return HandleContinuation(user_idx,
                                  *header,
                                  stream->get_buffer() + sizeof(*header));
    }

    if (stream->length() != static_cast<int>(sizeof(Message))) return false;
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || request->magic != kMessageMagic ||
        request->version != kMessageVersion ||
        request->operation != kOperationSave ||
        !IsValidServerGroup(request->server_group) ||
        !IsValidWriteMode(request->write_mode, request->correlation) ||
        !item_growth_state::IsValidGuid(request->item_guid)) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultFailed);
    MySQL* const db = GetGrowthDB(response.server_group);
    bool persisted = false;
    if (db && response.write_mode == kWriteAsync) {
        const std::string hex = HexGuid(response.item_guid);
        persisted = db->set_query(
                        "inSert into %s(item_guid,growth_progress,"
                        "fatigue_state) values(X'%s',%u,%u) on duplicate key "
                        "update growth_progress=values(growth_progress),"
                        "fatigue_state=values(fatigue_state)",
                        kStateTable,
                        hex.c_str(),
                        static_cast<unsigned int>(response.growth_progress),
                        static_cast<unsigned int>(response.fatigue_state)) &&
                    db->exec(true);
    } else if (db &&
               (response.write_mode == kWriteStageReset ||
                response.write_mode == kWriteCargoReset)) {
        persisted = BeginResetSaga(db, response, user_idx);
    } else if (db &&
               (response.write_mode == kWriteStageCompensate ||
                response.write_mode == kWriteCargoCompensate)) {
        persisted = CompensateResetSaga(db, response);
    }
    if (persisted) {
        response.result = static_cast<uint8_t>(kResultOk);
    }
    return SendResult(user_idx, response);
}

}  // namespace item_growth_db_0828
