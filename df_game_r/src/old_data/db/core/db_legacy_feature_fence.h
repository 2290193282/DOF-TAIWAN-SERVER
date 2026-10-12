#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_FEATURE_FENCE_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_FEATURE_FENCE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


class CStreamGuard;
class Stream;

namespace legacy_feature_fence {

enum {
    kDbSignal = 823,
    kMaximumForwardBytes = 2048,
};

// Only explicitly audited routes may cross this lower-bound fence.  The
// legacy DB worker consumes the wrapper after every request already queued
// before it, then forwards the embedded pointer-free request to queue 6.
enum FenceKind {
    kFenceFreelyGiveLevelUp = 1,
    kFenceEventAccountFatigueRead = 2,
    kFenceCollectBoxFinalize = 3,
    kFenceDecideLevelupFinalize = 4,
    kFenceTwEvent720721InventoryCommit = 5,
    kFenceTwSeaBottomInventoryCommit = 6,
    kFenceFreelyGiveInventoryMoneyCommit = 7,
    // Shutdown-only q6 -> DB_Q -> q6 causal acknowledgement.  It carries no
    // SQL or gameplay mutation and is accepted only for the fixed 829 ACK
    // envelope validated by db_shutdown_quiescence_0828.
    kFenceShutdownQuiescence = 8,
};

struct Header {
    uint32_t magic;
    uint32_t version;
    uint32_t fence_kind;
    uint32_t target_size;
    uint32_t target_checksum;
};

static_assert(sizeof(Header) == 20u,
              "native 0828 legacy-fence header must remain fixed");

// target contains a complete queue-6 envelope, including signal and user_idx.
// It remains owned by its caller and is copied into a DB_Q wrapper.
bool QueueAfterLegacy(FenceKind kind, CStreamGuard& target);

class DB_LegacyFeatureFence : public DBDispatcher {
public:
    DB_LegacyFeatureFence();
    virtual ~DB_LegacyFeatureFence();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace legacy_native_fence_0828

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_NATIVE_FENCE_0828_H_
