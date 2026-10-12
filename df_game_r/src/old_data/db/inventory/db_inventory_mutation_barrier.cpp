#include "old_data/db/inventory/db_inventory_mutation_barrier.h"

#include "common/config.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/security/policy/checksum.h"

#include <cstddef>
#include <cstring>
#include <limits>
#include <vector>
#include <zlib.h>

namespace inventory_mutation_barrier {
namespace {

const uint32_t kCompressedBlobRawSizeMask = 0x3fffffffu;

bool ReadExactBlob(MySQL* database,
                   int column,
                   unsigned int raw_size,
                   std::vector<char>& raw) {
    if (!database || raw_size == 0u) return false;
    const int encoded_length = database->get_binary_length(column);
    if (encoded_length < 4 ||
        static_cast<uLong>(encoded_length) > compressBound(raw_size) + 4u) {
        return false;
    }
    std::vector<char> encoded(static_cast<size_t>(encoded_length));
    if (!database->get_binary(column, &encoded[0], encoded_length)) {
        return false;
    }
    uint32_t declared_size = 0u;
    std::memcpy(&declared_size, &encoded[0], sizeof(declared_size));
    declared_size &= kCompressedBlobRawSizeMask;
    if (declared_size != raw_size) return false;
    raw.assign(raw_size, 0);
    uLongf decoded_size = static_cast<uLongf>(raw_size);
    return ::uncompress(reinterpret_cast<Bytef*>(&raw[0]), &decoded_size,
                        reinterpret_cast<const Bytef*>(&encoded[4]),
                        static_cast<uLong>(encoded.size() - 4u)) == Z_OK &&
           decoded_size == static_cast<uLongf>(raw_size);
}

bool ReadPersistedImage(MySQL* database,
                        uint32_t charac_no,
                        bool primary,
                        std::vector<char>& raw) {
    if (!database || charac_no == 0u) return false;
    const unsigned int record_count = primary
        ? oldsrv::config::kMainInventoryPrimaryItemCount
        : oldsrv::config::kMainInventoryExpandedItemCount;
    if (!database->set_query(
            primary
                ? "seLect inventory from inventory where charac_no=%u limit 2"
                : "seLect jewel from charac_inven_expand where charac_no=%u limit 2",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }

    const unsigned int raw_size =
        record_count * static_cast<unsigned int>(sizeof(Inven_Item));
    return ReadExactBlob(database, 0, raw_size, raw);
}

bool ReadPersistedFullImage(MySQL* database,
                            uint32_t charac_no,
                            std::vector<char>& primary,
                            std::vector<char>& expanded) {
    if (!database || charac_no == 0u ||
        !database->set_query(
            "seLect i.inventory,e.jewel from inventory i join "
            "charac_inven_expand e on e.charac_no=i.charac_no where "
            "i.charac_no=%u limit 2",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    const unsigned int primary_size =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int expanded_size =
        oldsrv::config::kMainInventoryExpandedItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    return ReadExactBlob(database, 0, primary_size, primary) &&
           ReadExactBlob(database, 1, expanded_size, expanded);
}

bool ReadPersistedFullImageAndEventCoin(MySQL* database,
                                        uint32_t charac_no,
                                        std::vector<char>& primary,
                                        std::vector<char>& expanded,
                                        unsigned int& event_coin) {
    if (!database || charac_no == 0u ||
        !database->set_query(
            "seLect i.inventory,e.jewel,i.event_coin from inventory i join "
            "charac_inven_expand e on e.charac_no=i.charac_no where "
            "i.charac_no=%u limit 2",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    const unsigned int primary_size =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int expanded_size =
        oldsrv::config::kMainInventoryExpandedItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    return ReadExactBlob(database, 0, primary_size, primary) &&
           ReadExactBlob(database, 1, expanded_size, expanded) &&
           database->get_uint(2, event_coin);
}

bool ReadPersistedFreelyGiveImageAndMoney(
    MySQL* database,
    uint32_t charac_no,
    std::vector<char>& equipment,
    std::vector<char>& primary,
    std::vector<char>& expanded,
    unsigned int& money) {
    if (!database || charac_no == 0u ||
        !database->set_query(
            "seLect i.equipslot,i.inventory,e.jewel,i.money from inventory i "
            "join charac_inven_expand e on e.charac_no=i.charac_no where "
            "i.charac_no=%u limit 2",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    const unsigned int equipment_size =
        oldsrv::config::kOrdinaryEquipmentTypeCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int primary_size =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int expanded_size =
        oldsrv::config::kMainInventoryExpandedItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    return ReadExactBlob(database, 0, equipment_size, equipment) &&
           ReadExactBlob(database, 1, primary_size, primary) &&
           ReadExactBlob(database, 2, expanded_size, expanded) &&
           database->get_uint(3, money);
}

bool ReadRecord(const std::vector<char>& raw,
                unsigned int record_index,
                int32_t& persisted_item_id,
                int32_t& persisted_count) {
    if (record_index >= raw.size() / sizeof(Inven_Item)) return false;
    const size_t offset =
        static_cast<size_t>(record_index) * sizeof(Inven_Item);
    std::memcpy(&persisted_item_id,
                &raw[offset + offsetof(Inven_Item, item_id_)],
                sizeof(persisted_item_id));
    std::memcpy(&persisted_count,
                &raw[offset + offsetof(Inven_Item, add_info_)],
                sizeof(persisted_count));
    return true;
}

bool ReadPersistedRecord(int server_group,
                         uint32_t charac_no,
                         uint16_t slot,
                         int32_t& persisted_item_id,
                         int32_t& persisted_count) {
    if (!GlobalData::s_db_mgr || server_group < 0 || server_group >= 18 ||
        charac_no == 0u ||
        slot >= oldsrv::config::kMainInventoryItemCount) {
        return false;
    }
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, static_cast<ENUM_SERVER_GROUP>(server_group));
    if (!database) return false;
    const bool primary =
        slot < oldsrv::config::kMainInventoryPrimaryItemCount;
    const unsigned int record_index = primary
        ? static_cast<unsigned int>(slot)
        : static_cast<unsigned int>(slot) -
              oldsrv::config::kMainInventoryPrimaryItemCount;
    std::vector<char> raw;
    return ReadPersistedImage(database, charac_no, primary, raw) &&
           ReadRecord(raw, record_index, persisted_item_id, persisted_count);
}

bool AddPersistedItemTotal(const std::vector<char>& raw,
                           uint32_t item_id,
                           uint64_t& total) {
    const unsigned int records =
        static_cast<unsigned int>(raw.size() / sizeof(Inven_Item));
    for (unsigned int index = 0u; index < records; ++index) {
        int32_t persisted_item_id = 0;
        int32_t persisted_count = 0;
        if (!ReadRecord(raw, index, persisted_item_id, persisted_count))
            return false;
        if (persisted_item_id != static_cast<int32_t>(item_id)) continue;
        if (persisted_count <= 0) return false;
        total += static_cast<uint32_t>(persisted_count);
        if (total > static_cast<uint64_t>(
                        std::numeric_limits<int32_t>::max())) {
            return false;
        }
    }
    return true;
}

}  // namespace

bool HasExactPersistedStack(int server_group,
                            uint32_t charac_no,
                            uint16_t slot,
                            uint32_t item_id,
                            uint32_t expected_count) {
    if (item_id == 0u || item_id > 0x7fffffffu ||
        expected_count > 0x7fffffffu) {
        return false;
    }
    int32_t persisted_item_id = 0;
    int32_t persisted_count = 0;
    if (!ReadPersistedRecord(server_group, charac_no, slot,
                             persisted_item_id, persisted_count)) {
        return false;
    }
    if (expected_count == 0u)
        return persisted_item_id == 0 && persisted_count == 0;
    return persisted_item_id == static_cast<int32_t>(item_id) &&
           persisted_count == static_cast<int32_t>(expected_count);
}

bool HasExactPersistedStackAndTotal(int server_group,
                                    uint32_t charac_no,
                                    uint16_t slot,
                                    uint32_t item_id,
                                    uint32_t expected_slot_count,
                                    uint32_t expected_total_count) {
    if (!GlobalData::s_db_mgr || server_group < 0 || server_group >= 18 ||
        charac_no == 0u ||
        slot >= oldsrv::config::kMainInventoryItemCount || item_id == 0u ||
        item_id > 0x7fffffffu || expected_slot_count > 0x7fffffffu ||
        expected_total_count > 0x7fffffffu ||
        expected_total_count < expected_slot_count) {
        return false;
    }
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, static_cast<ENUM_SERVER_GROUP>(server_group));
    std::vector<char> primary;
    std::vector<char> expanded;
    if (!database || !ReadPersistedFullImage(database, charac_no, primary,
                                              expanded)) {
        return false;
    }
    const bool source_primary =
        slot < oldsrv::config::kMainInventoryPrimaryItemCount;
    const unsigned int source_index = source_primary
        ? static_cast<unsigned int>(slot)
        : static_cast<unsigned int>(slot) -
              oldsrv::config::kMainInventoryPrimaryItemCount;
    int32_t persisted_item_id = 0;
    int32_t persisted_count = 0;
    if (!ReadRecord(source_primary ? primary : expanded, source_index,
                    persisted_item_id, persisted_count)) {
        return false;
    }
    if (expected_slot_count == 0u) {
        if (persisted_item_id != 0 || persisted_count != 0) return false;
    } else if (persisted_item_id != static_cast<int32_t>(item_id) ||
               persisted_count !=
                   static_cast<int32_t>(expected_slot_count)) {
        return false;
    }
    uint64_t total = 0ULL;
    return AddPersistedItemTotal(primary, item_id, total) &&
           AddPersistedItemTotal(expanded, item_id, total) &&
           total == static_cast<uint64_t>(expected_total_count);
}

bool HasExactPersistedInventoryMutation(
    int server_group,
    uint32_t charac_no,
    uint16_t slot,
    uint32_t item_id,
    uint32_t expected_slot_count,
    uint32_t expected_total_count,
    const uint8_t expected_inventory_sha256[32]) {
    if (!GlobalData::s_db_mgr || !expected_inventory_sha256 ||
        server_group < 0 || server_group >= 18 || charac_no == 0u ||
        slot >= oldsrv::config::kMainInventoryItemCount || item_id == 0u ||
        item_id > 0x7fffffffu || expected_slot_count > 0x7fffffffu ||
        expected_total_count > 0x7fffffffu ||
        expected_total_count < expected_slot_count) {
        return false;
    }
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, static_cast<ENUM_SERVER_GROUP>(server_group));
    std::vector<char> primary;
    std::vector<char> expanded;
    if (!database || !ReadPersistedFullImage(database, charac_no, primary,
                                              expanded)) {
        return false;
    }
    const bool source_primary =
        slot < oldsrv::config::kMainInventoryPrimaryItemCount;
    const unsigned int source_index = source_primary
        ? static_cast<unsigned int>(slot)
        : static_cast<unsigned int>(slot) -
              oldsrv::config::kMainInventoryPrimaryItemCount;
    int32_t persisted_item_id = 0;
    int32_t persisted_count = 0;
    if (!ReadRecord(source_primary ? primary : expanded, source_index,
                    persisted_item_id, persisted_count)) {
        return false;
    }
    if (expected_slot_count == 0u) {
        if (persisted_item_id != 0 || persisted_count != 0) return false;
    } else if (persisted_item_id != static_cast<int32_t>(item_id) ||
               persisted_count !=
                   static_cast<int32_t>(expected_slot_count)) {
        return false;
    }
    uint64_t total = 0ULL;
    if (!AddPersistedItemTotal(primary, item_id, total) ||
        !AddPersistedItemTotal(expanded, item_id, total) ||
        total != static_cast<uint64_t>(expected_total_count)) {
        return false;
    }
    std::vector<unsigned char> raw(primary.size() + expanded.size());
    std::memcpy(&raw[0], &primary[0], primary.size());
    std::memcpy(&raw[primary.size()], &expanded[0], expanded.size());
    uint8_t digest[32] = {};
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
               CHECKSUM_RESULT_OK &&
           std::memcmp(digest, expected_inventory_sha256,
                       sizeof(digest)) == 0;
}

bool HasExactPersistedInventoryImageAndEventCoin(
    int server_group,
    uint32_t charac_no,
    const uint8_t expected_inventory_sha256[32],
    uint32_t expected_event_coin) {
    if (!GlobalData::s_db_mgr || !expected_inventory_sha256 ||
        server_group < 0 || server_group >= 18 || charac_no == 0u) {
        return false;
    }
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandleExact(
        DB_HANDLE_FEATURE_DB3,
        static_cast<ENUM_SERVER_GROUP>(server_group));
    std::vector<char> primary;
    std::vector<char> expanded;
    unsigned int persisted_event_coin = 0u;
    if (!database ||
        !ReadPersistedFullImageAndEventCoin(database, charac_no, primary,
                                            expanded,
                                            persisted_event_coin) ||
        persisted_event_coin != expected_event_coin) {
        return false;
    }
    std::vector<unsigned char> raw(primary.size() + expanded.size());
    if (raw.empty()) return false;
    std::memcpy(&raw[0], &primary[0], primary.size());
    std::memcpy(&raw[primary.size()], &expanded[0], expanded.size());
    uint8_t digest[32] = {};
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
               CHECKSUM_RESULT_OK &&
           std::memcmp(digest, expected_inventory_sha256, 32u) == 0;
}

bool HasExactPersistedFreelyGiveImageAndMoney(
    int server_group,
    uint32_t charac_no,
    const uint8_t expected_inventory_sha256[32],
    uint32_t expected_money) {
    if (!GlobalData::s_db_mgr || !expected_inventory_sha256 ||
        server_group < 0 || server_group >= 18 || charac_no == 0u) {
        return false;
    }
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandleExact(
        DB_HANDLE_FEATURE_DB3,
        static_cast<ENUM_SERVER_GROUP>(server_group));
    std::vector<char> equipment;
    std::vector<char> primary;
    std::vector<char> expanded;
    unsigned int persisted_money = 0u;
    if (!database ||
        !ReadPersistedFreelyGiveImageAndMoney(
            database, charac_no, equipment, primary, expanded,
            persisted_money) ||
        persisted_money != expected_money || equipment.empty() ||
        primary.empty() || expanded.empty()) {
        return false;
    }
    std::vector<unsigned char> raw(
        equipment.size() + primary.size() + expanded.size());
    size_t offset = 0u;
    std::memcpy(&raw[offset], &equipment[0], equipment.size());
    offset += equipment.size();
    std::memcpy(&raw[offset], &primary[0], primary.size());
    offset += primary.size();
    std::memcpy(&raw[offset], &expanded[0], expanded.size());
    uint8_t digest[32] = {};
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
               CHECKSUM_RESULT_OK &&
           std::memcmp(digest, expected_inventory_sha256, 32u) == 0;
}

}  // namespace inventory_mutation_barrier_0828
