#include "infra/concurrent/timer/timer_dispatcher_abi.h"

#include <algorithm>

#include "old_game/pvp/battlefield/timer_assault_lifecycle.h"
#include "old_game/player/session/timer_return_user_timeout.h"
#include "infra/concurrent/timer/timer_connect_monitor_server.h"
#include "infra/concurrent/timer/timer_daily_schedule.h"
#include "infra/concurrent/timer/timer_death_tower.h"
#include "infra/security/policy/timer_hades_heartbeat.h"
#include "infra/concurrent/timer/timer_pcroom_punish_runtime.h"
#include "old_data/channel/channel_timers.h"
#include "old_game/activity/reward/inter_ontime_reward_runtime.h"
#include "old_game/activity/schedule/timer_event_management.h"
#include "old_game/activity/schedule/timer_event_christmas.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/dungeon/instance/timer_dungeon_data_statistic_runtime.h"
#include "old_game/dungeon/instance/timer_dungeon_statistic_entrance_hour_runtime.h"
#include "old_game/dungeon/instance/timer_dungeon_inout_runtime.h"
#include "old_game/dungeon/instance/timer_elevator_dungeon_runtime.h"
#include "old_game/dungeon/instance/zombie_itembox_runtime.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/dungeon/tower/timer_boss_tower_runtime.h"
#include "old_game/dungeon/room/timer_check_paid_pcroom_term_runtime.h"
#include "old_game/economy/mail/timer_check_mail_timeout.h"
#include "old_game/pvp/battlefield/timer_connect_p2p_pvp_timeout_runtime.h"
#include "old_game/pvp/battlefield/timer_blood_round_runtime.h"
#include "old_game/pvp/battlefield/timer_ultimate_next_round_runtime.h"
#include "old_game/pvp/match/timer_fair_pvp_rematch_runtime.h"
#include "old_game/pvp/battlefield/timer_tournament_dungeon_clear_state_select_runtime.h"
#include "old_game/pvp/battlefield/timer_update_pvp_grade_runtime.h"
#include "old_game/pvp/battlefield/relay_battle_timer_runtime.h"
#include "old_game/pvp/arena/timer_war_room_change_host.h"
#include "old_game/pvp/arena/timer_war_room_state_callbacks_runtime.h"
#include "old_game/social/party/cparty_tod_clear_reward_runtime.h"
#include "old_game/social/party/timer_dungeon_clear_runtime.h"
#include "old_game/economy/mall/npc_shop_timer_runtime.h"
#include "old_game/player/core/timer_avatar_expire_runtime.h"
#include "old_game/player/core/timer_creature_runtime.h"
#include "old_game/player/core/timer_insert_user_play_time_runtime.h"
#include "old_game/player/core/timer_premium_service_runtime.h"
#include "old_game/player/session/timer_update_online_preliminary_team.h"
#include "old_game/world/region/world_timer_runtime.h"

// IDA_EA: 0x863120A
// IDA_NAME: _ZN15TimerDispatcherC2Ev
// DEMANGLED: TimerDispatcher::TimerDispatcher(void)
TimerDispatcher::TimerDispatcher() {
    std::fill(dispatchers_, dispatchers_ + 175, static_cast<TimerSig*>(0));

    // IDA 0x863120A constructor sequence (generated lines 226..234): premium
    // expiry, weekday refresh and delayed start occupy exact messages 4..6.
    dispatchers_[4] = new TimerPremiumServiceOver();
    dispatchers_[5] = new TimerPremiumServiceUpdate();
    dispatchers_[6] = new TimerPremiumServiceStart();

    // IDA 0x8631623 constructs the WarRoom host-change timer at +0x11c,
    // the message-71 slot.  Its concrete gameplay callback remains C-owned.
    dispatchers_[71] = new TimerWarRoomChangeHost();

    // IDA TimerDispatcher C2 @0x863120A installs the Christmas server-event
    // expiry handler in message-table slot 83.  Inter_MonitorServerEventStart
    // schedules exactly this timer after accepting event id 1.
    dispatchers_[83] = new TimerEventChristmas();

    // IDA 0x86318B1..0x86318D0 constructs the periodic detail-channel
    // statistic callback and stores it at +0x150 (TIMER_MESSAGE 84).
    dispatchers_[84] = new TimerStatisticsDetailChannelInfo();

    // IDA 0x8631920..0x863193F constructs the game-channel log callback
    // and stores it at +0x160 (TIMER_MESSAGE 88).  It follows the channel
    // statistic allocation in the original constructor sequence.
    dispatchers_[88] = new TimerSaveLogGamechannel();

    // IDA TimerDispatcher::C2 @0x863120A allocates the daily dungeon-data
    // statistic writer at table word 102.  The callback serializes its exact
    // signal-175 LOGDB payload and re-registers itself for the next channel
    // minute window.
    dispatchers_[102] = new TimerDungeonDataStatistic();

    // IDA TimerDispatcher::C2 @0x8631D17..0x8631D2A constructs the daily
    // user-play-time writer at table word 105.  The callback flushes signal
    // 182 to LOGDB, then schedules its next 04:30 local-time run.
    dispatchers_[105] = new TimerInsertUserPlayTime();

    // IDA C2 @0x863120A allocates the CCoinRefillEvent callback at slot 101.
    // CCoinRefillEvent::registNextRefillTimer writes the same message id.
    dispatchers_[101] = new TimerEventCoinRefill();

    // IDA C2 @0x863120A constructs TimerAvatarExpire at table word 81.
    // TimerAvatarExpire::RegistTimer queues this exact message after reading
    // the configured avatar-expiry period.
    dispatchers_[81] = new TimerAvatarExpire();

    // IDA TimerDispatcher C2 @0x863120A constructs TimerCreature and writes
    // it at table word +92.  Its callback validates the queued user/token /
    // character tuple before checking the active creature's death state.
    // Immediately before it, the original constructor installs the mail
    // expiry callback at table word +8 (raw constructor v55 assignment).
    dispatchers_[8] = new TimerCheckMailTimeOut();
    dispatchers_[92] = new TimerCreature();

    // IDA constructor slots 13, 16, 17, 18 and 41 form the normal
    // dungeon-clear chain: reward application, card selection timeout,
    // state progression and CMD049 fallback.
    dispatchers_[11] = new TimerMapLoading();
    dispatchers_[13] = new TimerDungeonClear();
    dispatchers_[14] = new TimerDungeonFail();
    dispatchers_[16] = new TimerCardSelect();
    dispatchers_[17] = new TimerClearDungeonReward();
    dispatchers_[18] = new TimerClearDungeonScoreScoll();
    dispatchers_[19] = new TimerClearDungeonCardSelectRight();
    dispatchers_[41] = new TimerSetPlayResult();

    // IDA TimerDispatcher::C2 @0x8631641..0x86317CD constructs the blood
    // small-round continuation, ten independent phase callbacks and the
    // special-monster callback in their fixed message slots.  The phase
    // objects must stay distinct because CParty validates a key per message.
    dispatchers_[22] = new TimerBloodSmallRoundRegister();
    dispatchers_[23] = new TimerBloodPhaseRegister0();
    dispatchers_[24] = new TimerBloodPhaseRegister1();
    dispatchers_[25] = new TimerBloodPhaseRegister2();
    dispatchers_[26] = new TimerBloodPhaseRegister3();
    dispatchers_[27] = new TimerBloodPhaseRegister4();
    dispatchers_[28] = new TimerBloodPhaseRegister5();
    dispatchers_[29] = new TimerBloodPhaseRegister6();
    dispatchers_[30] = new TimerBloodPhaseRegister7();
    dispatchers_[31] = new TimerBloodPhaseRegister8();
    dispatchers_[32] = new TimerBloodPhaseRegister9();
    dispatchers_[33] = new TimerBloodCrazyMonster();

    // IDA TimerDispatcher::C2 @0x86317DF..0x8631861 constructs four
    // independent TimerBloodDungeonClearReward instances for the blood
    // dungeon's successive reward phases. They must not share timer state:
    // CParty validates each queued message/key pair before advancing the
    // corresponding reward transition.
    dispatchers_[34] = new TimerBloodDungeonClearReward();
    dispatchers_[35] = new TimerBloodDungeonClearReward();
    dispatchers_[36] = new TimerBloodDungeonClearReward();
    dispatchers_[37] = new TimerBloodDungeonClearReward();

    // IDA TimerDispatcher::C2 @0x8631867..0x8631886 installs the 11007
    // next-round difficulty callback at the sole message-38 slot.
    dispatchers_[38] = new TimerUltimateNextRoundStart();

    // IDA TimerDispatcher C2 places this return-to-village completion
    // callback in the party timer slot 20.
    dispatchers_[20] = new TimerEPLPReturnVillage();

    // IDA TimerDispatcher::C2 @0x8632690..0x86326A3 creates the TOD clear
    // reward callback at table word 44.  Its CParty target validates the
    // payload-selected timer key before serializing CMD347 and continuing
    // the original EPLP/timer sequence.
    dispatchers_[44] = new Timer_TOD_Send_ClearReward();

    // IDA TimerDispatcher::C2 @0x8631545..0x863160A constructs the six
    // WarRoom state-transition callbacks and writes them at
    // +0x104 through +0x118, i.e. TIMER_MESSAGE 65 through 70.
    dispatchers_[65] = new TimerWarRoomWaitCount();
    dispatchers_[66] = new TimerWarRoomReadyLockCount();
    dispatchers_[67] = new TimerWarRoomBattlePauseCount();
    dispatchers_[68] = new TimerWarRoomResultCount();
    dispatchers_[69] = new TImerWarRoomTimeBomb();
    dispatchers_[70] = new TimerWarRoomCheckFail();

    // IDA TimerDispatcher::C2 @0x86324E6..0x86324F9 constructs the PvP
    // peer-connection timeout callback at table word 59.  The room has
    // already generated the message-59 key before it queues the 180-second
    // timeout, so this must be a distinct table object rather than a generic
    // PvP timer fallback.
    dispatchers_[59] = new Timer_ConnectP2PPvPTimeout();

    // IDA TimerDispatcher::C2 @0x86325B2 writes Timer_FairPvPRematch to
    // +0xf8, the sole TIMER_MESSAGE 62 callback.  It dispatches only after
    // the room's message-62 key matches, then calls the current IMatch's
    // recovered OnRematchTimeout virtual slot.
    dispatchers_[62] = new Timer_FairPvPRematch();

    // IDA C2 @0x863120A constructs the tournament clear-state notifier in
    // table slot 46.  CParty queues this exact message after completing the
    // tournament reward calculation.
    dispatchers_[46] = new Timer_TournamentDugeonClearStateNotiReward();

    // IDA 0x863278C..0x863279F constructs the select-phase continuation
    // separately and stores it at message 47.  The notification timer above
    // is the only normal producer of this key after it changes the party to
    // state 2; both entries are required for the original reward flow.
    dispatchers_[47] = new Timer_TournametDungeonClearStateSelect();

    // 0828 ZombieMode.etc schedules the party-scoped random item-box
    // generator through the formerly unused timer message 48.
    dispatchers_[ZombieItemBox::kGenerateTimerMessage] =
        new TimerZombieItemBoxGenerate();

    // IDA TimerDispatcher::C2 @0x86327B1..0x863280E constructs three
    // independent Timer_TowerOnTime objects for the BossTower clear,
    // return-to-village, and ready-check timer messages.  These objects are
    // vptr-only, but the original constructor does not share one instance
    // between the three table entries.
    dispatchers_[163] = new Timer_TowerOnTime();
    dispatchers_[164] = new Timer_TowerOnTime();
    dispatchers_[165] = new Timer_TowerOnTime();

    // IDA TimerDispatcher::C2 @0x863120A, constructor sequence
    // 0x8632848..0x86328F5: one global altar tick at message 166 and four
    // independent stage callbacks at messages 167 through 170.
    dispatchers_[166] = new advancealtar::Timer_MainTick();
    dispatchers_[167] = new advancealtar::Timer_StageControl();
    dispatchers_[168] = new advancealtar::Timer_StageControl();
    dispatchers_[169] = new advancealtar::Timer_StageControl();
    dispatchers_[170] = new advancealtar::Timer_StageControl();

    // IDA TimerDispatcher::C2 @0x8631B11 stores TimerStartRelayBattle at
    // byte offset +0x18c, i.e. TIMER_MESSAGE 99.  CRelayBattleMgr::OnStart
    // and its force-timer death transition queue only this completed relay
    // OnFight callback with the manager's timer key.
    dispatchers_[99] = new TimerStartRelayBattle();

    // IDA TimerDispatcher::C2 @0x863120A constructs TimerBossDieTimeout at
    // table word 100.  Its payload is the party timer key plus the member
    // character number consumed by CParty::OnTimeoutBossDie.
    dispatchers_[100] = new TimerBossDieTimeout();

    // IDA TimerDispatcher::C2 @0x8631DE3..0x8631DFD stores the daily PvP
    // grade flush callback at table word 108.  GlobalData::Init queues its
    // first daily 02:00 run only on channel 1.
    dispatchers_[108] = new TimerUpdatePvPGrade();

    // IDA TimerDispatcher::C2 @0x863120A constructs the paid-PC-room
    // expiry callback at table word +7.  Premium login schedules this exact
    // message, so omitting the table entry silently loses the revalidation
    // and PC-room server logout/login sequence.
    dispatchers_[7] = new TimerCheckPaidPCRoomTerm();

    // Village-monster battle timeout owns the party timer slot 39.  The
    // callback checks both the party timer key and the 16-bit monster token.
    dispatchers_[39] = new TimerVillageAttackedPlayTimeout();

    // IDA TimerDispatcher::C2 @0x8632185..0x8632198 constructs the Elevator
    // Dungeon continuation at table word 40. CParty queues this only after
    // it consumes the dungeon-53 one-shot flag during map loading.
    dispatchers_[40] = new TimerElevatorDungeon();

    // These are the original C2 assignments at +0x1b4..+0x1c8 for messages
    // 109 through 114.  No fallback handler is installed.
    dispatchers_[109] = new TimerDetectDisconnect();
    dispatchers_[110] = new TimerCompleteLoadAssaultTimeout();
    dispatchers_[111] = new TimerConnectP2PAssaultTimeout();
    dispatchers_[112] = new TimerEndAssault();
    dispatchers_[113] = new TimerCloseAssault();
    dispatchers_[114] = new TimerConnectP2PAfterAssaultTimeout();

    // IDA TimerDispatcher::C2 @0x863120A constructs six independent
    // TimerDeathTower callbacks.  They are vptr-only objects, but each timer
    // message has its own allocation in the original table; sharing one
    // instance changes the original construction/destruction graph.
    dispatchers_[73] = new TimerDeathTower();
    dispatchers_[75] = new TimerDeathTower();
    dispatchers_[76] = new TimerDeathTower();
    dispatchers_[77] = new TimerDeathTower();
    dispatchers_[78] = new TimerDeathTower();
    dispatchers_[74] = new TimerDeathTower();

    // IDA TimerDispatcher C2 @0x863120A: the monitor-server reconnect
    // callback is stored at table word +122 (TIMER_MESSAGE 122).  The
    // disconnect Inter slot schedules this exact message after the original
    // thirty-second delay, so leaving the table entry empty drops the reply
    // chain even when the timer body itself is present.
    dispatchers_[122] = new TimerConnectMonitorServer();

    // IDA C2 @0x863CD5A..0x863CE02 installs the village-attacked event
    // callbacks in fixed message slots 133..135 and 138.
    // The two anti-cheat punishment callbacks are allocated separately at
    // 0x8632258 and 0x863227D before the surrounding user timer family.
    dispatchers_[129] = new TimerSetPunishUser();
    dispatchers_[133] = new TimerVillageMonsterUpdateRate();
    dispatchers_[134] = new TimerVillageMonsterRegen();
    dispatchers_[135] = new TimerStartVillageAttacked();
    dispatchers_[138] = new TimerStartExpEventAfterVillageAttack();

    // IDA TimerDispatcher C2 @0x863120A stores the periodic online-school
    // team persistence callback at table word 143.  The receiver queues this
    // exact timer after a new team is loaded from DB.
    dispatchers_[143] = new Timer_UpdateOnlinePreliminaryTeam();

    // IDA TimerDispatcher C2 @0x8632160..0x8632173 allocates the vptr-only
    // NPC mood callback and stores it at message 116.  Its dispatch body is
    // intentionally a successful no-op in the original binary.
    dispatchers_[116] = new TimerNPCMoodChange();

    // IDA TimerDispatcher C2 @0x86324A8..0x86324BB constructs the periodic
    // SecretShop statistic sender in message slot 144.
    dispatchers_[144] = new Timer_SecretShopStatistic();

    // IDA TimerDispatcher::C2 @0x863120A (generated TimerDispatcher.cpp
    // lines 568-570) constructs Timer_DailySchedule and stores it at table
    // word 145. Startup and the callback both queue this exact message number.
    dispatchers_[145] = new Timer_DailySchedule();

    // IDA TimerDispatcher::C2 @0x863120A stores the paired dungeon in/out
    // callbacks in table words 147 and 148.  Startup schedules only the
    // opening message; each original callback owns the next transition.
    // The PC-room login retry handler immediately precedes them at table word
    // 146 (C2 call at 0x8632543).
    dispatchers_[146] = new Timer_PcRoomAuthRetry();
    dispatchers_[147] = new Timer_DungeonInoutOpenTime();
    dispatchers_[148] = new Timer_DungeonInoutCloseTime();

    // IDA C2 @ 0x86325D7: the login return-user timeout is installed at
    // table offset +0x254, i.e. TIMER_MESSAGE 149.
    dispatchers_[149] = new Timer_ReturnUserTimeout();

    // IDA TimerDispatcher C2 @0x863120A stores the Hades reconnect/heartbeat
    // callback in message slot 150.  Inter_HadesDisConnect schedules exactly
    // this timer after a TCP disconnect.
    dispatchers_[150] = new Timer_HadesHeartBeat();

    // IDA C2 @0x863227D allocates the trade-punishment callback and stores it
    // at table word 151. It is distinct from the hack punishment callback at
    // word 129 because their queued effects differ.
    dispatchers_[151] = new TimerSetTradePunishUser();

    // IDA TimerDispatcher C2 @0x863263F..0x8632652 installs the daily shop
    // list refresh handler in message slot 153.
    dispatchers_[153] = new Timer_OneADayItemShop();

    // TimerDispatcher::C2 @0x863120A creates this vptr-only object and
    // stores it in table word 155 (the recovered source sequence is at
    // generated lines 604–606).  Inter_OnTimeEventRewardEnd schedules this
    // exact callback when its queued reward-user list is non-empty.
    dispatchers_[155] = new Timer_Send_Ontime_Reward();

    // IDA TimerDispatcher C2 @0x863120A constructs Timer_EventManagement
    // and stores it at table word 156.  Its callback refreshes event script
    // time values and schedules the next one-minute update.
    dispatchers_[156] = new Timer_EventManagement();

    // Original TimerDispatcher@0x863120A leaves word 159 vacant between the
    // QuickParty timer (158) and AssaultGhost (160).  Golden Commerce uses
    // that otherwise-unowned slot for its one-shot cargo-selection deadline
    // and token-bound server-authoritative start fallback.
    dispatchers_[golden_commerce::kSelectionDeadlineTimerMessage] =
        new golden_commerce::TimerSelectionDeadline();

    // IDA TimerDispatcher::C2 @0x863120A constructs the hourly dungeon
    // entrance statistic writer at table word 171. Startup schedules this
    // exact slot once; its callback thereafter reschedules every hour.
    dispatchers_[171] = new Timer_DungeonStatisticEntrance_Hour();

    // IDA 0x8632917..0x8632936 constructs the channel server-state refresh
    // callback and stores it at +0x2b0 (TIMER_MESSAGE 172).
    dispatchers_[172] = new Timer_RefreshServerState();

    // IDA C2 @0x863120A stores the CEventStayTime fan-out callback in table
    // slot 173; CEventStayTime::RegistNextTimer queues message 173.
    dispatchers_[173] = new Timer_StayTimeEvent();
}

// IDA_EA: 0x863298E
// IDA_NAME: _ZN15TimerDispatcherD2Ev
// DEMANGLED: TimerDispatcher::~TimerDispatcher()
TimerDispatcher::~TimerDispatcher() {
    for (int message = 0; message <= 174; ++message) {
        delete dispatchers_[message];
        dispatchers_[message] = 0;
    }
}

// IDA_EA: 0x86329F6
// IDA_NAME: _ZN15TimerDispatcher14get_dispatcherEi
// DEMANGLED: TimerDispatcher::get_dispatcher(int)
TimerSig* TimerDispatcher::get_dispatcher(int message) {
    return dispatchers_[message];
}

// IDA_EA: 0x8632A18
// IDA_NAME: _ZN15TimerDispatcher8dispatchER10TimerEntry
// DEMANGLED: TimerDispatcher::dispatch(TimerEntry &)
int TimerDispatcher::dispatch(TimerEntry& entry) {
    TimerSig* const dispatcher = get_dispatcher(entry.message);
    if (!dispatcher) {
        return 0;
    }
    return dispatcher->dispatch_sig(entry.object_id, entry.param1,
                                    static_cast<unsigned int>(entry.param2));
}
