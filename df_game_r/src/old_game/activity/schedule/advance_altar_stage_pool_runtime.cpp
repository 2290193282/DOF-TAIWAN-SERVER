#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <cstddef>
#include <cstdint>

namespace {

const uint32_t kStageControlPoolDummy = 36641u;
typedef StaticPool<advancealtar::StageControl, 600> StageControlPool;

#if defined(__i386__)
static_assert(sizeof(StageControlPool::CNode) == 0xc8u,
              "StaticPool<advancealtar::StageControl,600>::CNode IDA size");
static_assert(sizeof(StageControlPool) == 0x30u,
              "StaticPool<advancealtar::StageControl,600> IDA size");
#endif

}  // namespace

// IDA_EA: 0x82B94F4
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EE5CNodeC2Ev
template <>
StageControlPool::CNode::CNode()
    : advancealtar::StageControl(),
      dummy(kStageControlPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AF408
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EE5CNodeD2Ev
template <>
StageControlPool::CNode::~CNode() {}

// IDA_EA: 0x82AF41C
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EEC2Ev
template <>
StageControlPool::StaticPool()
    : free_nodes() {
    nodes = new CNode[600];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }
    for (int index = 0; index <= 599; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

// IDA_EA: 0x82AF5CA
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EED2Ev
template <>
StageControlPool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B2578
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EE7AcquireEv
template <>
advancealtar::StageControl* StageControlPool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }
    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kStageControlPoolDummy) {
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
    return static_cast<advancealtar::StageControl*>(node);
}

// IDA_EA: 0x82B26A6
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EE8GetIndexEPS1_
template <>
int StageControlPool::GetIndex(advancealtar::StageControl* stage_control) {
    CNode* const node = static_cast<CNode*>(stage_control);
    if (node < nodes) {
        return -1;
    }
    const int index = static_cast<int>(node - nodes);
    if (node->dummy != kStageControlPoolDummy) {
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

// IDA_EA: 0x82B28C8
// IDA_NAME: _ZN10StaticPoolIN12advancealtar12StageControlELi600EE4FreeEPS1_
template <>
advancealtar::StageControl* StageControlPool::Free(
    advancealtar::StageControl* stage_control) {
    CNode* const node = static_cast<CNode*>(stage_control);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
        return 0;
    }
    if (node->dummy != kStageControlPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 5;
        return 0;
    }
    if (node->reference_count == 0u) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 6;
        PrintBackTrace();
        return 0;
    }
    if (node->reference_count > 1u) {
        // Release only the duplicate acquisition.  The node is still active
        // and must not be published back to the free queue.
        --node->reference_count;
        return stage_control;
    }
    --node->reference_count;
    free_nodes.push(node);
    return stage_control;
}
