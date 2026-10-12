#include "old_game/economy/trade/trade_space_pool_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kTradeSpacePoolDummy = 36641u;

int TraceReferenceCount(uint8_t reference_count) {
    return static_cast<int>(static_cast<int8_t>(reference_count));
}

}  // namespace

// IDA_EA: 0x82B8E5A
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE5CNodeC2Ev
// DEMANGLED: StaticPool<CTradeSpace,300>::CNode::CNode(void)
StaticPool<CTradeSpace, 300>::CNode::CNode()
    : CTradeSpace(),
      dummy(kTradeSpacePoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AE5DA
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE5CNodeD2Ev
// DEMANGLED: StaticPool<CTradeSpace,300>::CNode::~CNode()
StaticPool<CTradeSpace, 300>::CNode::~CNode() {}

// IDA_EA: 0x80EE4A0
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE4SizeEv
// DEMANGLED: StaticPool<CTradeSpace,300>::Size(void)
int StaticPool<CTradeSpace, 300>::Size() {
    return kCapacity;
}

// IDA_EA: 0x80EE4AA
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE6RemainEv
// DEMANGLED: StaticPool<CTradeSpace,300>::Remain(void)
int StaticPool<CTradeSpace, 300>::Remain() {
    return static_cast<int>(free_nodes.size());
}

// IDA_EA: 0x82AE5EE
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EEC2Ev
// DEMANGLED: StaticPool<CTradeSpace,300>::StaticPool(void)
StaticPool<CTradeSpace, 300>::StaticPool()
    : free_nodes() {
    nodes = new CNode[kCapacity];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }

    for (int index = 0; index < kCapacity; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

// IDA_EA: 0x82AE79C
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EED2Ev
// DEMANGLED: StaticPool<CTradeSpace,300>::~StaticPool()
StaticPool<CTradeSpace, 300>::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B0982
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE7AcquireEv
// DEMANGLED: StaticPool<CTradeSpace,300>::Acquire(void)
CTradeSpace* StaticPool<CTradeSpace, 300>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kTradeSpacePoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<CTradeSpace*>(node);
}

// IDA_EA: 0x82B0AB0
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE8GetIndexEPS0_
// DEMANGLED: StaticPool<CTradeSpace,300>::GetIndex(CTradeSpace*)
int StaticPool<CTradeSpace, 300>::GetIndex(CTradeSpace* node) {
    const uintptr_t pool_begin = reinterpret_cast<uintptr_t>(nodes);
    const uintptr_t node_address = reinterpret_cast<uintptr_t>(node);
    if (node_address < pool_begin) {
        return -1;
    }

    const uintptr_t distance = node_address - pool_begin;
    int index = 0;
    if (distance) {
        index = static_cast<int>(distance / sizeof(CNode));
    }

    CNode* pool_node = static_cast<CNode*>(node);
    if (pool_node->dummy != kTradeSpacePoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              pool_node->dummy,
              pool_node);
        error_state = 1;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              TraceReferenceCount(pool_node->reference_count),
              pool_node);
        error_state = 2;
    }
    if (index <= kCapacity - 1) {
        return index;
    }
    return -1;
}

// IDA_EA: 0x82B0BEC
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE3GetEi
// DEMANGLED: StaticPool<CTradeSpace,300>::Get(int)
CTradeSpace* StaticPool<CTradeSpace, 300>::Get(int index) {
    if (index < 0 || index >= kCapacity) {
        return 0;
    }

    CNode* node = nodes + index;
    if (node->dummy != kTradeSpacePoolDummy) {
        cMyTrace trace("Get", 195, 0);
        trace("STATIC MEMORY_POOL Get error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 7;
    }
    if (node->reference_count >= 2u) {
        cMyTrace trace("Get", 202, 0);
        trace("STATIC MEMORY_POOL Get error reference count(%d), node(%x)",
              TraceReferenceCount(node->reference_count),
              node);
        error_state = 8;
    }
    return static_cast<CTradeSpace*>(node);
}

// IDA_EA: 0x82B0D02
// IDA_NAME: _ZN10StaticPoolI11CTradeSpaceLi300EE4FreeEPS0_
// DEMANGLED: StaticPool<CTradeSpace,300>::Free(CTradeSpace*)
CTradeSpace* StaticPool<CTradeSpace, 300>::Free(CTradeSpace* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* pool_node = static_cast<CNode*>(node);
    free_nodes.push(pool_node);
    if (pool_node->dummy != kTradeSpacePoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              pool_node->dummy,
              pool_node);
        error_state = 5;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              TraceReferenceCount(pool_node->reference_count),
              pool_node);
        error_state = 6;
        PrintBackTrace();
    }
    --pool_node->reference_count;
    return node;
}
