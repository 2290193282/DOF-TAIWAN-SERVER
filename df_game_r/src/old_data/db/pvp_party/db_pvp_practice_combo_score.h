#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_PRACTICE_COMBO_SCORE_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_PRACTICE_COMBO_SCORE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace pvp_practice_combo_score {

enum {
    // RECONSTRUCTED_0828: slot 839 is unused in both restored 852-entry
    // DBTask/InterDispatcher tables.  It is an internal route, not a game
    // packet ID.
    kDbSignal = 839,
    kScoreCount = 3,
    kResultOk = 0,
    kResultFailed = 1,
};

enum Operation {
    OPERATION_QUERY = 1,
    OPERATION_UPDATE_MONOTONIC = 2,
};

#pragma pack(push, 1)
struct Message {
    uint32_t charac_no;
    uint32_t score;
    int32_t server_group;
    uint16_t score_index;
    uint8_t operation;
    uint8_t result;
};
#pragma pack(pop)

static_assert(sizeof(Message) == 16u,
              "0828 combo-score DB message must remain 16 bytes");

class DB_PvpPracticeComboScore : public DBDispatcher {
public:
    DB_PvpPracticeComboScore();
    virtual ~DB_PvpPracticeComboScore();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

bool QueueQuery(int user_idx,
                int server_group,
                uint32_t charac_no,
                uint16_t score_index);
bool QueueMonotonicUpdate(int user_idx,
                          int server_group,
                          uint32_t charac_no,
                          uint16_t score_index,
                          uint32_t score);

}  // namespace pvp_practice_combo_score_0828

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_PRACTICE_COMBO_SCORE_0828_H_
