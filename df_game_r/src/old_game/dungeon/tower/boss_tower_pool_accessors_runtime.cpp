#include <cstring>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"

namespace {

const uint32_t kBossTowerPoolDummy = 36641u;
typedef StaticPool<WongWork::CBossTower, 600> BossTowerPool;

#if defined(__i386__)
static_assert(sizeof(BossTowerPool::CNode) == 0xb20u,
              "StaticPool<CBossTower,600>::CNode must retain the IDA ABI");
#endif

}  // namespace

// IDA_EA: 0x82B93D2
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EE5CNodeC2Ev
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::CNode::CNode(void)
template <>
StaticPool<WongWork::CBossTower, 600>::CNode::CNode()
    : WongWork::CBossTower(),
      dummy(kBossTowerPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AF1A2
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EE5CNodeD2Ev
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::CNode::~CNode()
template <>
StaticPool<WongWork::CBossTower, 600>::CNode::~CNode() {}

// IDA_EA: 0x82AF1BE
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EEC2Ev
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::StaticPool(void)
template <>
StaticPool<WongWork::CBossTower, 600>::StaticPool()
    : nodes(0), free_nodes(), error_state(0) {
    nodes = new CNode[600];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }
    for (int index = 0; index != 600; ++index) {
        free_nodes.push(nodes + index);
    }
}

// IDA_EA: 0x82AF36C
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EED2Ev
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::~StaticPool()
template <>
StaticPool<WongWork::CBossTower, 600>::~StaticPool() {
    if (nodes) delete[] nodes;
}

// IDA_EA: 0x82B2420
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EE4FreeEPS1_
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::Free(WongWork::CBossTower *)
template <>
WongWork::CBossTower*
StaticPool<WongWork::CBossTower, 600>::Free(WongWork::CBossTower* tower) {
    if (!tower) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* const node = static_cast<CNode*>(tower);
    free_nodes.push(node);
    if (node->dummy != kBossTowerPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 5;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return tower;
}

// IDA_EA: 0x82B20D0
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EE7AcquireEv
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::Acquire(void)
template <>
WongWork::CBossTower*
StaticPool<WongWork::CBossTower, 600>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kBossTowerPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<WongWork::CBossTower*>(node);
}

// IDA_EA: 0x82B21FE
// IDA_NAME: _ZN10StaticPoolIN8WongWork10CBossTowerELi600EE8GetIndexEPS1_
// DEMANGLED: StaticPool<WongWork::CBossTower,600>::GetIndex(WongWork::CBossTower *)
template <>
int StaticPool<WongWork::CBossTower, 600>::GetIndex(
    WongWork::CBossTower* tower) {
    CNode* const node = static_cast<CNode*>(tower);
    if (node < nodes) {
        return -1;
    }

    const int index = static_cast<int>(node - nodes);
    if (node->dummy != kBossTowerPoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 1;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 2;
    }
    return index <= 599 ? index : -1;
}

namespace WongWork {

// IDA_EA: 0x814BEA2
// IDA_NAME: _ZN8WongWork10CBossStage5resetEv
// DEMANGLED: WongWork::CBossStage::reset(void)
CBossStage* CBossStage::reset() {
    stage_start_tick_ = 0;
    stage_end_tick_ = 0;
    map_info_.Clear();
    std::memset(tail_9c8_, 0, sizeof(tail_9c8_));
    return this;
}

// IDA_EA: 0x82AA716
// IDA_NAME: _ZN8WongWork10CBossTower6setIdxEi
// DEMANGLED: WongWork::CBossTower::setIdx(int)
CBossTower* CBossTower::setIdx(int index) {
    pool_index_ = static_cast<uint32_t>(index);
    return this;
}

// IDA_EA: 0x82AA728
// IDA_NAME: _ZN8WongWork10CBossTower6getIdxEv
// DEMANGLED: WongWork::CBossTower::getIdx(void)
int CBossTower::getIdx() {
    return static_cast<int>(pool_index_);
}

}  // namespace WongWork
