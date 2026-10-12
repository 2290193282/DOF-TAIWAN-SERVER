#include "old_game/pvp/battlefield/inter_integrated_match_pvp_score.h"

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/db/pvp_party/db_integrated_match_pvp_score.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"


namespace {

bool IsCurrentRequester(
    CUser* user,
    const integrated_match_pvp_score_db::Message& message) {
    return user && user->get_state() >= 3 && user->getCurCharacR() &&
           user->get_acc_id() == message.requester_account_id &&
           user->getCurCharacNo() > 0 &&
           static_cast<uint32_t>(user->getCurCharacNo()) ==
               message.requester_charac_no &&
           user->GetServerGroup() == message.server_group;
}

bool IsCurrentTarget(
    CUser* requester,
    const integrated_match_pvp_score_db::Message& message) {
    CUser* target = 0;
    if (requester &&
        static_cast<uint16_t>(requester->get_unique_id()) ==
            message.target_unique_id) {
        target = requester;
    } else if (G_GameWorld()) {
        target = G_GameWorld()->find_from_world(message.target_unique_id);
    }
    return target && target->getCurCharacR() &&
           target->getCurCharacNo() > 0 &&
           static_cast<uint32_t>(target->getCurCharacNo()) ==
               message.target_charac_no;
}

}  // namespace


Inter_IntegratedMatchPvpScore::Inter_IntegratedMatchPvpScore()
    : ISigDispatcher() {}

Inter_IntegratedMatchPvpScore::~Inter_IntegratedMatchPvpScore() {}

int Inter_IntegratedMatchPvpScore::dispatch_sig(CUser* user,
                                                     char* payload,
                                                     int payload_size) {
    using namespace integrated_match_pvp_score_db;
    if (!payload || payload_size != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    const Message* const message =
        reinterpret_cast<const Message*>(payload);
    if (message->operation != kOperationLoad ||
        message->reply_required != 1u || !IsCurrentRequester(user, *message)) {
        return 0;
    }
    if (message->result != kResultSuccess ||
        !IsCurrentTarget(user, *message)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INTEGRATE_MATCH_PVP_SCORE, 1);
        return 0;
    }

    PacketGuard out;
    out.put_header(ENUM_PACKETCLASS_CMDPACKET,
                   ENUM_CMDPACKET_INTEGRATE_MATCH_PVP_SCORE);
    out.put_byte(1u);
    out.put_short(message->target_unique_id);
    out.put_int(message->score[kOneOnOneWins]);
    out.put_int(message->score[kOneOnOneMaxStreak]);
    out.put_int(message->score[kOneOnOnePerfectWins]);
    out.put_int(message->score[kLeaderBattleWins]);
    out.put_int(message->score[kLeaderBattleKills]);
    out.put_int(message->score[kLeaderBattleDoubleKills]);
    out.put_int(message->score[kTeamBattleWins]);
    out.put_int(message->score[kTeamBattleKills]);
    out.put_int(message->score[kTeamBattleDoubleKills]);
    out.put_int(message->score[kIndividualBattleWins]);
    out.put_int(message->score[kIndividualBattleKills]);
    out.put_int(message->score[kIndividualBattleMultiKills]);
    out.put_int(message->score[kLegacyAccumulatedWins]);
    out.finalize(true);
    user->Send(out);
    return 0;
}
