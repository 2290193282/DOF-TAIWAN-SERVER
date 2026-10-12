#ifndef OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STAGE_CONTROL_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STAGE_CONTROL_RUNTIME_H_

#include <stdint.h>

#include <map>
#include <vector>

#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"
#include "old_data/PVF/dungeon/advance_altar_dungeon_script.h"
#include "old_game/activity/schedule/character_expand_runtime_data.h"
#include "old_game/world/region/world_packet_structs.h"
#include "infra/concurrent/timer/timer_sig.h"

class CUser;
class CDungeon;
class CMap;
class CAdvanceAltarLog;
struct MSG_MONSTER_DIE;

namespace advancealtar {

struct _RewardItem;

struct StageEndType {
    enum T : int {};
};

// IDA_EA: 0x812D378.  This is an embedded member of ProcStage, not an
// independently allocated byte buffer.  Keeping the containers named here is
// necessary for both the original destruction order and the stage-pool node
// contract.
class ProcStage {
public:
    class ProcSummonObjectMs {
    public:
        ProcSummonObjectMs(CharacData& character_data,
                           const int& current_time_ms);
        ~ProcSummonObjectMs();

        int reset();
        // IDA_EA: 0x812D540 / 0x812D5BA / 0x812D5D0 / 0x812D658 /
        //         0x812D804 / 0x812DAEC
        void insertSummonObject(_SummonObjectMs& summon_object);
        int getLastInsertDataIndex();
        void completePushSummonObject(int last_insert_data_index);
        char summonObjectAndSendPacket();
        void debugLog() const;
        static bool compare(const _SummonObjectMs* left,
                            const _SummonObjectMs* right);
        // IDA_EA: 0x812DB00
        int dieMob(unsigned short monster_id,
                   unsigned short monster_type,
                   const MSG_MONSTER_DIE& message);
        // IDA_EA: 0x812D50A / 0x8134820.
        bool isEndSummon() const;
        int getAliveMonsterCount() const;

        int next_index;                                      // +0x00
        std::vector<_SummonObjectMs*> summon_objects;        // +0x04
        std::map<int16_t, _SummonObjectMs*> summon_by_id;    // +0x10
        // IDA 0x812DB00 increments +0x28 on death; 0x812D658 increments
        // +0x2c when CMD523 is sent.
        int dead_count;                                      // +0x28
        int created_count;                                   // +0x2c
        CharacData* character_data;                          // +0x30
        const int* current_time_ms;                          // +0x34
    };

    explicit ProcStage(CharacData& character_data);
    ~ProcStage();

    int resetAll();
    int reset(int reset_type);
    // IDA_EA: 0x812EAA6
    // IDA_NAME: _ZN12advancealtar9ProcStage5pauseEb
    // DEMANGLED: advancealtar::ProcStage::pause(bool)
    int pause(bool paused);
    // CMD647 must answer with the authoritative server-side pause flag instead
    // of echoing a request which may not match the current stage state.
    bool isPaused() const;
    // IDA_EA: 0x812DE70
    // IDA_NAME: _ZN12advancealtar9ProcStage10startStageEiNS_15StageDifficulty1TE
    // DEMANGLED: advancealtar::ProcStage::startStage(int,advancealtar::StageDifficulty::T)
    int startStage(int stage_index, int difficulty);
    // IDA_EA: 0x812E0D6 / 0x812E146 / 0x812EBA8
    char procTimeLine();
    ProcStage* incCurrentMs();
    int makeCurrentPhaseSummonObject();

    // IDA_EA: 0x812EB04 / 0x812EB26 / 0x812F770 / 0x8134838 /
    //         0x8134844
    // The stage dungeon is the direct source for both type and index.
    StageType::T getStageType() const;
    int getStageIndex() const;
    bool isRewardState() const;
    // IDA_EA: 0x812F70C / 0x812F74C.
    bool isSurvivalModeFirstRound() const;
    bool setNotSurvivalModeFirstRound();
    const TimeLine* getTimeLine() const;
    CMap* getMap() const;
    int getStageDifficulty() const;
    int getCurrentSurvivalRoundNo() const;
    // IDA_EA: 0x812EB4E
    // IDA_NAME: _ZNK12advancealtar9ProcStage23getClearSurvivalRoundNoEv
    // DEMANGLED: advancealtar::ProcStage::getClearSurvivalRoundNo(void)const
    int getClearSurvivalRoundNo() const;
    int getStartStageTime() const;
    int getMapBasisLevel() const;
    // IDA_EA: 0x812E16E
    // IDA_NAME: _ZN12advancealtar9ProcStage20addCreatedMapMonsterERNS_18_MapMonsterSummaryE
    // DEMANGLED: advancealtar::ProcStage::addCreatedMapMonster(advancealtar::_MapMonsterSummary &)
    int addCreatedMapMonster(_MapMonsterSummary& summary);
    // IDA_EA: 0x812E1DE / 0x812E2C6 / 0x812EA00.
    int dieMob(unsigned short monster_id,
               unsigned short monster_type,
               const MSG_MONSTER_DIE& message);
    // IDA_EA: 0x812EB5A.
    bool isEndAllPhase() const;
    int isEndStage(StageEndType::T& end_type) const;
    int endProcAllKillMonster();
    // IDA_EA: 0x812F3AE
    // IDA_NAME: _ZN12advancealtar9ProcStage20addAchievementRewardERSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiS2_EEE
    int addAchievementReward(std::map<int, _Reward>& rewards);
    // IDA_EA: 0x812F0FA
    // IDA_NAME: _ZN12advancealtar9ProcStage16rewardClearStageERSt6vectorINS_11_RewardItemESaIS2_EE
    int rewardClearStage(std::vector<_RewardItem>& rewards);
    // IDA_EA: 0x812E6DE
    // IDA_NAME: _ZNK12advancealtar9ProcStage14sendClearStageERKSt6vectorINS_11_RewardItemESaIS2_EERKS1_INS_8_BuyItemESaIS7_EERKSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiSD_EEEib
    void sendClearStage(const std::vector<_RewardItem>& rewards,
                        const std::vector<_BuyItem>& unlocked_items,
                        const std::map<int, _Reward>& achievement_rewards,
                        int reward_star,
                        bool improved_survival_record) const;
    // IDA_EA: 0x812E3F6
    // IDA_NAME: _ZN12advancealtar9ProcStage19clearStageAndRewardEv
    // DEMANGLED: advancealtar::ProcStage::clearStageAndReward(void)
    CUser* clearStageAndReward();
    struct State {
        enum T : int {};
    };
    State::T getState() const;
    ProcStage* setState(State::T state_value);

    static void sendMailRewardItem(
        CUser* user,
        const std::vector<_RewardItem>& reward_items,
        const char* title_key,
        const char* text_key);

    CharacData* character_data;                             // +0x00
    int state;                                              // +0x04
    int current_time_ms;                                    // +0x08
    // IDA 0x812EB5A indexes TimeLine::phases with +0x0c.  +0x10 is the
    // accumulated start time of the next phase, not a stage index.
    int current_phase_index;                                // +0x0c
    int next_phase_begin_time_ms;                           // +0x10
    int16_t next_created_monster_id;                        // +0x14
    uint8_t pad_16[2];                                      // +0x16
    ProcSummonObjectMs summon_control;                      // +0x18
    std::map<int16_t, _MapMonsterSummary> created_monsters; // +0x50
    int stage_difficulty;                                   // +0x68
    int map_basis_level;                                    // +0x6c
    // IDA 0x812EB04/0x812EB26 load this pointer at +0x70, then read the
    // advance-altar type or CDungeon index.  resetAll clears it.
    CDungeon* dungeon_;                                     // +0x70
    CMap* map_;                                              // +0x74
    uint8_t survival_mode_first_round;                      // +0x78
    uint8_t pad_79[3];                                      // +0x79
    int current_survival_round;                             // +0x7c
    std::vector<int> timeline_actions;                      // +0x80
    int start_stage_time;                                   // +0x8c
    const TimeLine* timeline;                               // +0x90
};

class StageControl;

class HistoryLog {
public:
    // IDA data: _ZN12advancealtar10HistoryLog22advanceAltarstatistics_E.
    // The logger owns the accumulated command-59 DB payloads for every
    // Advance Altar state transition, including CMD573 purchases.
    static CAdvanceAltarLog advanceAltarstatistics_;

    // IDA_EA: 0x8133BD4
    // IDA_NAME: _ZN12advancealtar10HistoryLog16dungeonClearInfoER5CUserRNS_12StageControlEb
    // DEMANGLED: advancealtar::HistoryLog::dungeonClearInfo(CUser &,advancealtar::StageControl &,bool)
    static int dungeonClearInfo(CUser& user,
                                StageControl& stage_control,
                                bool count_entrance);
    // IDA_EA: 0x81339B0
    // IDA_NAME: _ZN12advancealtar10HistoryLog12enterDungeonER5CUserRKNS_9ProcStageE
    // DEMANGLED: advancealtar::HistoryLog::enterDungeon(CUser &,advancealtar::ProcStage const&)
    static int enterDungeon(CUser& user, const ProcStage& stage);
    // IDA_EA: 0x8133ABC
    // IDA_NAME: _ZN12advancealtar10HistoryLog12leaveDungeonER5CUserRKNS_9ProcStageE
    // DEMANGLED: advancealtar::HistoryLog::leaveDungeon(CUser &,advancealtar::ProcStage const&)
    static int leaveDungeon(CUser& user, const ProcStage& stage);
    // IDA_EA: 0x8133B40
    // IDA_NAME: _ZN12advancealtar10HistoryLog11killMonsterER5CUseriRK15MSG_MONSTER_DIE
    static int killMonster(CUser& user,
                           int monster_id,
                           const MSG_MONSTER_DIE& message);
    // IDA_EA: 0x8133D56
    // IDA_NAME: _ZN12advancealtar10HistoryLog7starAddER5CUserNS_11AddStarType1TEii
    // DEMANGLED: advancealtar::HistoryLog::starAdd(CUser &,advancealtar::AddStarType::T,int,int)
    static int starAdd(CUser& user,
                       AddStarType::T type,
                       int stage_index,
                       int star);
    // IDA_EA: 0x8133DB0
    // IDA_NAME: _ZN12advancealtar10HistoryLog7starSubER5CUserNS_11SubStarType1TEi
    static int starSub(CUser& user, SubStarType::T type, int star);
    // IDA_EA: 0x8133DDA
    // IDA_NAME: _ZN12advancealtar10HistoryLog7buyItemENS_20AdvanceAltarShopType1TEissi
    static int buyItem(AdvanceAltarShopType::T shop_type,
                       int item_id,
                       int16_t first_level,
                       int16_t last_level,
                       int star);
    // IDA_EA: 0x8133E30
    // IDA_NAME: _ZN12advancealtar10HistoryLog16updateStatisticsEv
    static int updateStatistics();
};

class StageControl {
public:
    StageControl();
    ~StageControl();

    int reset();
    // IDA_EA: 0x812FA7A / 0x81300CE / 0x81300F6 / 0x81301E0
    // IDA_NAME: _ZN12advancealtar12StageControl11selectStageEP5CUseriNS_15StageDifficulty1TE /
    //           _ZN12advancealtar12StageControl7destroyEv /
    //           _ZN12advancealtar12StageControl13sendStageInfoEv /
    //           _ZN12advancealtar12StageControl11sendMapInfoEv
    // DEMANGLED: advancealtar::StageControl::selectStage(CUser *,int,advancealtar::StageDifficulty::T) /
    //            advancealtar::StageControl::destroy(void) /
    //            advancealtar::StageControl::sendStageInfo(void) /
    //            advancealtar::StageControl::sendMapInfo(void)
    int selectStage(CUser* user, int stage_index, int difficulty);
    // IDA_EA: 0x812FC4E
    // IDA_NAME: _ZN12advancealtar12StageControl9leaveUserEv
    // DEMANGLED: advancealtar::StageControl::leaveUser(void)
    int leaveUser();
    int destroy();
    void sendStageInfo();
    void sendMapInfo();
    // IDA_EA: 0x812FB98
    // IDA_NAME: _ZN12advancealtar12StageControl16onTimerStageTickEv
    // DEMANGLED: advancealtar::StageControl::onTimerStageTick(void)
    int onTimerStageTick();
    // IDA_EA: 0x812FBFA
    // IDA_NAME: _ZN12advancealtar12StageControl7onTimerE13TIMER_MESSAGENS_12StageEndType1TE
    // DEMANGLED: advancealtar::StageControl::onTimer(TIMER_MESSAGE,advancealtar::StageEndType::T)
    int onTimer(int timer_message, StageEndType::T end_type);
    // IDA_EA: 0x81305F6 / 0x8130644 / 0x8130692 / 0x81306BE
    int onClearReward(StageEndType::T end_type);
    int onEplp(StageEndType::T end_type);
    int onEplpReturnVillage();
    void sendEplp(StageEndType::T end_type);
    int getCenterCount() const;
    // IDA_EA: 0x812FDD4
    // IDA_NAME: _ZN12advancealtar12StageControl9gotoStageEP5CUser
    // DEMANGLED: advancealtar::StageControl::gotoStage(CUser *)
    static void gotoStage(CUser* user);
    // IDA_EA: 0x81348AE
    // IDA_NAME: _ZN12advancealtar12StageControl12getProcStageEv
    // DEMANGLED: advancealtar::StageControl::getProcStage(void)
    ProcStage* getProcStage();
    int getIndex() const;
    StageControl* setIndex(int index);

    int stage_control_index;  // +0x00
    ProcStage proc_stage;     // +0x04
    CharacData charac_data;   // +0x98
};

// IDA unit: 99_misc_global/advancealtar/Timer_MainTick.  TimerDispatcher owns
// one vptr-only instance in message slot 166; it advances every active altar
// stage and re-enqueues the next one-second tick.
class Timer_MainTick : public TimerSig {
public:
    // IDA_EA: 0x863D112 / 0x813DBE4 / 0x813DC14
    Timer_MainTick();
    virtual ~Timer_MainTick();

    // IDA_EA: 0x812F798
    // IDA_NAME: _ZN12advancealtar14Timer_MainTick12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x812F7B8
    // IDA_NAME: _ZN12advancealtar14Timer_MainTick19RegistNextTimerGageEv
    static int RegistNextTimerGage();
};

// IDA unit: 99_misc_global/advancealtar/Timer_StageControl. TimerDispatcher
// owns four independent vptr-only objects in message slots 167 through 170.
class Timer_StageControl : public TimerSig {
public:
    // IDA_EA: 0x863D12E / 0x813DB96 / 0x813DBC6
    Timer_StageControl();
    virtual ~Timer_StageControl();

    // IDA_EA: 0x812F806
    // IDA_NAME: _ZN12advancealtar18Timer_StageControl12dispatch_sigEiij
    virtual int dispatch_sig(int stage_index,
                             int timer_message,
                             unsigned int end_type);

    // IDA_EA: 0x812F850
    // IDA_NAME: _ZN12advancealtar18Timer_StageControl21RegistTimerCheckFinshEi
    static int RegistTimerCheckFinsh(int stage_index);

    // IDA_EA: 0x812F89C
    // IDA_NAME: _ZN12advancealtar18Timer_StageControl22RegistTimerClearRewardEiNS_12StageEndType1TE
    static int RegistTimerClearReward(int stage_index,
                                      StageEndType::T end_type);

    // IDA_EA: 0x812F8EC
    // IDA_NAME: _ZN12advancealtar18Timer_StageControl15RegistTimerEPLPEiNS_12StageEndType1TE
    static int RegistTimerEPLP(int stage_index, StageEndType::T end_type);

    // IDA_EA: 0x812F956
    // IDA_NAME: _ZN12advancealtar18Timer_StageControl28RegistTimerEPLPReturnVillageEi
    static int RegistTimerEPLPReturnVillage(int stage_index);
};

static_assert(sizeof(ProcStage::ProcSummonObjectMs) == 0x38u,
              "advancealtar::ProcStage::ProcSummonObjectMs must match IDA");
static_assert(sizeof(ProcStage) == 0x94u,
              "advancealtar::ProcStage must match IDA");
static_assert(sizeof(StageControl) == 0xc0u,
              "advancealtar::StageControl must match IDA");
#if defined(__i386__)
static_assert(sizeof(Timer_MainTick) == 0x04u,
              "advancealtar::Timer_MainTick must remain TimerSig-only");
static_assert(sizeof(Timer_StageControl) == 0x04u,
              "advancealtar::Timer_StageControl must remain TimerSig-only");
#endif

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STAGE_CONTROL_RUNTIME_H_
