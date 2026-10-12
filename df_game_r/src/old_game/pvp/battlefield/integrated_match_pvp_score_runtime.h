#ifndef OLDSRV_OLD_GAME_PVP_INTEGRATED_MATCH_PVP_SCORE_RUNTIME_H_
#define OLDSRV_OLD_GAME_PVP_INTEGRATED_MATCH_PVP_SCORE_RUNTIME_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;

namespace integrated_match_pvp_score {

// SQL003 owns fourteen persisted counters.  current_streak is deliberately
// server-only; the other thirteen fields are emitted in the exact CMD371
// client-reader order.
struct Score {
    Score();

    uint32_t one_on_one_wins;
    uint32_t one_on_one_current_streak;
    uint32_t one_on_one_max_streak;
    uint32_t one_on_one_perfect_wins;
    uint32_t leader_battle_wins;
    uint32_t leader_battle_kills;
    uint32_t leader_battle_double_kills;
    uint32_t team_battle_wins;
    uint32_t team_battle_kills;
    uint32_t team_battle_double_kills;
    uint32_t individual_battle_wins;
    uint32_t individual_battle_kills;
    uint32_t individual_battle_multi_kills;
    uint32_t legacy_accumulated_wins;
};

enum MatchMode {
    MATCH_MODE_INVALID = -1,
    MATCH_MODE_ONE_ON_ONE = 0,
    MATCH_MODE_LEADER_BATTLE = 1,
    MATCH_MODE_TEAM_BATTLE = 2,
    MATCH_MODE_INDIVIDUAL_BATTLE = 3
};

// RECONSTRUCTED_BUSINESS: 0828 matchingmode.etc names four integrated-match
// record groups, while the restored room supplies raw battle modes 1..5.
// Team/death-team rooms are 1v1 only when both battle teams contain exactly
// one participant.
MatchMode ResolveMatchMode(int raw_battle_mode,
                           int team_one_count,
                           int team_two_count);

bool QueueLoad(CUser* requester,
               CUser* target,
               uint16_t target_unique_id);

// `lost` is separate from `!won`: a draw must not reset the 1v1 streak.
// PERFECT is intentionally read-only until a trusted server-side producer is
// proven.  kill>=2 occurrence counters are a documented reconstruction.
bool Record(CUser* user,
            MatchMode mode,
            bool won,
            bool lost,
            uint32_t kill_count,
            uint32_t legacy_seed_wins);

}  // namespace integrated_match_pvp_score


class Dispatcher_IntegrateMatchPvPScore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_IntegrateMatchPvPScore();
    virtual ~Dispatcher_IntegrateMatchPvPScore();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_IntegrateMatchPvPScore) == 0x04u,
              "CMD371 dispatcher must stay vptr-only on i386");
#endif


#endif  // OLDSRV_OLD_GAME_PVP_INTEGRATED_MATCH_PVP_SCORE_RUNTIME_H_
