#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/tower/death_tower_timer_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <cstdint>
#include <cstring>

namespace WongWork {

// IDA_EA: 0x82A74B0
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgrC2Ev
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::CDungeonMgr(void)
CDeathTower::CDungeonMgr::CDungeonMgr() {
}

// IDA_EA: 0x82A74B6
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgrD2Ev
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::~CDungeonMgr()
CDeathTower::CDungeonMgr::~CDungeonMgr() {
}

// IDA_EA: 0x82A7498
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr5resetEv
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::reset(void)
CDeathTower::CDungeonMgr* CDeathTower::CDungeonMgr::reset() {
    dungeon_ = 0;
    end_stage_ = 0;
    return this;
}

// IDA_EA: 0x82A74CA
// IDA_NAME: _ZN8WongWork11CDeathTower6CStageC2Ev
// DEMANGLED: WongWork::CDeathTower::CStage::CStage(void)
CDeathTower::CStage::CStage()
    : map_info_() {
    resetCurrentStage();
}

// IDA_EA: 0x82A74EC
// IDA_NAME: _ZN8WongWork11CDeathTower6CStageD2Ev
// DEMANGLED: WongWork::CDeathTower::CStage::~CStage()
CDeathTower::CStage::~CStage() {
}

// IDA_EA: 0x82A74BC
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage17resetCurrentStageEv
// DEMANGLED: WongWork::CDeathTower::CStage::resetCurrentStage(void)
CDeathTower::CStage* CDeathTower::CStage::resetCurrentStage() {
    current_stage_ = 1;
    return this;
}

// IDA_EA: 0x84614EC
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage5resetEv
// DEMANGLED: WongWork::CDeathTower::CStage::reset(void)
CDeathTower::CStage* CDeathTower::CStage::reset() {
    map_info_.Clear();
    map_info_.next_monster_unique_id_ = get_rand_int(60000);
    map_info_.next_drop_item_unique_id_ = get_rand_int(60000);
    state_ = static_cast<ENUM_STAGE_STATE>(0);
    return this;
}

// IDA_EA: 0x82A7502
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData14stRewardItem_tC2Ev
// DEMANGLED: WongWork::CDeathTower::CPlayData::stRewardItem_t::stRewardItem_t(void)
CDeathTower::CPlayData::stRewardItem_t::stRewardItem_t() {
}

// IDA_EA: 0x8461CB4
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData14stRewardItem_t5resetEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::stRewardItem_t::reset(void)
bool CDeathTower::CPlayData::stRewardItem_t::reset() {
    item_count_ = 0;
    for (int index = 0; index < 10; ++index) {
        items_[index].reset();
        items_[index].item_id_ = -1;
    }
    return false;
}

// IDA_EA: 0x82A7538
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayDataC2Ev
// DEMANGLED: WongWork::CDeathTower::CPlayData::CPlayData(void)
CDeathTower::CPlayData::CPlayData() {
}

// IDA_EA: 0x82A7572
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayDataD2Ev
// DEMANGLED: WongWork::CDeathTower::CPlayData::~CPlayData()
CDeathTower::CPlayData::~CPlayData() {
}

// IDA_EA: 0x8461F18
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData5resetEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::reset(void)
bool CDeathTower::CPlayData::reset() {
    play_time_ = 0;
    last_play_time_ = 0;
    last_routed_turn_ = 0;
    for (int index = 0; index < 4; ++index) {
        member_alive_[index] = 0;
        member_ready_[index] = 0;
        reward_items_[index].reset();
        reward_exp_[index] = 0;
        enter_start_map_[index] = 0;
    }
    return false;
}

// IDA_EA: 0x823462A
// IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t4initEv
// DEMANGLED: WongWork::CDeathTower::stMapMonsterKillChecker_t::init(void)
CDeathTower::stMapMonsterKillChecker_t*
CDeathTower::stMapMonsterKillChecker_t::init() {
    use_skill_material_tick_ = 0;
    last_mob_die_tick_ = 0;
    uncontinually_mob_die_count_ = 0;
    return this;
}

// IDA_EA: 0x823464C
// IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t19setUseSkillMaterialEj
// DEMANGLED: WongWork::CDeathTower::stMapMonsterKillChecker_t::setUseSkillMaterial(unsigned int)
CDeathTower::stMapMonsterKillChecker_t*
CDeathTower::stMapMonsterKillChecker_t::setUseSkillMaterial(uint32_t tick) {
    use_skill_material_tick_ = tick;
    return this;
}

// IDA_EA: 0x8469C86
// IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t15initMonsterInfoEv
// DEMANGLED: WongWork::CDeathTower::stMapMonsterKillChecker_t::initMonsterInfo(void)
CDeathTower::stMapMonsterKillChecker_t*
CDeathTower::stMapMonsterKillChecker_t::initMonsterInfo() {
    uncontinually_mob_die_count_ = 0;
    return this;
}

// IDA_EA: 0x8469C96
// IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t10dieMonsterEjj
// DEMANGLED: WongWork::CDeathTower::stMapMonsterKillChecker_t::dieMonster(unsigned int,unsigned int)
int CDeathTower::stMapMonsterKillChecker_t::dieMonster(uint32_t tick,
                                                       uint32_t interval) {
    if (tick - last_mob_die_tick_ > interval) {
        uncontinually_mob_die_count_ = 0;
        last_mob_die_tick_ = tick;
    } else {
        ++uncontinually_mob_die_count_;
    }
    return uncontinually_mob_die_count_;
}

// IDA_EA: 0x8469CDA
// IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t23getUseSkillMaterialTimeEv
// DEMANGLED: WongWork::CDeathTower::stMapMonsterKillChecker_t::getUseSkillMaterialTime(void)
int CDeathTower::stMapMonsterKillChecker_t::getUseSkillMaterialTime() {
    return use_skill_material_tick_;
}

// IDA_EA: 0x82A75DA / 0x82A7664
CDeathTower::CDeathTower()
    : dungeon_manager_(), stage_(), play_data_() {
    std::memset(timer_keys_, 0, sizeof(timer_keys_));
}

CDeathTower::~CDeathTower() {
}

// IDA_EA: 0x82A7578 / 0x82A7586 / 0x82A7592
CDeathTower* CDeathTower::setIdx(int index) {
    index_ = index;
    return this;
}

int CDeathTower::getIdx() {
    return index_;
}

CDeathTower* CDeathTower::reset() {
    dungeon_manager_.reset();
    stage_.reset();
    play_data_.reset();
    party_ = 0;
    tower_state_ = static_cast<ENUM_TOWER_STATE>(0);
    return this;
}

// IDA_EA: 0x830EDD2
// IDA_NAME: _ZN8WongWork11CDeathTower11genTimerKeyE13TIMER_MESSAGE
// DEMANGLED: WongWork::CDeathTower::genTimerKey(TIMER_MESSAGE)
int CDeathTower::genTimerKey(TIMER_MESSAGE message) {
    return _genTimerKey(message);
}

// IDA_EA: 0x830EDEC
// IDA_NAME: _ZN8WongWork11CDeathTower12_genTimerKeyE13TIMER_MESSAGE
// DEMANGLED: WongWork::CDeathTower::_genTimerKey(TIMER_MESSAGE)
int CDeathTower::_genTimerKey(TIMER_MESSAGE message) {
    return ++timer_keys_[static_cast<int>(message) - 72];
}

}  // namespace WongWork

// IDA_EA: 0x863CB0A
// IDA_NAME: _ZN30TimerDeathTowerTopRankerReloadC2Ev
// DEMANGLED: TimerDeathTowerTopRankerReload::TimerDeathTowerTopRankerReload(void)
TimerDeathTowerTopRankerReload::TimerDeathTowerTopRankerReload()
    : TimerSig() {}

// IDA_EA: 0x863F5A6 / 0x863F5D6
// IDA_NAME: _ZN30TimerDeathTowerTopRankerReloadD2Ev /
//           _ZN30TimerDeathTowerTopRankerReloadD0Ev
// DEMANGLED: TimerDeathTowerTopRankerReload::~TimerDeathTowerTopRankerReload()
TimerDeathTowerTopRankerReload::~TimerDeathTowerTopRankerReload() {}

// IDA_EA: 0x8637816
// IDA_NAME: _ZN30TimerDeathTowerTopRankerReload12dispatch_sigEiij
// DEMANGLED: TimerDeathTowerTopRankerReload::dispatch_sig(int,int,unsigned int)
int TimerDeathTowerTopRankerReload::dispatch_sig(int,
                                                 int,
                                                 unsigned int) {
    DB_DeathTowerLoadTopRanker::makeRequest(false);
    DB_DeathTowerLoadTopRanker::makeRequest(true);
    registNextTimer(static_cast<time_t>(
                        GlobalData::s_systemTime_.getCurSec()) +
                    86400);
    return 1;
}

// IDA_EA: 0x8637854
// IDA_NAME: _ZN30TimerDeathTowerTopRankerReload15registNextTimerEl
// DEMANGLED: TimerDeathTowerTopRankerReload::registNextTimer(long)
int TimerDeathTowerTopRankerReload::registNextTimer(time_t timer) {
    if (timer) {
        std::tm local_time;
        localtime_r(&timer, &local_time);
        local_time.tm_hour = 4;
        local_time.tm_min = static_cast<int>(get_rand_int(300)) + 30;
        local_time.tm_sec = 0;
        timer = mktime(&local_time);
    } else {
        timer = static_cast<time_t>(GlobalData::s_systemTime_.getCurSec()) +
                static_cast<time_t>(get_rand_int(300));
    }

    const int delay_seconds = static_cast<int>(timer) -
        GlobalData::s_systemTime_.getCurSec();
    const int clamped_delay = std::max(1, delay_seconds);
    return G_TimerQueue()->InsertTimer(6, 0, 115, clamped_delay, 0, 0);
}

namespace {

const uint32_t kDeathTowerPoolDummy = 36641u;
typedef StaticPool<WongWork::CDeathTower, 600> DeathTowerPool;

#if defined(__i386__)
typedef char DeathTower_Size_Must_Be_0xB64[
    (sizeof(WongWork::CDeathTower) == 0xB64) ? 1 : -1];
typedef char DeathTowerPool_Node_Size_Must_Be_0xB6C[
    (sizeof(DeathTowerPool::CNode) == 0xB6C) ? 1 : -1];
typedef char DeathTowerPool_Size_Must_Be_0x30[
    (sizeof(DeathTowerPool) == 0x30) ? 1 : -1];
typedef char DeathTowerPool_Nodes_Offset_Must_Be_0x00[
    (offsetof(DeathTowerPool, nodes) == 0x00) ? 1 : -1];
typedef char DeathTowerPool_FreeNodes_Offset_Must_Be_0x04[
    (offsetof(DeathTowerPool, free_nodes) == 0x04) ? 1 : -1];
typedef char DeathTowerPool_ErrorState_Offset_Must_Be_0x2c[
    (offsetof(DeathTowerPool, error_state) == 0x2c) ? 1 : -1];
#endif

}  // namespace

// The generic bodies are emitted only for the recovered CDeathTower,600
// specialization below.  The mailbox pool owns its own independent provider.
template <typename Type, int Count>
StaticPool<Type, Count>::CNode::CNode()
    : Type(),
      dummy(kDeathTowerPoolDummy),
      reference_count(0) {
}

template <typename Type, int Count>
StaticPool<Type, Count>::CNode::~CNode() {
}

// IDA_EA: 0x82AEF58
// IDA_NAME: _ZN10StaticPoolIN8WongWork11CDeathTowerELi600EEC2Ev
// DEMANGLED: StaticPool<WongWork::CDeathTower,600>::StaticPool(void)
template <typename Type, int Count>
StaticPool<Type, Count>::StaticPool()
    : free_nodes() {
    nodes = new CNode[Count];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }

    for (int index = 0; index < Count; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

template <typename Type, int Count>
StaticPool<Type, Count>::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B1F78
// IDA_NAME: _ZN10StaticPoolIN8WongWork11CDeathTowerELi600EE4FreeEPS1_
// DEMANGLED: StaticPool<WongWork::CDeathTower,600>::Free(WongWork::CDeathTower *)
template <typename Type, int Count>
Type* StaticPool<Type, Count>::Free(Type* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* const pool_node = static_cast<CNode*>(node);
    free_nodes.push(pool_node);
    if (pool_node->dummy != kDeathTowerPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              pool_node->dummy,
              pool_node);
        error_state = 5;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              pool_node->reference_count,
              pool_node);
        error_state = 6;
        PrintBackTrace();
    }
    --pool_node->reference_count;
    return node;
}

template <typename Type, int Count>
int StaticPool<Type, Count>::Remain() {
    return static_cast<int>(free_nodes.size());
}

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kDeathTowerPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<Type*>(node);
}

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Get(int index) {
    if (index < 0 || index >= Count) {
        return 0;
    }
    return nodes + index;
}

template <typename Type, int Count>
int StaticPool<Type, Count>::GetIndex(Type* node) {
    if (reinterpret_cast<std::uintptr_t>(node) <
        reinterpret_cast<std::uintptr_t>(nodes)) {
        return -1;
    }

    const std::size_t byte_offset =
        reinterpret_cast<std::uintptr_t>(node) -
        reinterpret_cast<std::uintptr_t>(nodes);
    const int index = byte_offset == 0
        ? 0
        : static_cast<int>(byte_offset / sizeof(CNode));

    CNode* const pool_node = static_cast<CNode*>(node);
    if (pool_node->dummy != kDeathTowerPoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              pool_node->dummy,
              pool_node);
        error_state = 1;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              pool_node->reference_count,
              pool_node);
        error_state = 2;
    }
    return index <= Count - 1 ? index : -1;
}

// IDA_EA: 0x80EE460
// IDA_NAME: _ZN10StaticPoolIN8WongWork11CDeathTowerELi600EE4SizeEv
template <>
int StaticPool<WongWork::CDeathTower, 600>::Size() {
    return 600;
}

// IDA_EA: 0x80EE46A
// IDA_NAME: _ZN10StaticPoolIN8WongWork11CDeathTowerELi600EE6RemainEv
template <>
int StaticPool<WongWork::CDeathTower, 600>::Remain() {
    return static_cast<int>(free_nodes.size());
}

template class StaticPool<WongWork::CDeathTower, 600>;
