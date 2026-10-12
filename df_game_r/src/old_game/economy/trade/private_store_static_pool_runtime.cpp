#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kPrivateStorePoolDummy = 36641u;

typedef StaticPool<private_store::CPrivateStore, 300> PrivateStorePool;

#if defined(__i386__)
static_assert(sizeof(PrivateStorePool::CNode) == 0x6cu,
              "CPrivateStore pool nodes must retain the 0x6c-byte IDA stride");
static_assert(sizeof(PrivateStorePool) == 0x30u,
              "CPrivateStore pool must retain the 0x30-byte IDA layout");
#endif

int TraceReferenceCount(uint8_t reference_count) {
    return static_cast<int>(static_cast<int8_t>(reference_count));
}

}  // namespace

// IDA_EA: 0x85CDAB8
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EE5CNodeC2Ev
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::CNode::CNode(void)
template <>
PrivateStorePool::CNode::CNode()
    : private_store::CPrivateStore(),
      dummy(kPrivateStorePoolDummy),
      reference_count(0) {}

// IDA_EA: 0x85CCBC8
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EE5CNodeD2Ev
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::CNode::~CNode()
template <>
PrivateStorePool::CNode::~CNode() {}

// IDA_EA: 0x85CCBEE
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EEC2Ev
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::StaticPool(void)
template <>
PrivateStorePool::StaticPool()
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

// IDA_EA: 0x85CCD90
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EED2Ev
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::~StaticPool()
template <>
PrivateStorePool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x85CD592
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EE7AcquireEv
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::Acquire(void)
template <>
private_store::CPrivateStore* PrivateStorePool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kPrivateStorePoolDummy) {
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
    return static_cast<private_store::CPrivateStore*>(node);
}

// IDA_EA: 0x85CD7A4
// IDA_NAME: _ZN10StaticPoolIN13private_store13CPrivateStoreELi300EE4FreeEPS1_
// DEMANGLED: StaticPool<private_store::CPrivateStore,300>::Free(private_store::CPrivateStore *)
template <>
private_store::CPrivateStore* PrivateStorePool::Free(
    private_store::CPrivateStore* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* const pool_node = static_cast<CNode*>(node);
    free_nodes.push(pool_node);
    if (pool_node->dummy != kPrivateStorePoolDummy) {
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
