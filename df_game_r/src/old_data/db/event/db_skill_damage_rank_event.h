#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SKILL_DAMAGE_RANK_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SKILL_DAMAGE_RANK_EVENT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace skill_damage_rank_event {

enum {
    // RECONSTRUCTED_0828: slot 842 is unused in both restored 852-entry
    // DB routing tables. It is an internal route, not a client packet ID.
    kDbSignal = 842,
    kDbHandle = 3,
    kMaximumGrowTypeCount = 4,
    kMaximumRowsPerGrowType = 10,
    kMaximumRecordCount =
        kMaximumGrowTypeCount * kMaximumRowsPerGrowType,
    kMaximumCharacterNameBytes = 29,
};

enum Operation {
    OPERATION_UPDATE_BEST_DAMAGE = 1,
    OPERATION_QUERY_RANKING = 2,
};

#pragma pack(push, 1)
struct RankRecord {
    uint32_t best_damage;
    uint8_t grow_type;
    char character_name[kMaximumCharacterNameBytes + 1];
};

struct Payload {
    uint32_t request_charac_no;
    uint32_t score_charac_no;
    uint32_t best_damage;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t job_type;
    uint8_t grow_type;
    uint8_t second_grow_type;
    uint8_t grow_type_count;
    uint8_t record_count;
    uint8_t reserved;
    uint8_t grow_types[kMaximumGrowTypeCount];
    char character_name[kMaximumCharacterNameBytes + 1];
    RankRecord records[kMaximumRecordCount];
};
#pragma pack(pop)

static_assert(sizeof(RankRecord) == 35u,
              "0828 skill-damage rank record must remain 35 bytes");
static_assert(sizeof(Payload) == 1458u,
              "0828 skill-damage DB payload must remain 1458 bytes");

class DB_SkillDamageRankEvent : public DBDispatcher {
public:
    DB_SkillDamageRankEvent();
    virtual ~DB_SkillDamageRankEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

bool Queue(int user_idx, const Payload& payload);
void ClearPayload(Payload& payload);

}  // namespace skill_damage_rank_event_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SKILL_DAMAGE_RANK_EVENT_0828_H_

