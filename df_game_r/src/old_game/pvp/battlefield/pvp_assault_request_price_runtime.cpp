#include "old_game/pvp/battlefield/pvp_assault.h"

#include "common/config.h"
#include <cstdlib>

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/pvp/battlefield/pvp_assault_pool_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/pvp/battlefield/timer_assault_lifecycle.h"
#include "common/manager/system_time.h"

enum ENUM_ERROR : int {};


namespace {

bool IsRepeatEventing(int event_id) {
    CEventBase* const event = GlobalData::s_event_manager
                                  ? GlobalData::s_event_manager->GetRepeatEvent(
                                        event_id)
                                  : 0;
    return event && event->IsEventing(0) != 0;
}

bool IsStayGate(CUser* user) {
    GameWorld* const world = G_GameWorld();
    if (!world) return true;
    Village* const village = world->getUserVillage(user);
    return !village || user->get_area(false) == village->get_gate_area();
}

void ThrowAssaultError(int error) {
    throw static_cast<ENUM_ERROR>(error);
}

void CheckAvailableAssault(CUser* checked, const CUser* requester) {
    int unique_id = 0;
    CParty* const party = checked->CheckInParty() ? checked->GetParty() : 0;
    const int minimum_age = CPowerManager::IsPowerWarEventOn() &&
                                    checked->getCurCharacVill() == 7
                                ? 12
                                : 15;

    if (checked->CheckLimitMiniumAge(minimum_age, unique_id) != 1)
        ThrowAssaultError(95);
    if (checked->get_charac_level() <= 29) {
        unique_id = static_cast<uint16_t>(checked->get_unique_id());
        ThrowAssaultError(checked == requester ? 13 : 14);
    }
    if (!party) return;

    if (party->is_limit_minimum_age(minimum_age, unique_id) != 1)
        ThrowAssaultError(95);
    if (party->is_limit_minimum_level(30, unique_id) != 1)
        ThrowAssaultError(checked == requester ? 254 : 255);
}

}  // namespace

namespace pvp_assault {

// IDA_EA: 0x82F0022
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16AvailableAssaultEP5CUserPKS1_
// DEMANGLED: pvp_assault::CAssaultMgr::AvailableAssault(CUser *,CUser const*)
char CAssaultMgr::AvailableAssault(CUser* user, const CUser* requester) {
    CheckAvailableAssault(user, requester);
    return 0;
}

// IDA_EA: 0x82EDC86
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr26GetRequestAssaultNeedMoneyEP5CUserS2_
int CAssaultMgr::GetRequestAssaultNeedMoney(CUser* requester, CUser* target) {
    if (IsRepeatEventing(32)) return 0;
    if (CPowerManager::IsPowerWarEventOn() &&
        static_cast<unsigned char>(requester->getCurCharacVill()) == 7u)
        return 0;

    CDataManager* const data = G_CDataManager();
    const ServerParameterScript* const parameter =
        data->get_server_parameter_script();
    const int base_price = parameter->getAssaultPrice(
        requester->get_charac_level());

    if (target->GetCurCharacChaosPoint()) {
        return static_cast<int>(
            static_cast<long double>(base_price) *
            parameter->getChaosPointWeight(target->GetCurCharacChaosPoint()));
    }

    const long double auto_client_weight =
        static_cast<long double>(target->GetAutoClientRate()) / -100.0L +
        1.0L;
    long double elapsed = static_cast<long double>(
        GlobalData::s_systemTime_.getCurSec() -
        requester->GetCurCharacChaosKillTime());
    if (elapsed > 0.0L) {
        if (static_cast<int>(elapsed) >
            data->get_assault_price_time_limit_seconds()) {
            elapsed = data->get_assault_price_time_limit_seconds();
        }
    } else {
        elapsed += 1.0L;
    }

    long double time_window = static_cast<long double>(
                                  data->get_assault_price_time_limit_seconds()) *
                              auto_client_weight;
    if (time_window <= 0.0L) time_window += 1.0L;

    long double time_weight = 1.0L;
    if (elapsed - time_window <= 0.0L) {
        time_weight =
            static_cast<long double>(data->get_assault_price_time_weight_limit()) -
            elapsed / time_window;
    }
    if (time_weight >= 1.0L) {
        const long double maximum =
            static_cast<long double>(data->get_assault_price_time_weight_limit());
        if (time_weight > maximum) time_weight = maximum;
    } else {
        time_weight = 1.0L;
    }

    long double level_weight = 1.0L;
    const int level_gap = requester->get_charac_level() -
                          target->get_charac_level();
    if (level_gap > 8) {
        level_weight =
            static_cast<long double>(data->get_assault_price_level_gap_weight()) *
                static_cast<long double>(level_gap - 8) +
            1.0L;
    }

    const int weighted_price = static_cast<int>(
        static_cast<long double>(base_price) * time_weight * level_weight *
        auto_client_weight);
    return static_cast<int>(
        static_cast<long double>(weighted_price) *
        parameter->getAssaultPriceChaospointWeight(
            requester->GetCurCharacChaosPoint()));
}

// IDA_EA: 0x82EDFBA
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16OnRequestAssaultEP5CUserS2_Pi
int CAssaultMgr::OnRequestAssault(CUser* requester,
                                  CUser* target,
                                  int* preflight_result) {
    if (requester->isCompetitionMercenary()) ThrowAssaultError(234);
    if (target->isCompetitionMercenary()) ThrowAssaultError(21);

    const bool power_war = CPowerManager::IsPowerWarEventOn() != 0;
    if (!power_war &&
        requester->GetCurCharacAssaultedUserId() != target->get_unique_id())
        ThrowAssaultError(19);

    bool party_prevent = false;
    if (expert_job::CAlchemist::GetAssaultPrevent(requester,
                                                    &party_prevent)) {
        if (!power_war || requester->getCurCharacVill() != 7)
            ThrowAssaultError(party_prevent ? 253 : 251);
    }
    if (GlobalData::s_villageMonsterMgr && GlobalData::s_villageMonsterMgr->OnEvent())
        ThrowAssaultError(217);
    // The direct CMD10 path, but not CMD447's quote preflight, is counted as
    // a PowerWar assault attempt before the remaining common validation.
    if (power_war && requester->getCurCharacVill() == 7 &&
        !preflight_result) {
        GlobalData::s_power_manager->GetPowerWarLog()->IncAssaultAttemptCount();
    }
    if (IsRepeatEventing(42)) ThrowAssaultError(178);
    if (requester->getCurCharacVill() == 8) ThrowAssaultError(19);

    if (power_war) {
        if (requester->getCurCharacVill() != 7) ThrowAssaultError(252);
        if (G_GameWorld()->IsPowerWarSafeZone(
                requester->getCurCharacVill(), requester->get_area(false)))
            ThrowAssaultError(19);
        if (requester->getPowerSide() == target->getPowerSide() ||
            !requester->isJoinPowerWar() || !target->isJoinPowerWar())
            ThrowAssaultError(19);
        CParty* const party = requester->GetParty();
        if (party && party->CanDoPartyActionAssalutState() != 1)
            ThrowAssaultError(19);
        if (target->GetCurCharacSuperState()) ThrowAssaultError(19);
    } else if (requester->getCurCharacVill() == 7) {
        ThrowAssaultError(19);
    }

    if (!power_war && expert_job::CAlchemist::GetAssaultPrevent(target, 0))
        ThrowAssaultError(17);
    if (G_CEnvironment()->get_server_group() == 3 &&
        (target->getCurCharacNo() == 2009613 ||
         target->getCurCharacNo() == 2905512))
        ThrowAssaultError(19);
    if (!assault_channel && target->getCurrCharacBlackCount() <= 99 &&
        !target->GetCurCharacChaosPoint())
        ThrowAssaultError(8);
    if (requester->GetAssaultPlace()) ThrowAssaultError(19);
    if (target->GetAssaultPlace()) ThrowAssaultError(18);
    if (!requester->getCurCharacR() || !requester->getCurCharacInvenR())
        ThrowAssaultError(19);
    if (requester->getCurCharacNo() == target->getCurCharacNo())
        ThrowAssaultError(19);
    if (std::abs(requester->get_posX() - target->get_posX()) > 100 ||
        std::abs(requester->get_posY() - target->get_posY()) > 100)
        ThrowAssaultError(99);
    if (requester->getCurCharacVill() != target->getCurCharacVill() ||
        requester->get_area(false) != target->get_area(false))
        ThrowAssaultError(19);
    if (!power_war && (IsStayGate(requester) || IsStayGate(target)))
        ThrowAssaultError(19);

    private_store::CPrivateStore* const requester_store =
        private_store::GetInstancePrivateStoreMgr()->FindStoreSeller(requester);
    if (requester_store && requester_store->isDollCreated() != 1)
        ThrowAssaultError(60);
    private_store::CPrivateStore* const target_store =
        private_store::GetInstancePrivateStoreMgr()->FindStoreSeller(target);
    if (target_store && target_store->isDollCreated() != 1)
        ThrowAssaultError(60);

    CParty* const requester_party = requester->CheckInParty()
                                        ? requester->GetParty()
                                        : 0;
    if (requester_party && requester_party->IsExistUser(target))
        ThrowAssaultError(19);
    if (power_war && target->getCurCharacVill() == 7) {
        CParty* const target_party = target->GetParty();
        if (target_party && target_party->IsThereGhostInParty())
            ThrowAssaultError(19);
    }

    CheckAvailableAssault(requester, requester);
    CheckAvailableAssault(target, requester);

    const int money = requester->getCurCharacInvenR()->get_money();
    const int quoted_money = requester->GetCurCharacAssaultMoney();
    if (money < quoted_money) ThrowAssaultError(10);

    if (preflight_result) return 1;

    CAssaultPlace* const place =
        GetInstanceAssaultPlaceFactory()->AcquireAssaultPlace();
    if (!place) ThrowAssaultError(22);

    place->Reset();
    const unsigned int assault_place_id = _GetNextAssaultPlaceId();
    place->SetAssaultPlaceId(assault_place_id);
    place->MakeTeam(requester, target);
    assault_places.insert(std::make_pair(assault_place_id, place));
    TimerCompleteLoadAssaultTimeout::registNextTimer(
        60, place->GetAssaultPlaceId(), place->GetAuthenValue());

    int charged_money = quoted_money;
    if (power_war && requester->getCurCharacVill() == 7) {
        charged_money = 0;
    }
    if (charged_money > 0) {
        const Inven_Item head_equipment =
            requester->getCurCharacInvenR()->GetInvenSlot(
                0, oldsrv::config::kEquipmentTypeTitle);
        CEquipItem* const equip_item = static_cast<CEquipItem*>(
            G_CDataManager()->find_item(head_equipment.item_id_));
        if (equip_item) {
            charged_money = static_cast<int>(
                equip_item->getUpgradeCostDiscount() *
                    static_cast<float>(charged_money) / -100.0f +
                static_cast<float>(charged_money));
        }
        requester->getCurCharacInvenW()->use_money(
            charged_money, static_cast<eMoneySubReason>(21), true);
        requester->SetCurCharacAssaultMoney(0);
        requester->SetCurCharacAssaultedUserId(0);
    }

    requester->IncCurCharacAssaultCount();
    if (power_war && requester->getCurCharacVill() == 7) {
        GlobalData::s_power_manager->GetPowerWarLog()->IncTotalJoinAssaultUser(
            place->GetAssaulterCount());
        GlobalData::s_power_manager->GetPowerWarLog()
            ->IncAssaultAttemptSuccessCount();
    }

    LogManager::logFormat(
        1,
        "PvpAssault.cpp",
        "pvp_assault::CAssaultMgr::OnRequestAssault",
        0,
        "[ASSAULT_DIAG][REQUEST_ACCEPT] place=%u members=%u charged=%d "
        "power_war=%d result=1",
        assault_place_id,
        static_cast<unsigned int>(place->GetAssaulterCount()),
        charged_money,
        power_war ? 1 : 0);

    _SendVictoryInfo(requester, target);
    PacketGuard packet;
    packet.put_header(0, 8);
    packet.put_short(target->get_unique_id());
    packet.put_byte(4);
    packet.put_int(charged_money);
    packet.finalize(true);
    requester->Send(packet);
    return 1;
}

// IDA_EA: 0x82ED976
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr22_GetNextAssaultPlaceIdEv
unsigned int CAssaultMgr::_GetNextAssaultPlaceId() {
    ++next_assault_place_id;
    if (!next_assault_place_id) {
        ++next_assault_place_id;
    }
    return next_assault_place_id;
}

// IDA_EA: 0x82EFCB2
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16_SendVictoryInfoEP5CUserS2_
void CAssaultMgr::_SendVictoryInfo(CUser* requester, CUser* target) {
    PacketGuard packet;
    packet.put_header(0, 266);
    if (requester->getPowerSide() == 1) {
        packet.put_int(requester->GetPowerWarPartyVictory());
        packet.put_int(target->GetPowerWarPartyVictory());
    } else {
        packet.put_int(target->GetPowerWarPartyVictory());
        packet.put_int(requester->GetPowerWarPartyVictory());
    }
    packet.finalize(true);

    CParty* const requester_party = requester->GetParty();
    if (requester_party) {
        requester_party->send_to_party(packet);
    } else {
        requester->Send(packet);
    }
    CParty* const target_party = target->GetParty();
    if (target_party) {
        target_party->send_to_party(packet);
    } else {
        target->Send(packet);
    }
}

}  // namespace pvp_assault
