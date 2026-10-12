#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_CHARACTER_SNAPSHOT_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_CHARACTER_SNAPSHOT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


class Stream;

namespace exp_keeping {
struct Message;
}

namespace zombie_mode {
struct Message;
}

namespace legacy_character_snapshot {

enum {
    // Slot 828 is empty in the restored 0..851 DBTask table.  It is private
    // to DB_Q and deliberately has no queue-6 dispatcher registration.
    kDbSignal = 828,
    kMaximumContinuationBytes = 160,
    kMaximumServerGroupCount = 18,
};

enum FeatureKind {
    kFeatureExpKeeping = 1,
    kFeatureZombieMode = 2,
};

enum ApplyStatus {
    kApplyRequest = 0,
    kApplyVerified = 1,
    kApplyFailed = 2,
};

struct CharacterSnapshot {
    uint32_t level;
    uint32_t experience;
    uint32_t remain_sp_first;
    uint32_t remain_sp_second;
    uint32_t remain_sfp_first;
    uint32_t remain_sfp_second;
};

// Fixed, pointer-free DB_Q request.  `continuation` contains one complete
// feature message and is zero-filled beyond continuation_size.  The legacy
// worker validates both the duplicated identity/snapshots and an FNV-1a
// checksum before it touches the canonical MyISAM rows.
struct Message {
    uint32_t magic;
    uint32_t version;
    uint32_t feature_kind;
    uint32_t apply_status;
    int32_t reply_user_idx;
    int32_t server_group;
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t continuation_size;
    uint32_t continuation_checksum;
    CharacterSnapshot baseline;
    CharacterSnapshot target;
    uint8_t continuation[kMaximumContinuationBytes];
};

static_assert(sizeof(CharacterSnapshot) == 24u,
              "legacy snapshot fence snapshot must remain 24 bytes");
static_assert(sizeof(Message) == 248u,
              "legacy snapshot fence message must remain 248 bytes");

// These entry points are callable only by the native SQL019/SQL026 owners
// after their immutable target has been committed to the sidecar table.
bool QueueExpKeepingApply(int reply_user_idx,
                          const exp_keeping::Message& continuation);
bool QueueZombieModeApply(int reply_user_idx,
                          const zombie_mode::Message& continuation);

class DB_LegacyCharacterSnapshot : public DBDispatcher {
public:
    DB_LegacyCharacterSnapshot();
    virtual ~DB_LegacyCharacterSnapshot();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace legacy_character_snapshot_0828

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_LEGACY_CHARACTER_SNAPSHOT_0828_H_
