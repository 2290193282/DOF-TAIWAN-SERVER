#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_INTEGRATED_MATCH_PVP_SCORE_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_INTEGRATED_MATCH_PVP_SCORE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


class Stream;

namespace integrated_match_pvp_score_db {

enum {
    // Empty in both frozen 852-entry registries.  SQL003 is owned only by
    // the dedicated native queue and replies through the ordinary Inter map.
    kDbSignal = 826,
    kOperationLoad = 1,
    kOperationRecord = 2,
    kResultSuccess = 0,
    kResultDatabaseFailure = 1,
    kResultRequest = 0xff,
    kScoreFieldCount = 14,
};

enum ScoreField {
    kOneOnOneWins = 0,
    kOneOnOneCurrentStreak = 1,
    kOneOnOneMaxStreak = 2,
    kOneOnOnePerfectWins = 3,
    kLeaderBattleWins = 4,
    kLeaderBattleKills = 5,
    kLeaderBattleDoubleKills = 6,
    kTeamBattleWins = 7,
    kTeamBattleKills = 8,
    kTeamBattleDoubleKills = 9,
    kIndividualBattleWins = 10,
    kIndividualBattleKills = 11,
    kIndividualBattleMultiKills = 12,
    kLegacyAccumulatedWins = 13,
};

// Pointer-free request/reply.  `legacy_seed_wins` is captured by the game
// producer before the current match mutates legacy pvp_result, preventing a
// delayed queue-6 INSERT from double-counting that match.
struct Message {
    int32_t user_idx;
    uint32_t requester_account_id;
    uint32_t requester_charac_no;
    uint32_t target_charac_no;
    int32_t server_group;
    uint32_t legacy_seed_wins;
    uint32_t kill_count;
    uint16_t target_unique_id;
    uint8_t operation;
    uint8_t result;
    uint8_t match_mode;
    uint8_t won;
    uint8_t lost;
    uint8_t reply_required;
    uint32_t score[kScoreFieldCount];
};

static_assert(offsetof(Message, score) == 36u && sizeof(Message) == 92u,
              "integrated-match DB message ABI drift");

class DB_IntegratedMatchPvpScore : public DBDispatcher {
public:
    DB_IntegratedMatchPvpScore();
    virtual ~DB_IntegratedMatchPvpScore();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static bool Queue(int user_idx, const Message& request);
};

}  // namespace integrated_match_pvp_score_db_0828

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_INTEGRATED_MATCH_PVP_SCORE_0828_H_
