#include "old_game/dungeon/instance/packet_dispatcher_res_peer.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"


namespace {

const char kResPeerSignature[] =
    "virtual int DisPatcher_ResPeer::dispatch_sig(CUser*, PacketBuf&)";

int SendResponseError(CUser* user, int response_type, int error) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_RESPONSE_PEER);
    packet.put_byte(0);
    packet.put_byte(error);
    packet.put_byte(response_type);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

void SendAssaultJoinNotice(CUser* target,
                           CUser* responder,
                           uint16_t target_unique_id) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_RESPONSE_PEER);
    packet.put_short(responder->get_unique_id());
    packet.put_byte(0);
    packet.put_int(target_unique_id);
    packet.finalize(true);
    target->Send(packet);
}

int GetPartyJoinError(int result) {
    if (result == -2) return 19;
    if (result == -1) return 4;
    return 1;
}

int GetAssaultJoinAvailabilityError(CUser* responder, CUser* target) {
    if (CPowerManager::IsPowerWarEventOn()) {
        const int responder_village = responder->getCurCharacVill();
        const int target_village = target->getCurCharacVill();
        if (responder_village == 7 && target_village == 7)
            return responder->getPowerSide() == target->getPowerSide() ? 0 : 19;
        return responder_village != 7 && target_village != 7 ? 0 : 19;
    }

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    const bool target_recent_assault =
        target->GetCurCharacChaosKillTime() + 600 > current_second &&
        expert_job::CAlchemist::GetAssaultPrevent(responder, 0);
    const bool responder_recent_assault =
        responder->GetCurCharacChaosKillTime() + 600 > current_second &&
        expert_job::CAlchemist::GetAssaultPrevent(target, 0);
    CParty* const responder_party = responder->GetParty();
    const bool responder_party_recent_assault =
        responder_party && responder_party->CheckMemberLastTryAssaultTime() &&
        expert_job::CAlchemist::GetAssaultPrevent(target, 0);
    CParty* const target_party = target->GetParty();
    const bool target_party_recent_assault =
        target_party && target_party->CheckMemberLastTryAssaultTime() &&
        expert_job::CAlchemist::GetAssaultPrevent(responder, 0);

    return target_recent_assault || responder_recent_assault ||
                   responder_party_recent_assault || target_party_recent_assault
               ? 250
               : 0;
}

int JoinAssaultParty(CUser* responder,
                     CUser* target,
                     uint16_t target_unique_id) {
    CParty* party = 0;
    if (!responder->CheckInParty() && !target->CheckInParty()) {
        party = G_CGameManager()->GetParty();
        if (!party) return SendResponseError(responder, 0, 4);

        party->create_party(target);
        party->setDungIndex(0);
        party->setDungDiffi(5);
        party->join_user(responder);
        party->send_party_realtime_info();
        SendAssaultJoinNotice(target, responder, target_unique_id);
        party->send_party_ipinfo();
        G_GameWorld()->send_party_info_to_all(party, 0);
        return 0;
    }

    if (responder->CheckInParty() && !target->CheckInParty()) {
        party = responder->GetParty();
        if (!party) return SendResponseError(responder, 0, 4);
        if (!party->CanDoPartyActionAssalutState())
            return SendResponseError(responder, 0, 19);

        const int result = party->join_user(target);
        if (result <= 0)
            return SendResponseError(responder, 0, GetPartyJoinError(result));

        party->send_quick_party_matching_result(false, 0);
        party->set_quick_party(false);
        party->send_party_realtime_info();
        SendAssaultJoinNotice(target, responder, target_unique_id);
        party->send_party_ipinfo();
        G_GameWorld()->send_party_info_to_all(party, 2);
        return 0;
    }

    if (!responder->CheckInParty() && target->CheckInParty()) {
        party = G_CGameManager()->GetParty(target->GetPartyIndex());
        if (!party) return SendResponseError(responder, 0, 4);
        if (!party->CanDoPartyActionAssalutState())
            return SendResponseError(responder, 0, 19);

        const int result = party->join_user(responder);
        if (result <= 0)
            return SendResponseError(responder, 0, GetPartyJoinError(result));

        party->send_quick_party_matching_result(false, 0);
        party->set_quick_party(false);
        party->send_party_realtime_info();
        SendAssaultJoinNotice(target, responder, target_unique_id);
        party->send_party_ipinfo();
        G_GameWorld()->send_party_info_to_all(party, 2);
    }
    return 0;
}

}  // namespace

// IDA_EA: 0x8595510
// IDA_NAME: _ZN18DisPatcher_ResPeerC2Ev
// DEMANGLED: DisPatcher_ResPeer::DisPatcher_ResPeer(void)
DisPatcher_ResPeer::DisPatcher_ResPeer() {}

// IDA_EA: 0x8232EB6
// IDA_NAME: _ZN18DisPatcher_ResPeerD2Ev
// DEMANGLED: DisPatcher_ResPeer::~DisPatcher_ResPeer()
// IDA_EA: 0x8232EE6
// IDA_NAME: _ZN18DisPatcher_ResPeerD0Ev
// DEMANGLED: DisPatcher_ResPeer::~DisPatcher_ResPeer()
DisPatcher_ResPeer::~DisPatcher_ResPeer() {}

// IDA_EA: 0x81F088E
// IDA_NAME: _ZN18DisPatcher_ResPeer12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_ResPeer::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_ResPeer::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) return 0;

    MSG_RES_TO_PEER response = {};
    if (packet.get_short(response.target_unique_id) != 1)
        return LineFunc(30677, kResPeerSignature, 0, 0);
    if (packet.get_byte(response.response_type) != 1)
        return LineFunc(30678, kResPeerSignature, 0, 0);
    if (packet.get_int(response.input_value) != 1)
        return LineFunc(30679, kResPeerSignature, 0, 0);

    const int previous_type = user->GetReceivedRequestType();
    const int response_type = static_cast<int>(
        static_cast<int8_t>(response.response_type));
    const bool is_matching_response =
        ((previous_type == 5 || previous_type == 7) &&
         (response_type == 5 || response_type == 7)) ||
        ((previous_type == 6 || previous_type == 8) &&
         (response_type == 6 || response_type == 8)) ||
        ((previous_type != 5 && previous_type != 7 &&
          previous_type != 6 && previous_type != 8) &&
         response_type == previous_type);
    if (!is_matching_response) return 0;

    user->InitReceivedRequestType();
    switch (response_type) {
    case 0: {
        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        if (!target) return SendResponseError(user, 0, 3);
        if (target == user || target->getCurCharacNo() == user->getCurCharacNo())
            return 0;
        if (user->get_state() == 5 || target->get_state() == 5) return 0;

        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0)
            return SendResponseError(user, 0, assault_error);
        if (target->get_state() != 3) return SendResponseError(user, 0, 19);
        if (user->CheckInParty() && target->CheckInParty())
            return SendResponseError(user, 0, 18);
        if (user->isCompetitionMercenary() || target->isCompetitionMercenary()) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(0), 234);
        }
        const int availability_error = GetAssaultJoinAvailabilityError(
            user, target);
        if (availability_error == 19)
            return SendResponseError(user, 0, availability_error);
        if (availability_error == 250) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(0), 250);
        }
        return JoinAssaultParty(user, target, response.target_unique_id);
    }

    case 1: {
        if (user->CheckInTrade()) return 0;

        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        int error = -1;
        if (!target) {
            error = 3;
        } else if (target->CheckInTrade()) {
            error = 18;
        } else if (user->get_state() != target->get_state()) {
            error = 19;
        }
        if (error != -1) return SendResponseError(user, 1, error);

        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0)
            return SendResponseError(user, 1, assault_error);

        CTradeSpace* const trade_space = G_CGameManager()->GetTradeSpace();
        if (!trade_space) return SendResponseError(user, 1, 1);

        trade_space->set_traders(target, user);
        PacketGuard output;
        output.clear();
        output.put_header(0, ENUM_NOTIPACKET_RESPONSE_PEER);
        output.put_short(user->get_unique_id());
        output.put_byte(1);
        output.put_int(response.input_value);
        output.put_byte(user->GetReliablePerson());
        output.put_int(user->getCurCharacCreateTime());
        output.finalize(true);
        target->Send(output);

        output.clear();
        output.put_header(1, ENUM_CMDPACKET_RESPONSE_PEER);
        output.put_byte(1);
        output.put_short(response.target_unique_id);
        output.put_byte(1);
        output.put_byte(target->GetReliablePerson());
        output.finalize(true);
        user->Send(output);
        return 0;
    }

    case 2: {
        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        if (!target) return SendResponseError(user, 2, 3);

        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0)
            return SendResponseError(user, 2, assault_error);

        PvP_Room* const room = G_CGameManager()->GetPvp(
            target->GetPvpIndex(), user, 0);
        if (!room) return SendResponseError(user, 2, 19);

        int seat = 0;
        const int result = room->join_room(
            user, seat, G_GameWorld()->IsSameTeamChannelJoin());
        if (result > 0) return SendResponseError(user, 2, result);

        G_CGameManager()->CheckOutParty(user, false);
        PacketGuard output;
        room->make_seat_info(reinterpret_cast<char*>(&output), seat);
        G_GameWorld()->send_all(output);
        G_GameWorld()->goto_pvp(user);
        return 0;
    }

    case 3: {
        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        if (!target) return SendResponseError(user, 3, 3);

        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0)
            return SendResponseError(user, 3, assault_error);

        WarRoom* const room = G_CGameManager()->GetWarRoom(
            target->GetWarRoomIndex());
        if (!room) return SendResponseError(user, 3, 19);

        int slot = 0;
        int result = room->IsJoinable(user);
        if (result <= 0) result = room->Join(user, slot);
        if (result > 0) return SendResponseError(user, 3, result);

        G_CGameManager()->CheckOutParty(user, false);
        PacketGuard output;
        room->MakeSlotInfo(reinterpret_cast<char*>(&output), slot);
        G_GameWorld()->send_all(output);
        G_GameWorld()->goto_warroom(user);
        if (room->GetWaiterCount() > 2 && !room->GetState()) room->Start();
        return 0;
    }

    case 5:
    case 7: {
        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        if (!target) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type), 3);
        }
        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type),
                static_cast<unsigned char>(assault_error));
        }
        if (user->GetCurCharacExpertJobType() != 1) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type), 23);
        }
        PacketGuard output;
        GlobalData::s_ExpertJobMgr->OnResponsePeer(
            user,
            target,
            user,
            target,
            response.input_value,
            static_cast<ENUM_PEER_REQUEST_TYPE>(response_type),
            output);
        return 0;
    }

    case 6:
    case 8: {
        CUser* const target = G_GameWorld()->find_from_world(
            response.target_unique_id);
        if (!target) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type), 3);
        }
        const int assault_error =
            pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
                0, user, target, false);
        if (assault_error > 0) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type),
                static_cast<unsigned char>(assault_error));
        }
        if (target->GetCurCharacExpertJobType() != 1) {
            return user->SendCmdPeerErrorPacket(
                ENUM_CMDPACKET_RESPONSE_PEER,
                static_cast<ENUM_PEER_REQUEST_TYPE>(response_type), 23);
        }
        PacketGuard output;
        GlobalData::s_ExpertJobMgr->OnResponsePeer(
            user,
            target,
            target,
            user,
            response.input_value,
            static_cast<ENUM_PEER_REQUEST_TYPE>(response_type),
            output);
        return 0;
    }

    default:
        return 0;
    }
}

// IDA_EA: 0x8232EA2
// IDA_NAME: _ZN18DisPatcher_ResPeer4readER9PacketBufR8MSG_BASE
int DisPatcher_ResPeer::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232EAC
// IDA_NAME: _ZN18DisPatcher_ResPeer7processEP5CUserR8MSG_BASER9ParamBase
int DisPatcher_ResPeer::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
