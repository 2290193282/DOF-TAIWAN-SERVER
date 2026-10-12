#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_INVENTORY_MUTATION_BARRIER_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_INVENTORY_MUTATION_BARRIER_H_

#include <stdint.h>

namespace inventory_mutation_barrier {

// Reads the authoritative DB_HANDLE_3 inventory image and accepts only the
// exact post-mutation stack state. Event ledgers call this after the normal
// inventory snapshot request has run on DB_Q and before publishing rewards.
bool HasExactPersistedStack(int server_group,
                            uint32_t charac_no,
                            uint16_t slot,
                            uint32_t item_id,
                            uint32_t expected_count);

// Sea Bottom additionally journals the pre-mutation total of the payment
// item across all 352 main-inventory slots.  Requiring both the exact source
// slot post-image and total_before-1 excludes a disconnect-time slot move
// from being mistaken for consumption.
bool HasExactPersistedStackAndTotal(int server_group,
                                    uint32_t charac_no,
                                    uint16_t slot,
                                    uint32_t item_id,
                                    uint32_t expected_slot_count,
                                    uint32_t expected_total_count);

// Strong Sea Bottom recovery barrier: the authoritative primary+expanded
// DB3 image must additionally equal the journaled post-mutation SHA-256.
bool HasExactPersistedInventoryMutation(
    int server_group,
    uint32_t charac_no,
    uint16_t slot,
    uint32_t item_id,
    uint32_t expected_slot_count,
    uint32_t expected_total_count,
    const uint8_t expected_inventory_sha256[32]);

// Decide-level rewards may touch several stacks plus EventCoin in one full
// signal-35 snapshot.  This verifies the complete primary+expanded DB3 image
// and the coin column through the native worker's exact handle 17.
bool HasExactPersistedInventoryImageAndEventCoin(
    int server_group,
    uint32_t charac_no,
    const uint8_t expected_inventory_sha256[32],
    uint32_t expected_event_coin);

// Phoenix actions can mutate an equipped ordinary weapon, a main-inventory
// stack, and character gold in one signal-35 snapshot.  The digest is over
// the exact DB serializer projection: 12 ordinary equipment records followed
// by all 352 primary+expanded main-inventory records.
bool HasExactPersistedFreelyGiveImageAndMoney(
    int server_group,
    uint32_t charac_no,
    const uint8_t expected_inventory_sha256[32],
    uint32_t expected_money);

}  // namespace inventory_mutation_barrier_0828

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_INVENTORY_MUTATION_BARRIER_0828_H_
