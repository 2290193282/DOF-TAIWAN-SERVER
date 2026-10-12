#include "old_game/pvp/battlefield/integrated_match_pvp_score_runtime.h"

#include <stdint.h>
#include <cstring>

#include "common/manager/global_tools.h"
#include "old_data/db/pvp_party/db_integrated_match_pvp_score.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"


namespace integrated_match_pvp_score {
namespace {

uint32_t CurrentLegacyWins(CUser* user) {
    if (!user || !user->getPVPResultR()) {
        return 0u;
    }
    const int wins = user->getPVPResultR()->values[0];
    return wins > 0 ? static_cast<uint32_t>(wins) : 0u;
}

void InitMessage(
    integrated_match_pvp_score_db::Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.result = integrated_match_pvp_score_db::kResultRequest;
}

}  // namespace


Score::Score()
    : one_on_one_wins(0),
      one_on_one_current_streak(0),
      one_on_one_max_streak(0),
      one_on_one_perfect_wins(0),
      leader_battle_wins(0),
      leader_battle_kills(0),
      leader_battle_double_kills(0),
      team_battle_wins(0),
      team_battle_kills(0),
      team_battle_double_kills(0),
      individual_battle_wins(0),
      individual_battle_kills(0),
      individual_battle_multi_kills(0),
      legacy_accumulated_wins(0) {}

MatchMode ResolveMatchMode(int raw_battle_mode,
                           int team_one_count,
                           int team_two_count) {
    // RECONSTRUCTED_BUSINESS: matchingmode.etc provides four presentation
    // groups but no server enum map.  This mapping is derived from the 0627
    // room mode semantics and the 0828 integrated-match UI grouping.
    if (raw_battle_mode == 3) {
        return MATCH_MODE_LEADER_BATTLE;
    }
    if (raw_battle_mode == 1 || raw_battle_mode == 4) {
        return MATCH_MODE_INDIVIDUAL_BATTLE;
    }
    if (raw_battle_mode == 2 || raw_battle_mode == 5) {
        return team_one_count == 1 && team_two_count == 1
                   ? MATCH_MODE_ONE_ON_ONE
                   : MATCH_MODE_TEAM_BATTLE;
    }
    return MATCH_MODE_INVALID;
}

bool QueueLoad(CUser* requester,
               CUser* target,
               uint16_t target_unique_id) {
    if (!requester || !target || requester->getCurCharacNo() <= 0 ||
        target->getCurCharacNo() <= 0 || target_unique_id == 0u) {
        return false;
    }
    integrated_match_pvp_score_db::Message message;
    InitMessage(message);
    // DISPATCH_Q resolves its routing identity through
    // CGameManager::getUser(), whose key is the composite GetUID()
    // (pool-slot << 16 | increment id).  The world unique id is only the
    // u16 protocol field echoed to CMD371; using it as the queue user index
    // makes every asynchronous reply miss the requester.
    message.user_idx = requester->GetUID();
    message.requester_account_id = requester->get_acc_id();
    message.requester_charac_no =
        static_cast<uint32_t>(requester->getCurCharacNo());
    message.target_charac_no =
        static_cast<uint32_t>(target->getCurCharacNo());
    message.server_group = target->GetServerGroup();
    message.legacy_seed_wins = CurrentLegacyWins(target);
    message.target_unique_id = target_unique_id;
    message.operation =
        integrated_match_pvp_score_db::kOperationLoad;
    message.reply_required = 1u;
    return integrated_match_pvp_score_db::
        DB_IntegratedMatchPvpScore::Queue(message.user_idx, message);
}

bool Record(CUser* user,
            MatchMode mode,
            bool won,
            bool lost,
            uint32_t kill_count,
            uint32_t legacy_seed_wins) {
    if (!user || user->getCurCharacNo() <= 0 ||
        mode == MATCH_MODE_INVALID) {
        return false;
    }
    integrated_match_pvp_score_db::Message message;
    InitMessage(message);
    message.user_idx = user->GetUID();
    message.target_charac_no =
        static_cast<uint32_t>(user->getCurCharacNo());
    message.server_group = user->GetServerGroup();
    message.legacy_seed_wins = legacy_seed_wins;
    message.kill_count = kill_count;
    message.operation =
        integrated_match_pvp_score_db::kOperationRecord;
    message.match_mode = static_cast<uint8_t>(mode);
    message.won = won ? 1u : 0u;
    message.lost = lost ? 1u : 0u;
    return integrated_match_pvp_score_db::
        DB_IntegratedMatchPvpScore::Queue(message.user_idx, message);
}

}  // namespace integrated_match_pvp_score


Dispatcher_IntegrateMatchPvPScore::Dispatcher_IntegrateMatchPvPScore() {}

Dispatcher_IntegrateMatchPvPScore::~Dispatcher_IntegrateMatchPvPScore() {}

int Dispatcher_IntegrateMatchPvPScore::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    uint16_t target_unique_id = 0;
    if (packet.get_short(target_unique_id) != 1) {
        return LineFunc(
            371,
            "virtual int Dispatcher_IntegrateMatchPvPScore::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CUser* target = 0;
    if (target_unique_id ==
        static_cast<uint16_t>(user->get_unique_id())) {
        target = user;
    } else {
        target = G_GameWorld()->find_from_world(target_unique_id);
    }
    if (!target) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INTEGRATE_MATCH_PVP_SCORE, 3);
        return 0;
    }

    if (!integrated_match_pvp_score::QueueLoad(
            user, target, target_unique_id)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INTEGRATE_MATCH_PVP_SCORE, 1);
    }
    return 0;
}

int Dispatcher_IntegrateMatchPvPScore::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_IntegrateMatchPvPScore::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
