#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_ITEM_GROWTH_STATE_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_ITEM_GROWTH_STATE_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_game/economy/equipment/item_growth_state.h"

#include <stdint.h>
#include <vector>


class MySQL;
struct Inven_Item;
struct SIG_ACCOUNT_CARGO_DATA;
struct SIG_LOAD_INVENTORY;
struct SIG_MAILBOX_LIST;
struct itemGloballyUniqueIdentifier_t;

namespace item_growth_db {

// RECONSTRUCTED_0828: 845 was empty at both ends of the restored 852-slot
// DBTask/InterDispatcher tables.  It is private to this GUID sidecar and is
// not claimed as an original 0627 signal.
enum {
    kDbSignal = 845,
    kMessageMagic = 0x47383249u,  // "I28G" in little-endian storage.
    kMessageVersion = 1,
    kResultOk = 0,
    kResultFailed = 1,
    kOperationSave = 1,
    kOperationPreloadInventory = 2,
    kOperationPreloadAccountCargo = 3,
    kOperationPreloadMailbox = 4,
    kOperationReconcilePersisted = 5,
    kWriteAsync = 0,
    kWriteStageReset = 1,
    kWriteStageCompensate = 2,
    kWriteCargoReset = 3,
    kWriteCargoCompensate = 4,
    kWriteStageFinalize = 5,
    kWriteCargoFinalize = 6,
    kSagaOwnerNone = 0,
    kSagaOwnerStage = 1,
    kSagaOwnerCargo = 2,
    kSagaProofInventory = 1,
    kSagaProofAccountCargo = 2,
    kSagaSpaceInventory = 1,
    kSagaSpaceAccountCargo = 2,
    kLegacyProofMagic = 0x50383249u,  // "I28P"
};

struct Guid {
    uint8_t bytes[item_growth_state::kGuidBytes];
};
typedef std::vector<Guid> GuidList;

#pragma pack(push, 1)
struct Message {
    uint32_t magic;
    uint16_t version;
    uint8_t operation;
    uint8_t write_mode;
    uint32_t correlation;
    int32_t server_group;
    uint8_t item_guid[item_growth_state::kGuidBytes];
    uint16_t growth_progress;
    uint8_t fatigue_state;
    uint8_t result;
    uint8_t saga_owner;
    uint8_t source_space;
    uint8_t target_space;
    uint8_t proof_flags;
    uint16_t source_slot;
    uint16_t target_slot;
    int32_t owner_user_idx;
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t source_item_id;
    uint32_t target_item_id;
    uint16_t baseline_progress;
    uint8_t baseline_fatigue;
    uint8_t reserved;
    uint8_t pre_inventory_sha256[32];
    uint8_t post_inventory_sha256[32];
    uint8_t pre_account_cargo_sha256[32];
    uint8_t post_account_cargo_sha256[32];
};

struct SagaProof {
    uint8_t saga_owner;
    uint8_t source_space;
    uint8_t target_space;
    uint8_t proof_flags;
    uint16_t source_slot;
    uint16_t target_slot;
    int32_t owner_user_idx;
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t source_item_id;
    uint32_t target_item_id;
    uint16_t baseline_progress;
    uint8_t baseline_fatigue;
    uint8_t reserved;
    uint8_t pre_inventory_sha256[32];
    uint8_t post_inventory_sha256[32];
    uint8_t pre_account_cargo_sha256[32];
    uint8_t post_account_cargo_sha256[32];
};

// The continuation payload immediately follows this pointer-free header.
// `operation` selects one exact, already-static-asserted official response
// ABI; variable or caller-selected response shapes are rejected.
struct ContinuationHeader {
    uint32_t magic;
    uint16_t version;
    uint8_t operation;
    uint8_t reserved;
    int32_t server_group;
    int32_t response_signal;
    uint32_t payload_bytes;
    uint32_t account_id;
    uint32_t charac_no;
};

// Optional trailer on official signal35/signal497 requests.  It never changes
// their payload ABI; a legacy owner emits queue6 reconciliation only after its
// own SQL commit has succeeded.
struct LegacySaveProofTrailer {
    uint32_t magic;
    uint16_t version;
    uint8_t proof_kind;
    uint8_t reserved;
    int32_t server_group;
    uint32_t account_id;
    uint32_t charac_no;
};
#pragma pack(pop)

static_assert(sizeof(SagaProof) == 160u,
              "equipment-growth saga proof must remain pointer-free");
static_assert(sizeof(Message) == 190u,
              "equipment-growth DB message ABI changed");
static_assert(sizeof(ContinuationHeader) == 28u,
              "equipment-growth continuation header must remain 28 bytes");
static_assert(sizeof(LegacySaveProofTrailer) == 20u,
              "legacy persisted-proof trailer ABI changed");

bool AppendGuid(GuidList* list, const void* guid_bytes);
void AppendItems(GuidList* list, const Inven_Item* items, unsigned int count);

// Called only by the dedicated native continuation owner, never by a legacy
// DB load owner or PacketBuf.  Every valid requested GUID is cached as zero
// after a successful miss, and rows found in the sidecar override those
// defaults before the original result is queued to DISPATCH_Q.
bool Preload(MySQL* db, const GuidList& guids);

class DB_ItemGrowthState : public DBDispatcher {
public:
    DB_ItemGrowthState();
    virtual ~DB_ItemGrowthState();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueSave(
        int user_idx,
        int server_group,
        const itemGloballyUniqueIdentifier_t& guid,
        const item_growth_state::State& state,
        uint8_t write_mode = kWriteAsync,
        uint32_t correlation = 0u,
        const SagaProof* proof = 0);
};

// Legacy signals 8/498/127 retain their old DB_Q owners.  Those owners deep
// copy their completed official response into one of these continuations;
// only the native worker reads SQL006 and it forwards the original response
// to DISPATCH_Q after the sidecar preload succeeds.
bool QueueInventoryPreload(int user_idx,
                           int server_group,
                           const SIG_LOAD_INVENTORY& payload);
bool QueueAccountCargoPreload(int user_idx,
                              int server_group,
                              uint32_t account_id,
                              const SIG_ACCOUNT_CARGO_DATA& payload);
bool QueueMailboxPreload(int user_idx,
                         int server_group,
                         uint32_t charac_no,
                         const SIG_MAILBOX_LIST& payload);

bool QueuePersistedProof(int user_idx,
                         int server_group,
                         uint32_t account_id,
                         uint32_t charac_no,
                         uint8_t proof_kind);

void InitLegacySaveProofTrailer(LegacySaveProofTrailer* trailer,
                                int server_group,
                                uint32_t account_id,
                                uint32_t charac_no,
                                uint8_t proof_kind);

uint32_t NextCorrelation();

}  // namespace item_growth_db_0828

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_ITEM_GROWTH_STATE_0828_H_
