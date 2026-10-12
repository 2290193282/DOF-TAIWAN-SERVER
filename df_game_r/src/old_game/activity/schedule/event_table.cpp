#include "old_game/activity/schedule/event_table.h"
#include "old_game/activity/schedule/event_runtime_db_sync.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/activity/condition/character_restrict.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/reduce_upgrade_item_pay.h"
#include "old_game/activity/schedule/seria_room_ani_deco_event.h"
#include "old_game/activity/schedule/growth_creature_event.h"
#include "old_game/activity/schedule/bingo_runtime.h"
#include "old_game/activity/reward/cerashop_reward_point_event.h"
#include "old_game/activity/schedule/new_account_level_up_event_to_job_abi.h"
#include "old_game/activity/schedule/one_a_day_item_shop_event.h"
#include "old_game/activity/schedule/online_preliminary_event.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/pcroom_play_time_event.h"
#include "old_game/activity/schedule/premium_gold_card_event.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/world_value_types.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/economy/mall/delivery_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/growth_equip_event.h"
#include "old_game/player/core/cuser.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"


// IDA owner boundary: 03_channel_select/CLeadingChannelEvent.
// IDA_EA: ctor/dtor/start/end @0x811FD64/@0x811FD8A/@0x811FDBA/
// @0x811FDD8/@0x811FE34/@0x811FE92.
class CLeadingChannelEvent : public CEventBase {
public:
    CLeadingChannelEvent();
    virtual ~CLeadingChannelEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

private:
    int exp_rate_;
};

typedef char CLeadingChannelEvent_Size_Must_Be_0x10[
    (sizeof(CLeadingChannelEvent) == 0x10) ? 1 : -1];

// IDA_EA: 0x811715C
// IDA_NAME: _ZN21CReduceUpgradeItemPayC2Ev
// DEMANGLED: CReduceUpgradeItemPay::CReduceUpgradeItemPay(void)
CReduceUpgradeItemPay::CReduceUpgradeItemPay() : CEventBase() {}

// IDA_EA: 0x8117178
// IDA_NAME: _ZN21CReduceUpgradeItemPayD2Ev
// DEMANGLED: CReduceUpgradeItemPay::~CReduceUpgradeItemPay()
CReduceUpgradeItemPay::~CReduceUpgradeItemPay() {}

// IDA_EA: 0x81171C6
// IDA_NAME: _ZN21CReduceUpgradeItemPay10StartEventEv
// DEMANGLED: CReduceUpgradeItemPay::StartEvent(void)
void CReduceUpgradeItemPay::StartEvent() {
    SetEventFlag(true);
}

// IDA_EA: 0x81171E2
// IDA_NAME: _ZN21CReduceUpgradeItemPay10StartEventE10Word_Param
// DEMANGLED: CReduceUpgradeItemPay::StartEvent(Word_Param)
void CReduceUpgradeItemPay::StartEvent(Word_Param param) {
    // The original stores the four payload bytes directly over stADDINFO.
    // Keep the same two named signed-halfword values without a raw overlay.
    add_info_.word_00 = static_cast<int16_t>(param.value & 0xffffu);
    add_info_.word_02 =
        static_cast<int16_t>((param.value >> 16) & 0xffffu);
    StartEvent();

    cMyTrace trace(
        "virtual void CReduceUpgradeItemPay::StartEvent(Word_Param)",
        28,
        0);
    trace("CReduceUpgradeItemPay(AddInfo:%d,%d) Event Start!",
          add_info_.word_00,
          add_info_.word_02);
}

// IDA_EA: 0x8117270
// IDA_NAME: _ZN21CReduceUpgradeItemPay8EndEventEv
// DEMANGLED: CReduceUpgradeItemPay::EndEvent(void)
void CReduceUpgradeItemPay::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CReduceUpgradeItemPay::EndEvent()", 34, 0);
    trace("CPowerWarRewardRandomBuffEvent Event End!");
}

// IDA_EA: 0x854B56E
// IDA_NAME: _ZN21CReduceUpgradeItemPay13GetReduceRateEv
// DEMANGLED: CReduceUpgradeItemPay::GetReduceRate(void)
int CReduceUpgradeItemPay::GetReduceRate() {
    return add_info_.word_00;
}

namespace {

// IDA data: event table at 0x936FA60, row stride 5 dwords:
// event_id / apply_type / event-base / event-name / PVF string key.
// The manager owns and deletes each non-null event-base pointer, precisely as
// CEventManager::~CEventManager @0x8114D4E does in the original binary.
struct EventTableEntry {
    int event_id;
    int apply_type;
    CEventBase* event_base;
    const char* event_name;
    const char* explain_key;
};

EventTableEntry kEventTable[] = {
    {1, 0, 0, "CUnlimitFatigueEvent", "event_msg_01"},
    {2, 1, 0, "CMaxFatigueFactorEvent", "event_msg_02"},
    {3, 1, 0, "CExpDoubleEvent", "event_msg_03"},
    {4, 4, 0, "CCoinEventPerDay", "event_msg_04"},
    {5, 1, 0, "CCoinEventOnCharCreate", "event_msg_05"},
    {6, 0, 0, "CLeadingChannelEvent", "event_msg_06"},
    {7, 1, 0, "CItemDropRatioEvent", "event_msg_07"},
    {8, 1, 0, "CPCRoomBurningEvent", "event_msg_08"},
    {9, 3, 0, "CSchoolMatchEvent", "event_msg_09"},
    {10, 0, 0, "CPCRoomFatigueEvent", "event_msg_10"},
    {12, 0, 0, "CReformingDanjinEvent", "event_msg_12"},
    {13, 2, 0, "CCoinRefillEvent", "event_msg_13"},
    {15, 0, 0, "CBurningFatigueEvent", "event_msg_15"},
    {16, 0, 0, "CClearRewardCardEvent", "event_msg_16"},
    {17, 1, 0, "CCeraShopBonusItemEvent", "event_msg_17"},
    {18, 0, 0, "CTournamentPvPEvent", "event_msg_18"},
    {19, 0, 0, "CGoldCardBlankItemEvent", "event_msg_19"},
    {21, 0, 0, "CCollectArchieveEventLog", "event_msg_21"},
    {22, 0, 0, "CPCRoomWorldDropEvent", "event_msg_22"},
    {24, 1, 0, "CPartyExpBonusEvent", "event_msg_24"},
    {29, 0, 0, "CPcRoomCardBlankItemEvent", "event_msg_29"},
    {30, 0, 0, "CPowerWarEvent", "event_msg_30"},
    {32, 0, 0, "CStabToDeathEvent", "event_msg_32"},
    {33, 0, 0, "CGuildWarEvent", "event_msg_00"},
    {34, 0, 0, "CAutoMarketConditionsControlEvent", "event_msg_00"},
    {35, 0, 0, "CVendingMachineBonusEvent", "event_msg_35"},
    {36, 0, 0, "CBurnigGoldMonsterEvent", "event_msg_36"},
    {38, 0, 0, "CNoNeedGoldOnGuildCreateEvent", "event_msg_33"},
    {39, 1, 0, "CDeathTowerWinPointEvent", "event_msg_34"},
    {40, 0, 0, "CNotApplyBalkeunEvent", "event_msg_37"},
    {41, 1, 0, "CCharacterDayEvent", "event_msg_38"},
    {42, 0, 0, "CAssaultOnOffEvent", "event_msg_39"},
    {43, 0, 0, "CFatigueBuffEvent", "event_msg_40"},
    {45, 1, 0, "CBloodDungeonRewardFirstEvent", "event_msg_42"},
    {46, 1, 0, "CBloodDungeonRewardSecondEvent", "event_msg_43"},
    {48, 0, 0, "CPCRoomWorldDropEvent2nd", "event_msg_45"},
    {49, 1, 0, "CRestrictCharacCreationEvent", "event_msg_46"},
    {50, 1, 0, "CReduceUpgradeItemPay", "event_msg_47"},
    {51, 2, 0, "COnTimeEvent", "event_msg_48"},
    {52, 0, 0, "CBreakAwayPreventEvent", "event_msg_49"},
    {53, 0, 0, "CPowerWarVictoriousEvent", "event_msg_50"},
    {54, 0, 0, "CPvPExpPenaltyEvent", "event_msg_51"},
    {55, 0, 0, "CPvPLiveEvent", "event_msg_52"},
    {56, 0, 0, "CIntegratedPvPServerMatchEvent", "event_msg_53"},
    {58, 0, 0, "CSecretShopEvent", "event_msg_54"},
    {57, 0, 0, "COnlinePreliminaryEvent", "event_msg_55"},
    {61, 0, 0, "CDnFLeaguePromoteFirstEvent", "event_msg_57"},
    {62, 0, 0, "CDnFLeaguePromoteSecondEvent", "event_msg_58"},
    {64, 0, 0, "CDoubleGoldCardEvent", "event_msg_60"},
    {65, 0, 0, "CPremiumGoldCard", "event_msg_61"},
    {67, 0, 0, "CGmRegistEvent", "event_msg_63"},
    {86, 0, 0, "CFatigueAttendance", "event_msg_86"},
    {87, 0, 0, "CWeekendBonusEvent", "event_msg_87"},
    {91, 0, 0, "CUXGameLogEvent", "event_msg_91"},
    {92, 0, 0, "CPCRoomPlayTimeEvent", "event_msg_92"},
    {93, 0, 0, "LevelUpBefore70LvEvent", "event_msg_93"},
    {94, 0, 0, "CDimensionActivationEvent", "event_msg_94"},
    {95, 0, 0, "BlueMarbleDungeonEvent", "event_msg_95"},
    {96, 1, 0, "AttendanceEvent", "event_msg_96"},
    {100, 0, 0, "GrowthEquipEvent", "event_msg_100"},
    {101, 0, 0, "CFatigueGiveItemEvent", "event_msg_101"},
    {102, 0, 0, "CStopOverlabExpEvent", "event_msg_102"},
    {103, 0, 0, "GiveGrowCreatureEvent", "event_msg_103"},
    {104, 0, 0, "NewAccountLevelUpEventToJob", "event_msg_104"},
    {105, 0, 0, "SeriaRoomAniDecoEvent", "event_msg_105"},
    {106, 0, 0, "BingoEvent", "event_msg_106"},
    {109, 0, 0, "OneADayItemShopEvent", "event_msg_109"},
    {113, 0, 0, "CConditionEvent", "event_msg_67"},
    {116, 2, 0, "CAradRyosikaEvent", "event_msg_190"},
    {118, 0, 0, "FreelyGiveItemEvent0828", "event_msg_118"},
    {119, 0, 0, "CEventAdvanceAltarOpen", "event_msg_103"},
    {129, 0, 0, "CKingOfPickEvent0828", "event_msg_129"},
    {155, 4, 0, "Arad_MomijiEvent", "event_msg_196"},
    {158, 0, 0, "LevelupSupportEvent", "event_msg_158"},
    {159, 2, 0, "CEventStayTime", "event_msg_159"},
    {160, 4, 0, "CEventCreateDnf", "event_msg_160"},
    {161, 2, 0, "CEventCeraShopRewardPoint", "event_msg_161"},
    {162, 2, 0, "EventNewCharacterReward", "event_msg_162"},
    {164, 0, 0, "HeroMissionEvent", "event_msg_163"},
    {163, 0, 0, "LevelupSupport2ndEvent", "event_msg_164"},
    {165, 0, 0, "EventGiveMeBox", "event_msg_165"},
    {260, 0, 0, "BlueHorseEvent", "event_msg_260"},
};

const int kEventTableCount =
    static_cast<int>(sizeof(kEventTable) / sizeof(kEventTable[0]));

typedef char CEventTable_Count_Must_Be_0x52[
    (kEventTableCount == 0x52) ? 1 : -1];

EventTableEntry* FindEventTableEntryById(int event_id) {
    for (int index = 0; index < kEventTableCount; ++index) {
        if (kEventTable[index].event_id == event_id) {
            return &kEventTable[index];
        }
    }
    return 0;
}

void SetEventBase(int event_id, CEventBase* event_base) {
    EventTableEntry* const entry = FindEventTableEntryById(event_id);
    if (entry) {
        entry->event_base = event_base;
    }
}

// The original `__static_initialization_and_destruction_0` creates all 79
// 0627 typed objects before Init.  The 0828 additions are represented in the
// metadata table as well; Event 260 is transferred to an ABI-safe sidecar
// instead of indexing beyond repeat_events_[166].  The HeroMissionEvent row
// follows the
// original Singleton<Get> construction path; event 100 remains the real
// heap-owned GrowthEquipEvent, not a login-path facade.
struct EventTableStaticObjects {
    EventTableStaticObjects() {
        SetEventBase(1, new CUnlimitFatigueEvent());
        SetEventBase(2, new CMaxFatigueFactorEvent());
        SetEventBase(3, new CExpDoubleEvent());
        SetEventBase(4, new CCoinEventPerDay());
        SetEventBase(5, new CCoinEventOnCharCreate());
        SetEventBase(6, new CLeadingChannelEvent());
        SetEventBase(7, new CItemDropRatioEvent());
        SetEventBase(8, new CPCRoomBurningEvent());
        SetEventBase(9, new CSchoolMatchEvent());
        SetEventBase(10, new CPCRoomFatigueEvent());
        SetEventBase(12, new CReformingDanjinEvent());
        SetEventBase(13, new CCoinRefillEvent());
        SetEventBase(15, new CBurningFatigueEvent());
        SetEventBase(16, new CClearRewardCardEvent());
        SetEventBase(17, new CCeraShopBonusItemEvent());
        SetEventBase(18, new CTournamentPvPEvent());
        SetEventBase(19, new CGoldCardBlankItemEvent());
        SetEventBase(21, new CCollectArchieveEventLog());
        SetEventBase(22, new CPCRoomWorldDropEvent());
        SetEventBase(24, new CPartyExpBonusEvent());
        SetEventBase(29, new CPcRoomCardBlankItemEvent());
        SetEventBase(30, new CPowerWarEvent());
        SetEventBase(32, new CStabToDeathEvent());
        SetEventBase(33, new CGuildWarEvent());
        SetEventBase(34, new CAutoMarketConditionsControlEvent());
        SetEventBase(35, new CVendingMachineBonusEvent());
        SetEventBase(36, new CBurnigGoldMonsterEvent());
        SetEventBase(38, new CNoNeedGoldOnGuildCreateEvent());
        SetEventBase(39, new CDeathTowerWinPointEvent());
        SetEventBase(40, new CNotApplyBalkeunEvent());
        SetEventBase(41, new CCharacterDayEvent());
        SetEventBase(42, new CAssaultOnOffEvent());
        SetEventBase(43, new CFatigueBuffEvent());
        SetEventBase(45, new CBloodDungeonRewardFirstEvent());
        SetEventBase(46, new CBloodDungeonRewardSecondEvent());
        SetEventBase(48, new CPCRoomWorldDropEvent2nd());
        SetEventBase(49, new CRestrictCharacCreationEvent());
        SetEventBase(50, new CReduceUpgradeItemPay());
        SetEventBase(51, new COnTimeEvent());
        SetEventBase(52, new CBreakAwayPreventEvent());
        SetEventBase(53, new CPowerWarVictoriousEvent());
        SetEventBase(54, new CPvPExpPenaltyEvent());
        SetEventBase(55, new CPvPLiveEvent());
        SetEventBase(56, new CIntegratedPvPServerMatchEvent());
        SetEventBase(57, new COnlinePreliminaryEvent());
        SetEventBase(58, new CSecretShopEvent());
        SetEventBase(61, new CDnFLeaguePromoteFirstEvent());
        SetEventBase(62, new CDnFLeaguePromoteSecondEvent());
        SetEventBase(64, new CDoubleGoldCardEvent());
        SetEventBase(65, new CPremiumGoldCard());
        SetEventBase(67, new CGmRegistEvent());
        SetEventBase(86, new CFatigueAttendance());
        SetEventBase(87, new CWeekendBonusEvent());
        SetEventBase(91, new CUXGameLogEvent());
        SetEventBase(92, new CPCRoomPlayTimeEvent());
        SetEventBase(93, new LevelUpBefore70LvEvent());
        SetEventBase(94, new CDimensionActivationEvent());
        SetEventBase(95, new BlueMarbleDungeonEvent());
        SetEventBase(96, new AttendanceEvent());
        SetEventBase(100, new GrowthEquipEvent());
        SetEventBase(101, new CFatigueGiveItemEvent());
        SetEventBase(102, new CStopOverlabExpEvent());
        SetEventBase(103, new GiveGrowCreatureEvent());
        SetEventBase(104, new NewAccountLevelUpEventToJob());
        SetEventBase(105, new SeriaRoomAniDecoEvent());
        SetEventBase(106, new BingoEvent());
        SetEventBase(109, new OneADayItemShopEvent());
        SetEventBase(113, new CConditionEvent());
        SetEventBase(116, new CAradRyosikaEvent());
        SetEventBase(118, new FreelyGiveItemEvent());
        SetEventBase(119, new CEventAdvanceAltarOpen());
        SetEventBase(129, new CKingOfPickEvent());
        SetEventBase(155, new Arad_MomijiEvent());
        SetEventBase(158, new LevelupSupportEvent());
        SetEventBase(159, new CEventStayTime());
        SetEventBase(160, new CEventCreateDnf());
        SetEventBase(161, new CEventCeraShopRewardPoint());
        SetEventBase(162, new EventNewCharacterReward());
        SetEventBase(163, new LevelupSupport2ndEvent());
        SetEventBase(164, ARAD::Singleton<HeroMissionEvent>::Get());
        SetEventBase(165, new EventGiveMeBox());
        SetEventBase(260, new BlueHorseEvent());
    }
};

EventTableStaticObjects g_event_table_static_objects;

}  // namespace

// IDA_EA: 0x811A0C8
// IDA_NAME: _ZN11CEventTableC2Ev
// DEMANGLED: CEventTable::CEventTable(void)
CEventTable::CEventTable() {}

// IDA_EA: 0x811A0CE
// IDA_NAME: _ZN11CEventTableD2Ev
// DEMANGLED: CEventTable::~CEventTable()
CEventTable::~CEventTable() {}

// IDA_EA: 0x811A0D4
// IDA_NAME: _ZN11CEventTable18GetEventTableCountEv
// DEMANGLED: CEventTable::GetEventTableCount(void)
int CEventTable::GetEventTableCount() {
    return kEventTableCount;
}

// IDA_EA: 0x811A0DE
// IDA_NAME: _ZN11CEventTable12GetEventBaseEi
// DEMANGLED: CEventTable::GetEventBase(int)
CEventBase* CEventTable::GetEventBase(int index) {
    if (index < 0 || CEventTable::GetEventTableCount() <= index) {
        return 0;
    }

    return kEventTable[index].event_base;
}

// IDA_EA: 0x811A11E
// IDA_NAME: _ZN11CEventTable10GetEventIdEi
// DEMANGLED: CEventTable::GetEventId(int)
int CEventTable::GetEventId(int index) {
    if (index < 0 || CEventTable::GetEventTableCount() <= index) {
        return 0;
    }

    return kEventTable[index].event_id;
}

// IDA_EA: 0x811A15D
// IDA_NAME: _Z15_FindEventTableiPKcRb
// DEMANGLED: _FindEventTable(int,char const *,bool &)
int _FindEventTable(int event_id,
                    const char* event_name,
                    bool& name_mismatch) {
    for (int index = 0; index < CEventTable::GetEventTableCount(); ++index) {
        if (kEventTable[index].event_id != event_id) {
            continue;
        }

        // The original intentionally compares only the DB string's length:
        // the deployed schema truncates the id-34 class name to varchar(30).
        if (std::strncmp(kEventTable[index].event_name,
                         event_name,
                         std::strlen(event_name)) != 0) {
            name_mismatch = true;
        }
        return index;
    }
    return -1;
}

// IDA_EA: 0x811A1E0
// IDA_NAME: _ZN11CEventTable16VerifyEventTableERSt6vectorIiSaIiEE
// DEMANGLED: CEventTable::VerifyEventTable(std::vector<int> &)
bool CEventTable::VerifyEventTable(std::vector<int>& missing_indexes) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(1),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }

    missing_indexes.clear();
    std::map<int, int> existing_indexes;
    db_handle->set_query("seLect event_id, event_name from dnf_event_info");
    if (!db_handle->exec(true)) {
        return false;
    }

    const int rows = db_handle->get_n_rows();
    for (int row = 0; row < rows; ++row) {
        if (!db_handle->fetch()) {
            break;
        }

        int event_id = 0;
        char event_name[256] = {};
        if (!db_handle->get_int(0, event_id) ||
            !db_handle->get_str(1, event_name, sizeof(event_name))) {
            return false;
        }

        bool name_mismatch = false;
        const int table_index =
            _FindEventTable(event_id, event_name, name_mismatch);
        if (table_index < 0) {
            continue;
        }
        if (name_mismatch) {
            return false;
        }
        existing_indexes.insert(std::make_pair(table_index, 1));
    }

    for (int index = 0; index < CEventTable::GetEventTableCount(); ++index) {
        if (existing_indexes.find(index) == existing_indexes.end()) {
            missing_indexes.push_back(index);
        }
    }
    return true;
}


// IDA_EA: 0x811A4D4
// IDA_NAME: _ZN11CEventTable16InsertEventTableERSt6vectorIiSaIiEE
// DEMANGLED: CEventTable::InsertEventTable(std::vector<int>&)
bool CEventTable::InsertEventTable(std::vector<int>& missing_indexes) {
    if (missing_indexes.empty()) {
        return true;
    }

    std::string values;
    char tuple[256] = {};
    for (std::vector<int>::const_iterator it = missing_indexes.begin();
         it != missing_indexes.end();
         ++it) {
        const int index = *it;
        if (index < 0 || CEventTable::GetEventTableCount() <= index) {
            return false;
        }

        const EventTableEntry& entry = kEventTable[index];
        const char* const explain =
            g_scriptStringManager_.findString(4, entry.explain_key, 0);
        std::sprintf(tuple,
                     "(%d,%d,'%s','%s')",
                     entry.event_id,
                     entry.apply_type,
                     entry.event_name,
                     explain ? explain : "");
        values += tuple;
        if (it + 1 != missing_indexes.end()) {
            values += ',';
        }
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(1),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "inSert into dnf_event_info(event_id, apply_type, event_name, event_explain) values%s",
        values.c_str());
    return db_handle->exec(true);
}

// IDA_EA: 0x8115982
// IDA_NAME: _ZN13CEventManager8AddEventEiP10CEventBase
// DEMANGLED: CEventManager::AddEvent(int,CEventBase *)
int CEventManager::AddEvent(int event_id, CEventBase* event) {
    if (event_id == 260) {
        return blue_horse_event::InstallBlueHorseEventSidecar(event) ? 1 : 0;
    }
    if (event_id < 0 || event_id > 165) {
        return 0;
    }
    repeat_events_[event_id] = event;
    return 1;
}

// IDA_EA: 0x8114DCC
// IDA_NAME: _ZN13CEventManager16InitEventManagerEv
// DEMANGLED: CEventManager::InitEventManager(void)
bool CEventManager::InitEventManager() {
    repeat_events_[1] = new CUnlimitFatigueEvent();
    repeat_events_[2] = new CMaxFatigueFactorEvent();
    repeat_events_[3] = new CExpDoubleEvent();
    repeat_events_[4] = new CCoinEventPerDay();
    repeat_events_[5] = new CCoinEventOnCharCreate();
    repeat_events_[6] = new CLeadingChannelEvent();
    repeat_events_[7] = new CItemDropRatioEvent();
    repeat_events_[8] = new CPCRoomBurningEvent();
    repeat_events_[33] = new CGuildWarEvent();
    repeat_events_[10] = new CPCRoomFatigueEvent();
    repeat_events_[12] = new CReformingDanjinEvent();
    repeat_events_[13] = new CCoinRefillEvent();
    repeat_events_[15] = new CBurningFatigueEvent();
    repeat_events_[16] = new CClearRewardCardEvent();
    repeat_events_[17] = new CCeraShopBonusItemEvent();
    repeat_events_[18] = new CTournamentPvPEvent();
    repeat_events_[19] = new CGoldCardBlankItemEvent();
    repeat_events_[29] = new CPcRoomCardBlankItemEvent();
    repeat_events_[30] = new CPowerWarEvent();
    repeat_events_[21] = new CCollectArchieveEventLog();
    repeat_events_[22] = new CPCRoomWorldDropEvent();
    repeat_events_[24] = new CPartyExpBonusEvent();
    repeat_events_[32] = new CStabToDeathEvent();
    repeat_events_[9] = new CSchoolMatchEvent();
    repeat_events_[34] = new CAutoMarketConditionsControlEvent();
    repeat_events_[35] = new CVendingMachineBonusEvent();
    repeat_events_[36] = new CBurnigGoldMonsterEvent();
    repeat_events_[38] = new CNoNeedGoldOnGuildCreateEvent();
    repeat_events_[39] = new CDeathTowerWinPointEvent();
    repeat_events_[40] = new CNotApplyBalkeunEvent();
    repeat_events_[41] = new CCharacterDayEvent();
    repeat_events_[49] = new CRestrictCharacCreationEvent();
    repeat_events_[50] = new CReduceUpgradeItemPay();
    repeat_events_[51] = new COnTimeEvent();
    repeat_events_[52] = new CBreakAwayPreventEvent();
    repeat_events_[53] = new CPowerWarVictoriousEvent();
    repeat_events_[58] = new CSecretShopEvent();
    repeat_events_[86] = new CFatigueAttendance();
    repeat_events_[87] = new CWeekendBonusEvent();
    repeat_events_[113] = new CConditionEvent();
    repeat_events_[91] = new CUXGameLogEvent();
    repeat_events_[95] = new BlueMarbleDungeonEvent();
    repeat_events_[96] = new AttendanceEvent();
    repeat_events_[102] = new CStopOverlabExpEvent();
    // IDA static initialization @0x811B640 constructs the base-only
    // SeriaRoomAniDecoEvent and stores it in repeat-event slot 105. The
    // event participates in normal CEventManager destruction/load lifecycle;
    // it is not a packet-dispatcher-local substitute.
    repeat_events_[105] = new SeriaRoomAniDecoEvent();
    repeat_events_[119] = new CEventAdvanceAltarOpen();
    repeat_events_[129] = new CKingOfPickEvent();
    repeat_events_[116] = new CAradRyosikaEvent();
    repeat_events_[118] = new FreelyGiveItemEvent();
    repeat_events_[155] = new Arad_MomijiEvent();
    repeat_events_[158] = new LevelupSupportEvent();
    repeat_events_[159] = new CEventStayTime();
    repeat_events_[160] = new CEventCreateDnf();
    repeat_events_[165] = new EventGiveMeBox();
    if (!blue_horse_event::InstallBlueHorseEventSidecar(
            new BlueHorseEvent())) {
        return false;
    }
    return LoadEventFromDB() == true;
}

// IDA_EA: 0x8115998
// IDA_NAME: _ZN13CEventManager14GetRepeatEventEi
// DEMANGLED: CEventManager::GetRepeatEvent(int)
CEventBase* CEventManager::GetRepeatEvent(int event_id) {
    if (event_id == 260) {
        return blue_horse_event::GetBlueHorseEventSidecar();
    }
    if (event_id < 0 || event_id > 165) {
        return 0;
    }
    return repeat_events_[event_id];
}

// IDA_EA: 0x81159B6
// IDA_NAME: _ZN13CEventManager15LoadEventFromDBEv
// DEMANGLED: CEventManager::LoadEventFromDB(void)
bool CEventManager::LoadEventFromDB() {
    // dnf_event_log is read once during process startup.  Operators update the
    // persistent rows first and re-run df_game_r; there is no online polling
    // or hot activation path here.
    return event_runtime_db_sync::SyncInitial();
}

// IDA_EA: 0x8115CC6
// IDA_NAME: _ZN13CEventManager17TriggerEventStartEi10Word_Param
// DEMANGLED: CEventManager::TriggerEventStart(int,Word_Param)
bool CEventManager::TriggerEventStart(int event_id, Word_Param param) {
    if (::filterEvent(event_id)) {
        return true;
    }

    CEventBase* event = GetRepeatEvent(event_id);
    if (!event) {
        return false;
    }

    event->StartEvent(param);
    BroadcastEventInfo();
    event->deliveryStart(delivery_);
    // Original CEventManager::TriggerEventStart dispatches the vtable +0x14
    // StartAction hook here.  Calling the no-argument StartEvent a second time
    // discards parameterized event state (for example EXP/drop/fatigue factors)
    // and can register event timers twice.
    event->StartAction();
    return true;
}

// IDA_EA: 0x8115D60
// IDA_NAME: _ZN13CEventManager15TriggerEventEndEi
// DEMANGLED: CEventManager::TriggerEventEnd(int)
bool CEventManager::TriggerEventEnd(int event_id) {
    if (::filterEvent(event_id)) {
        return true;
    }

    CEventBase* event = GetRepeatEvent(event_id);
    if (!event) {
        return false;
    }

    event->deliveryEnd(delivery_);
    BroadcastEventInfo();
    event->EndEvent();
    return true;
}

// IDA_EA: 0x8115DE2
// IDA_NAME: _ZN13CEventManager17MakeNotiEventInfoER11PacketGuardP5CUser
// DEMANGLED: CEventManager::MakeNotiEventInfo(PacketGuard &,CUser *)
bool CEventManager::MakeNotiEventInfo(PacketGuard& guard, CUser* user) {
    std::vector<stEVENTADDINFO> add_infos;

    guard.clear();
    guard.put_header(0, 108);

    int event_count = 0;
    int event_count_index = guard.get_index();
    guard.put_short(event_count);

    if (GlobalData::s_event_manager) {
        for (int event_id = 0; event_id <= 165; ++event_id) {
            CEventBase* event = GetRepeatEvent(event_id);
            if (event && event->IsEventing(user)) {
                guard.put_short(event_id);
                ++event_count;

                stADDINFO add_info = event->GetAddInfo();
                if (add_info.word_00 >= 0) {
                    stEVENTADDINFO event_add_info;
                    event_add_info.event_index = event_id;
                    event_add_info.begin_value = add_info.word_00;
                    event_add_info.end_value = add_info.word_02;
                    add_infos.push_back(event_add_info);
                }
            }
        }
        CEventBase* const blue_horse = GetRepeatEvent(260);
        if (blue_horse && blue_horse->IsEventing(user)) {
            guard.put_short(260);
            ++event_count;

            stADDINFO add_info = blue_horse->GetAddInfo();
            if (add_info.word_00 >= 0) {
                stEVENTADDINFO event_add_info;
                event_add_info.event_index = 260;
                event_add_info.begin_value = add_info.word_00;
                event_add_info.end_value = add_info.word_02;
                add_infos.push_back(event_add_info);
            }
        }
    }

    guard.put_short(event_count_index, event_count);

    if (GlobalData::s_event_manager) {
        for (int event_id = 0; event_id <= 165; ++event_id) {
            CEventBase* event = GetRepeatEvent(event_id);
            if (event && event->IsEventing(user)) {
                event->AppendInfo(guard);
            }
        }
        CEventBase* const blue_horse = GetRepeatEvent(260);
        if (blue_horse && blue_horse->IsEventing(user)) {
            blue_horse->AppendInfo(guard);
        }
    }

    if (add_infos.empty()) {
        guard.put_byte(0);
    } else {
        guard.put_byte(static_cast<int>(add_infos.size()));
        for (std::vector<stEVENTADDINFO>::const_iterator it =
                 add_infos.begin();
             it != add_infos.end(); ++it) {
            guard.put_short(it->event_index);
            guard.put_short(it->begin_value);
            guard.put_short(it->end_value);
        }
    }

    guard.finalize(true);
    return false;
}

// IDA_EA: 0x8116108
// IDA_NAME: _ZN13CEventManager18BroadcastEventInfoEv
// DEMANGLED: CEventManager::BroadcastEventInfo(void)
void CEventManager::BroadcastEventInfo() {
    PacketGuard guard;
    GameWorldUserMap& users = G_GameWorld()->users_in_world_;

    for (GameWorldUserMap::iterator it = users.begin();
         it != users.end(); ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2) {
            MakeNotiEventInfo(guard, user);
            user->Send(guard);
        }
    }
}

// IDA_EA: 0x8116212
// IDA_NAME: _ZN13CEventManager30IsExistOverlappedExpBonusEventEv
// DEMANGLED: CEventManager::IsExistOverlappedExpBonusEvent(void)
bool CEventManager::IsExistOverlappedExpBonusEvent() {
    CEventBase* event = GlobalData::s_event_manager->GetRepeatEvent(102);
    const char is_eventing = static_cast<char>(event->IsEventing(0));
    const char is_affected_effect = static_cast<char>(
        ::GlobalData::s_globalEffectManager_->isAffectedEffect(2u));
    return is_eventing || is_affected_effect;
}

// IDA_EA: 0x8116276
// IDA_NAME: _ZN13CEventManager16changeCharacNameEP5CUseriPci
// DEMANGLED: CEventManager::changeCharacName(CUser *,int,char *,int)
bool CEventManager::changeCharacName(CUser* user,
                                     int charac_no,
                                     char* name,
                                     int length) {
    bool result = false;
    for (int event_id = 0; ; ++event_id) {
        result = event_id <= 165;
        if (event_id > 165) {
            break;
        }

        CEventBase* event = repeat_events_[event_id];
        if (event && event->IsEventing(0)) {
            event->changeCharacName(user, charac_no, name, length);
        }
    }
    CEventBase* const blue_horse = GetRepeatEvent(260);
    if (blue_horse && blue_horse->IsEventing(0)) {
        blue_horse->changeCharacName(user, charac_no, name, length);
    }
    return result;
}
// IDA_EA: 0x811630E
// IDA_NAME: _ZN13CEventManager14dailyresetDataEv
// DEMANGLED: CEventManager::dailyresetData(void)
bool CEventManager::dailyresetData() {
    bool result = false;
    for (int event_id = 0; ; ++event_id) {
        result = event_id <= 165;
        if (event_id > 165) {
            break;
        }

        CEventBase* event = repeat_events_[event_id];
        if (event && event->IsEventing(0)) {
            event->dailyresetData();
        }
    }
    CEventBase* const blue_horse = GetRepeatEvent(260);
    if (blue_horse && blue_horse->IsEventing(0)) {
        blue_horse->dailyresetData();
    }
    return result;
}
