#include "old_game/world/region/game_world.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "common/manager/fatigue_battery_handle_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/channel/arad_event_notification_runtime.h"
#include "old_data/channel/arad_server_state_message.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/legacy_dispatch/db_action_point_update.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/player/core/cbooster_gage_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/manager/system_time.h"

#include <map>
#include <stdint.h>
#include <string>

// The provider exists in global/secu_gold_control_abi.cpp.  Its canonical
// header deliberately exposes only the typed public operations; the original
// midnight path calls this exact member-symbol bridge directly.
namespace {

bool ShouldOpenAradEventPeriodDialog() {
    ARAD::Arad_DataManager* const arad_data =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get();
    ARAD::Arad_EventPeriodDataManager* const event_periods =
        static_cast<ARAD::Arad_EventPeriodDataManager*>(
            arad_data->findGameScript(
                std::string("Arad_EventPeriodDataManager")));
    return event_periods->isApplied(
        static_cast<ENUM_REPEAT_EVENT_CODE>(142),
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec()));
}

}  // namespace

// IDA_EA: 0x86CC3EC
// IDA_NAME: _ZN9GameWorld15ResetAllFatigueEv
// DEMANGLED: GameWorld::ResetAllFatigue(void)
int GameWorld::ResetAllFatigue() {
    // IDA global `_ZZN9GameWorld15ResetAllFatigueEvE7isFirst` is initialized
    // to one.  It only gates the first online user batch's period-dialogue
    // probe and is cleared after a non-empty directory traversal.
    static bool is_first = true;

    short daily_badges[3] = {};
    // Activity state is loaded from dnf_event_log during startup.  Keep
    // SetWeekendBounsEvent() available for explicit legacy/plugin callers,
    // but daily maintenance must not create a second calendar authority.
    king_of_pick::OnProperClearGlobalReset();
    shooting_star_dimension_pocket::OnProperClearGlobalReset();

    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() <= 2) {
            continue;
        }

        const int16_t charging = GlobalData::s_fatigueBatteryHandle_
            ->ChargingFatigueBattery(user, 0);
        user->IncFatigueBatteryCharging(charging);
        const int16_t active = GlobalData::s_fatigueBatteryHandle_
            ->ActiveFatigueBattery(user);
        if (active) {
            user->DecFatigueBatteryCharging(active);
            const int current_charging = user->GetFatigueBatteryCharging();
            cMyTrace trace("void GameWorld::ResetAllFatigue()", 4742, 0);
            trace("FATIGUE BATTERY ACTIVE: charno(%d) , "
                  "descFatigueAmount(%d), cur charging(%d)",
                  user->getCurCharacNo(), active, current_charging);
        }

        user->DBUpdateDBLogItem();
        user->ResetDBLogItem();
        user->RecoverFatigue(0);
        user->SendFatigue();
        user->resetCurCharacMemberBonusFatigue();
        user->resetAccountMemberBonusFatigue();
        user->resetAccountUsedFatigue();
        user->SetDailyBadge(daily_badges);
        user->ResetDailyCharacExpandData();
        static_cast<CBoosterGage*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(17)))->send_data(user, 0);
        user->send_MissionList();
        user->ClearCurConditionEventStep();
        user->ClearCurConditionEventRewardStep();
        user->SendConditionEventInfo();
        user->ClearProperDungeonClearCount();
        king_of_pick::OnProperClearSerialReset(user);
        shooting_star_dimension_pocket::OnProperClearSerialReset(user);
        user->SendProperDungeonClearCount();
        user->ResetRevengeDungeonClear();
        if (user->getCurCharacR()) {
            // Publish after both expand-data and revenge counters have been
            // reset so the keyed in/out map cannot expose a stale midnight
            // value.
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                 ENUM_NOTIPACKET_USERINFO,
                                 1);
        }
        user->resetNPCRelationShipDailyData();
        user->sendNPCRelationShipFavor();
        static_cast<CDungeonGainedGold*>(user->getDungeonGainedGold())->reset();

        const APSystem::_SIG_LOAD_ACTION_POINT* const action_point =
            user->action_point_manager_.GetActionPoint();
        APSystem::DB_UpdateActionPoint::makeRequest(
            user->GetUID(), *action_point, true);
        APSystem::CUserProc::SetTodayActionAndCheckMedalReward(user);
        reinterpret_cast<Secu_GoldControl*>((static_cast<Secu_GoldControl*>(user->GetGoldControl())))->SavetoDB(
            false, true, false);

        G_CDataManager()->get_event_script_mng()->send_event_init_data(
            user, true);
        user->resetBlueMarbleEnterCount();
        user->sendBlueMarbleEnterCount();
        user->ResetCurCharacUsedGiftFatigueQuantity();

        if (is_first && ShouldOpenAradEventPeriodDialog()) {
            AradServerStateMessage state;
            ARAD::notifyOpenMessageDialog(user, 0, 142, state);
        }

        user->setPcroomDailyRewardRemain(4);
        InterSelectPcroomDailyReward::SendPacket(user);
        ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
            user, HeroMissionCondition::MissionNo::ATTENDANCE, 0u);
    }

    const bool has_users = !users_in_world_.empty();
    if (has_users) {
        is_first = false;
    }
    return has_users;
}

// IDA_EA: 0x86CCC8E
// IDA_NAME: _ZN9GameWorld15DeleteDailyItemEv
// DEMANGLED: GameWorld::DeleteDailyItem(void)
int GameWorld::DeleteDailyItem() {
    const int start_tick = GlobalData::s_systemTime_.getCurTickCount();
    GameWorldUserMap& users = users_in_world_;
    cMyTrace start_trace("void GameWorld::DeleteDailyItem()", 5082, 0);
    start_trace("DeleteDailyItem size(%d) start tick(%u)",
                static_cast<unsigned char>(users.size()), start_tick);

    for (GameWorldUserMap::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->second) {
            it->second->processDelDailyItem();
        }
    }

    GlobalData::s_systemTime_.update();
    const int end_tick = GlobalData::s_systemTime_.getCurTickCount();
    cMyTrace end_trace("void GameWorld::DeleteDailyItem()", 5094, 0);
    return end_trace("DeleteDailyItem end tick(%u)", end_tick);
}

// IDA_EA: 0x86CCF58
// IDA_NAME: _ZN9GameWorld14resetDailyDataEv
// DEMANGLED: GameWorld::resetDailyData(void)
bool GameWorld::resetDailyData() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() > 1) {
            user->resetDailyData();
        }
    }

    // The original returns the terminal iterator comparison, which is false
    // whether the user map is empty or has been fully traversed.
    return false;
}

// IDA_EA: 0x86CD004
// IDA_NAME: _ZN9GameWorld16DailyEventModifyEv
// DEMANGLED: GameWorld::DailyEventModify(void)
// IDA disassembly confirms that the original target is a literal empty body.
void GameWorld::DailyEventModify() {
    (void)this;
}

// IDA_EA: 0x86CD010
// IDA_NAME: _ZN9GameWorld18ReselectDailyQuestEv
// DEMANGLED: GameWorld::ReselectDailyQuest(void)
bool GameWorld::ReselectDailyQuest() {
    return G_CDataManager()->reselectDailyTrainingQuest() == true;
}
