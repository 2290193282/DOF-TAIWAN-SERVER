#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_DRAGON_TIMEATTACK_RANK_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_DRAGON_TIMEATTACK_RANK_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace dragon_timeattack_rank {

enum {
    // RECONSTRUCTED_0828: slot 841 is empty in both restored 852-entry
    // routing tables.  It is an internal DB/Inter route, not a packet ID.
    kDbSignal = 841,
    kDbHandle = 3,
    // CLIENT_EA: sub_949870 maps only 0..11 to job-name resources; 12 is
    // the no-character UI sentinel and must never become a DB partition.
    kMaximumJobType = 11,
    kMaximumRecordCount = 100,
    kMaximumCharacterNameBytes = 29,
};

enum Operation {
    OPERATION_UPDATE_BEST_TIME = 1,
    OPERATION_QUERY_RANKING = 2,
};

#pragma pack(push, 1)
struct RankRecord {
    uint32_t clear_time_ms;
    uint8_t job_type;
    uint8_t grow_type;
    uint8_t second_grow_type;
    uint8_t charac_level;
    char character_name[kMaximumCharacterNameBytes + 1];
};

struct Payload {
    uint32_t request_charac_no;
    uint32_t score_charac_no;
    uint32_t score_clear_time_ms;
    uint32_t personal_best_clear_time_ms;
    uint32_t personal_rank;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t requested_job_type;
    uint8_t score_job_type;
    uint8_t score_grow_type;
    uint8_t score_second_grow_type;
    uint8_t score_charac_level;
    uint8_t record_count;
    char score_character_name[kMaximumCharacterNameBytes + 1];
    RankRecord records[kMaximumRecordCount];
};
#pragma pack(pop)

static_assert(sizeof(RankRecord) == 38u,
              "0828 dragon time-attack record must remain 38 bytes");
static_assert(sizeof(Payload) == 3862u,
              "0828 dragon time-attack DB payload must remain 3862 bytes");

class DB_DragonTimeAttackRank : public DBDispatcher {
public:
    DB_DragonTimeAttackRank();
    virtual ~DB_DragonTimeAttackRank();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

void ClearPayload(Payload& payload);
bool Queue(int user_idx, const Payload& payload);

}  // namespace dragon_timeattack_rank_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_DRAGON_TIMEATTACK_RANK_0828_H_
