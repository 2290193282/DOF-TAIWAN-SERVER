#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kBlueMarblePoolDummy = 36641u;

typedef StaticPool<BlueMarble, 300> BlueMarblePool;

#if defined(__i386__)
static_assert(sizeof(BlueMarblePool::CNode) == 0x48u,
              "BlueMarble pool node must retain the observed ABI");
#endif

}  // namespace

// IDA_EA: 0x82B971E
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EE5CNodeC2Ev
template <>
BlueMarblePool::CNode::CNode()
    : BlueMarble(),
      dummy(kBlueMarblePoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AF8C6
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EE5CNodeD2Ev
template <>
BlueMarblePool::CNode::~CNode() {}

// IDA_EA: 0x82AF8DA
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EEC2Ev
template <>
BlueMarblePool::StaticPool()
    : free_nodes() {
    nodes = new CNode[300];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }
    for (int index = 0; index <= 299; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

// IDA_EA: 0x82AFA8C
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EED2Ev
template <>
BlueMarblePool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B4542
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EE7AcquireEv
template <>
BlueMarble* BlueMarblePool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }
    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kBlueMarblePoolDummy) {
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
    return static_cast<BlueMarble*>(node);
}

// IDA_EA: 0x82B465E
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EE8GetIndexEPS0_
template <>
int BlueMarblePool::GetIndex(BlueMarble* blue_marble) {
    CNode* const node = static_cast<CNode*>(blue_marble);
    if (node < nodes) {
        return -1;
    }
    const int index = static_cast<int>(node - nodes);
    if (node->dummy != kBlueMarblePoolDummy) {
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
    return index <= 299 ? index : -1;
}

// IDA_EA: 0x82B4874
// IDA_NAME: _ZN10StaticPoolI10BlueMarbleLi300EE4FreeEPS0_
// DEMANGLED: StaticPool<BlueMarble,300>::Free(BlueMarble*)
template <>
BlueMarble* BlueMarblePool::Free(BlueMarble* blue_marble) {
    CNode* const node = static_cast<CNode*>(blue_marble);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    free_nodes.push(node);
    if (node->dummy != kBlueMarblePoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 5;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return blue_marble;
}
