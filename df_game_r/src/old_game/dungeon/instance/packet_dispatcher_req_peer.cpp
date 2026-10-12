#include "old_game/dungeon/instance/packet_dispatcher_req_peer.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"


namespace {

const char kReqPeerSignature[] =
    "virtual int DisPatcher_ReqPeer::dispatch_sig(CUser*, PacketBuf&)";

int SendPeerError(CUser* user, uint8_t request_type, int error) {
    return user->SendCmdPeerErrorPacket(
        ENUM_CMDPACKET_REQUEST_PEER,
        static_cast<ENUM_PEER_REQUEST_TYPE>(request_type),
        static_cast<unsigned char>(error));
}

int SendPeerRequest(CUser* requester,
                    CUser* target,
                    uint8_t request_type,
                    int payload) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 7);
    packet.put_short(requester->get_unique_id());
    packet.put_byte(request_type);
    packet.put_int(payload);
    packet.finalize(true);
    return target->Send(packet);
}

int GetTargetOption(uint8_t request_type) {
    switch (request_type) {
    case 0:
        return 6;
    case 1:
        return 31;
    case 2:
        return 7;
    case 3:
        return 8;
    default:
        return 36;
    }
}

bool IsPowerWarLocationCompatible(CUser* requester, CUser* target) {
    if (!CPowerManager::IsPowerWarEventOn()) return true;

    const int requester_village = requester->getCurCharacVill();
    const int target_village = target->getCurCharacVill();
    if (requester_village == 7 && target_village == 7)
        return requester->getPowerSide() == target->getPowerSide();
    return requester_village != 7 && target_village != 7;
}

bool IsAssaultTryAvailable(CUser* requester, CUser* target) {
    const int current_second = GlobalData::s_systemTime_.getCurSec();
    CParty* const requester_party = requester->GetParty();
    CParty* const target_party = target->GetParty();

    const bool target_side_available =
        !expert_job::CAlchemist::GetAssaultPrevent(requester, 0) ||
        (target->GetCurCharacChaosKillTime() + 600 <= current_second &&
         (!target_party || !target_party->CheckMemberLastTryAssaultTime()));

    const bool requester_side_available =
        requester->GetCurCharacChaosKillTime() + 600 <= current_second ||
        (!expert_job::CAlchemist::GetAssaultPrevent(target, 0) &&
         (!requester_party ||
          !requester_party->CheckMemberLastTryAssaultTime() ||
          !expert_job::CAlchemist::GetAssaultPrevent(target, 0)));

    return CPowerManager::IsPowerWarEventOn() ||
           (target_side_available && requester_side_available);
}

bool SendAssaultPeerRequest(CUser* requester,
                            CUser* target,
                            int input_value,
                            uint16_t extension,
                            int requester_nat_type,
                            int target_nat_type) {
    if (requester->get_state() == 5 || !requester_nat_type) {
        SendPeerError(requester, 0, 19);
        return false;
    }
    if (target->isBlackUser(requester->get_charac_no(-1))) {
        SendPeerError(requester, 0, 77);
        return false;
    }
    if (requester->isCompetitionMercenary()) {
        SendPeerError(requester, 0, 234);
        return false;
    }
    if (target->isCompetitionMercenary()) {
        SendPeerError(requester, 0, 190);
        return false;
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(target)) {
        SendPeerError(requester, 0, 61);
        return false;
    }
    if (target->IsCurCharacGhost()) {
        SendPeerError(requester, 0, 19);
        return false;
    }
    if (target->get_state() == 5 || !target_nat_type) {
        SendPeerError(requester, 0, 3);
        return false;
    }
    if (!IsPowerWarLocationCompatible(requester, target)) {
        SendPeerError(requester, 0, 19);
        return false;
    }

    CParty* const party = requester->GetParty();
    if (party && (party->get_member_count() > 3 ||
                  !party->CanDoPartyActionAssalutState())) {
        SendPeerError(requester, 0, 19);
        return false;
    }
    if (requester->CheckInParty() && target->CheckInParty()) {
        SendPeerError(requester, 0, 18);
        return false;
    }
    if (!IsAssaultTryAvailable(requester, target)) {
        SendPeerError(requester, 0, 250);
        return false;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 7);
    packet.put_short(requester->get_unique_id());
    packet.put_byte(0);
    packet.put_int(input_value);
    packet.put_short(requester->getCurCharacTotalMaxFatigue() -
                     requester->getCurCharacTotalFatigue());
    packet.put_short(requester->getCurCharacStamina());
    packet.put_short(extension);
    packet.finalize(true);
    target->Send(packet);
    return true;
}

}  // namespace

// IDA_EA: 0x85954D8
// IDA_NAME: _ZN18DisPatcher_ReqPeerC2Ev
// DEMANGLED: DisPatcher_ReqPeer::DisPatcher_ReqPeer(void)
DisPatcher_ReqPeer::DisPatcher_ReqPeer() {}

// IDA_EA: 0x8232DF2
// IDA_NAME: _ZN18DisPatcher_ReqPeerD2Ev
// DEMANGLED: DisPatcher_ReqPeer::~DisPatcher_ReqPeer()
// IDA_EA: 0x8232E22
// IDA_NAME: _ZN18DisPatcher_ReqPeerD0Ev
// DEMANGLED: DisPatcher_ReqPeer::~DisPatcher_ReqPeer()
DisPatcher_ReqPeer::~DisPatcher_ReqPeer() {}

// IDA_EA: 0x81EED08
// IDA_NAME: _ZN18DisPatcher_ReqPeer12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_ReqPeer::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_ReqPeer::dispatch_sig(CUser* user, PacketBuf& packet) {
    const int state = user->get_state();
    if (state != 3 && state != 6 && state != 8) return 0;

    PacketGuard output;
    MSG_REQ_TO_PEER request = {};
    if (packet.get_short(request.target_unique_id) != 1)
        return LineFunc(29274, kReqPeerSignature, 0, 0);
    if (packet.get_byte(request.request_type) != 1)
        return LineFunc(29275, kReqPeerSignature, 0, 0);
    if (packet.get_int(request.input_value) != 1)
        return LineFunc(29276, kReqPeerSignature, 0, 0);

    // The 140828 type-0 sender appends one required u16.  Every other proven
    // subtype ends after the common seven-byte body.  Parse the complete
    // branch before any target lookup or business mutation so a truncated or
    // forged-tail request cannot silently become an extension value of zero.
    uint16_t native_extension = 0;
    if (request.request_type == 0u &&
        packet.get_short(native_extension) != 1) {
        return LineFunc(29276, kReqPeerSignature, 0, 0);
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        SendPeerError(user, request.request_type, 60);
        return 0;
    }
    if (user->IsBusyDisjointStore()) {
        SendPeerError(user, request.request_type, 191);
        return 0;
    }
    if (user->IsCurCharacGhost() ||
        (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
         request.request_type != 2)) {
        SendPeerError(user, request.request_type, 19);
        return 0;
    }

    CUser* const target = G_GameWorld()->find_from_world(
        request.target_unique_id);
    if (!target) {
        SendPeerError(user, request.request_type, 3);
        return 0;
    }
    if (target->alonePlay_Mode(request)) {
        user->Send_SoloPlay_Reject_NotiPack(
            target->getCurCharacName(), 1, target->get_SoloPlay_msgType(request));
        return 0;
    }

    const int option = GetTargetOption(request.request_type);
    if (option != 36 && target->GetGameOptionRef()->get_etc_option(option)) {
        user->Send_SoloPlay_Reject_NotiPack(
            target->getCurCharacName(), 0, target->get_SoloPlay_msgType(request));
        return 0;
    }
    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        (target->get_state() == 6 || target->get_state() == 7)) {
        SendPeerError(user, request.request_type, 19);
        return 0;
    }
    if (target->IsBusyDisjointStore()) {
        SendPeerError(user, request.request_type, 192);
        return 0;
    }
    // CLIENT_IDA_EVIDENCE: the helper's char argument is this peer request
    // subtype; the helper itself owns the CMD REQUEST_PEER response header.
    if (pvp_assault::GetInstanceAssaultMgr()->check_party_teleport_user(
            static_cast<char>(request.request_type), user, target, true) > 0)
        return 0;

    switch (request.request_type) {
    case 0:
        if (!SendAssaultPeerRequest(user,
                                    target,
                                    request.input_value,
                                    native_extension,
                                    user->network_.get_nat_type(),
                                    target->network_.get_nat_type()))
            return 0;
        break;

    case 1: {
        ServiceRestrictManager* const restrict_manager =
            ARAD::Singleton<ServiceRestrictManager>::Get();
        if (restrict_manager->isRestricted(
                user, RestrictCategory::AuthMobile, 6) ||
            restrict_manager->isRestricted(
                target, RestrictCategory::AuthMobile, 6)) {
            SendPeerError(user, 1, 209);
            return 0;
        }
        const int protection = GlobalData::s_pSecuProtectionField->CheckOpposite(
            user, static_cast<SECURITY_PROTCTION>(54), target);
        if (protection) {
            SendPeerError(user, 1, protection);
            return 0;
        }
        if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(target)) {
            SendPeerError(user, 1, 61);
            return 0;
        }
        if (target->IsCurCharacGhost()) {
            SendPeerError(user, 1, 19);
            return 0;
        }
        if (target->isBlackUser(user->get_charac_no(-1))) {
            SendPeerError(user, 1, 77);
            return 0;
        }
        if (target->CheckInTrade()) {
            SendPeerError(user, 1, 18);
            return 0;
        }
        if (user->get_state() != target->get_state()) {
            SendPeerError(user, 1, 1);
            return 0;
        }
        PacketGuard trade_request;
        trade_request.clear();
        trade_request.put_header(0, 7);
        trade_request.put_short(user->get_unique_id());
        trade_request.put_byte(1);
        trade_request.put_int(request.input_value);
        trade_request.put_int(user->getCurCharacCreateTime());
        trade_request.finalize(true);
        target->Send(trade_request);
        break;
    }

    case 2:
        if (!user->GetPVPRoom())
            return LineFunc(29844, kReqPeerSignature, 0, 0);
        if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(target)) {
            SendPeerError(user, 2, 61);
            return 0;
        }
        if (target->IsCurCharacGhost()) {
            SendPeerError(user, 2, 19);
            return 0;
        }
        if (target->isCompetitionMercenary()) {
            SendPeerError(user, 2, 190);
            return 0;
        }
        if (target->get_state() > 4) return 0;
        SendPeerRequest(user, target, 2, request.target_unique_id);
        break;

    case 3: {
        WarRoom* const room = user->GetWarRoom();
        if (!room) return LineFunc(29887, kReqPeerSignature, 0, 0);
        if (target->get_state() > 4 || room->GetState() == 2) return 0;
        SendPeerRequest(user, target, 3, request.target_unique_id);
        break;
    }

    case 4: {
        const int protection = GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(55));
        if (protection) {
            SendPeerError(user, 4, protection);
            return 0;
        }
        if (target->get_state() > 4) return 0;
        if (!user->isGMUser() && target->isGMUser()) return 0;
        if (pvp_assault::GetInstanceAssaultMgr()->OnRequestAssault(
                user, target, 0) != 1)
            return 30096;
        break;
    }

    case 5:
    case 7: {
        ServiceRestrictManager* const restrict_manager =
            ARAD::Singleton<ServiceRestrictManager>::Get();
        if (restrict_manager->isRestricted(
                user, RestrictCategory::AuthMobile, 8)) {
            SendPeerError(user, request.request_type, 209);
            return 0;
        }
        const int protection = GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(33));
        if (protection) {
            SendPeerError(user, request.request_type, protection);
            return 0;
        }
        if (target->GetCurCharacExpertJobType() != 1) {
            SendPeerError(user, request.request_type, 23);
            return 0;
        }
        GlobalData::s_ExpertJobMgr->OnRequestPeer(
            user,
            target,
            static_cast<ENUM_PEER_REQUEST_TYPE>(request.request_type),
            output);
        break;
    }

    case 6:
    case 8: {
        ServiceRestrictManager* const restrict_manager =
            ARAD::Singleton<ServiceRestrictManager>::Get();
        if (restrict_manager->isRestricted(
                user, RestrictCategory::AuthMobile, 8)) {
            SendPeerError(user, request.request_type, 209);
            return 0;
        }
        const int protection = GlobalData::s_pSecuProtectionField->CheckOpposite(
            user, static_cast<SECURITY_PROTCTION>(32), target);
        if (protection) {
            SendPeerError(user, request.request_type, protection);
            return 0;
        }
        if (user->GetCurCharacExpertJobType() != 1) {
            SendPeerError(user, request.request_type, 23);
            return 0;
        }
        GlobalData::s_ExpertJobMgr->OnRequestPeer(
            user,
            target,
            static_cast<ENUM_PEER_REQUEST_TYPE>(request.request_type),
            output);
        break;
    }

    default:
        break;
    }

    target->SetReceivedRequestType(static_cast<char>(request.request_type));
    return 0;
}

// IDA_EA: 0x8232DDE
// IDA_NAME: _ZN18DisPatcher_ReqPeer4readER9PacketBufR8MSG_BASE
int DisPatcher_ReqPeer::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232DE8
// IDA_NAME: _ZN18DisPatcher_ReqPeer7processEP5CUserR8MSG_BASER9ParamBase
int DisPatcher_ReqPeer::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
