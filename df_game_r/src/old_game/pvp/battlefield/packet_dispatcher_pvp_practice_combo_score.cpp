#include "old_game/pvp/battlefield/packet_dispatcher_pvp_practice_combo_score.h"

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/pvp_party/db_pvp_practice_combo_score.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

#include <stdint.h>

namespace {

bool IsComboTrainingContext(CUser* user) {
    if (!user || user->get_state() != 5 || !user->getCurCharacR() ||
        user->getCurCharacNo() <= 0) {
        return false;
    }
    CParty* const party = user->GetParty();
    CMap* const map = party ? party->battle_field().GetCurrentMap() : 0;
    return map && map->isComboTraining();
}

void SendComboScore(CUser* user, uint16_t score_index, uint32_t score) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_COMBO_SCORE_INFO);
    packet.put_short(static_cast<int>(score_index));
    packet.put_int(static_cast<int>(score));
    packet.finalize(true);
    user->Send(packet);
}

int ReadNoMessage(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int ProcessNoMessage(CUser* user, MSG_BASE& message, ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

}  // namespace

Dispatcher_RequestComboScore::Dispatcher_RequestComboScore() {}
Dispatcher_RequestComboScore::~Dispatcher_RequestComboScore() {}

int Dispatcher_RequestComboScore::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    unsigned short score_index = 0xffffu;
    if (packet.get_short(score_index) != 1 ||
        score_index >= pvp_practice_combo_score::kScoreCount ||
        !IsComboTrainingContext(user)) {
        return 0;
    }
    (void)pvp_practice_combo_score::QueueQuery(
        user->GetUID(),
        user->GetServerGroup(),
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<uint16_t>(score_index));
    return 0;
}

int Dispatcher_RequestComboScore::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    return ReadNoMessage(packet, message);
}

int Dispatcher_RequestComboScore::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    return ProcessNoMessage(user, message, param);
}

Dispatcher_ComboScoreInfo::Dispatcher_ComboScoreInfo() {}
Dispatcher_ComboScoreInfo::~Dispatcher_ComboScoreInfo() {}

int Dispatcher_ComboScoreInfo::dispatch_sig(CUser* user,
                                             PacketBuf& packet) {
    unsigned short score_index = 0xffffu;
    unsigned int score = 0u;
    if (packet.get_short(score_index) != 1 || packet.get_int(score) != 1 ||
        score_index >= pvp_practice_combo_score::kScoreCount ||
        score == 0u || !IsComboTrainingContext(user)) {
        return 0;
    }
    // RECONSTRUCTED_SECURITY_BOUNDARY: the 0828 client computes this score.
    // The server can prove context and monotonicity, but the original score
    // formula/anti-tamper validation has not survived in the 0627 owner.
    (void)pvp_practice_combo_score::QueueMonotonicUpdate(
        user->GetUID(),
        user->GetServerGroup(),
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<uint16_t>(score_index),
        static_cast<uint32_t>(score));
    return 0;
}

int Dispatcher_ComboScoreInfo::read(PacketBuf& packet,
                                     MSG_BASE& message) {
    return ReadNoMessage(packet, message);
}

int Dispatcher_ComboScoreInfo::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    return ProcessNoMessage(user, message, param);
}

Inter_PvpPracticeComboScore::Inter_PvpPracticeComboScore()
    : ISigDispatcher() {}

Inter_PvpPracticeComboScore::~Inter_PvpPracticeComboScore() {}

int Inter_PvpPracticeComboScore::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    using namespace pvp_practice_combo_score;
    if (!payload || payload_size != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    const Message* const response =
        reinterpret_cast<const Message*>(payload);
    if (!IsComboTrainingContext(user) ||
        response->operation != OPERATION_QUERY ||
        response->result != kResultOk ||
        response->score_index >= kScoreCount ||
        response->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo()) ||
        response->server_group != user->GetServerGroup()) {
        // user_idx can be recycled while DB work is pending.  Both stable
        // identity fields must still match before a NOTI514 is emitted.
        return 0;
    }
    SendComboScore(user, response->score_index, response->score);
    return 0;
}
