#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kCPartyPoolDummy = 36641u;

typedef StaticPool<CParty, 600> CPartyPool;

#if defined(__i386__)
static_assert(sizeof(CPartyPool::CNode) == 0x1b08u,
              "StaticPool<CParty,600>::CNode must retain the observed ABI");
#endif

}  // namespace

// IDA_EA: 0x80EE420
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE4SizeEv
template <>
int CPartyPool::Size() {
    return 600;
}

// IDA_EA: 0x80EE42A
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE6RemainEv
template <>
int CPartyPool::Remain() {
    return static_cast<int>(free_nodes.size());
}

// IDA_EA: 0x82B8F72
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE5CNodeC2Ev
template <>
CPartyPool::CNode::CNode()
    : CParty(),
      dummy(kCPartyPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AE816
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE5CNodeD2Ev
template <>
CPartyPool::CNode::~CNode() {}

// IDA_EA: 0x82AE82A
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EEC2Ev
template <>
CPartyPool::StaticPool()
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

// IDA_EA: 0x82AE9D8
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EED2Ev
template <>
CPartyPool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: CGameManager consumers only call the original acquire/index/free
// trio below.  The old specialization intentionally has no Get(int) member.

// IDA_EA: 0x82B0E5A
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE7AcquireEv
// DEMANGLED: StaticPool<CParty,600>::Acquire(void)
template <>
CParty* StaticPool<CParty, 600>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kCPartyPoolDummy) {
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
    return static_cast<CParty*>(node);
}

// IDA_EA: 0x82B0F88
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE8GetIndexEPS0_
// DEMANGLED: StaticPool<CParty,600>::GetIndex(CParty*)
template <>
int StaticPool<CParty, 600>::GetIndex(CParty* party) {
    CNode* const node = static_cast<CNode*>(party);
    if (node < nodes) {
        return -1;
    }

    const int index = static_cast<int>(node - nodes);
    if (node->dummy != kCPartyPoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 1;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state = 2;
    }
    return index <= 599 ? index : -1;
}

// IDA_EA: 0x82B11AA
// IDA_NAME: _ZN10StaticPoolI6CPartyLi600EE4FreeEPS0_
// DEMANGLED: StaticPool<CParty,600>::Free(CParty*)
template <>
CParty* StaticPool<CParty, 600>::Free(CParty* party) {
    CNode* const node = static_cast<CNode*>(party);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    free_nodes.push(node);
    if (node->dummy != kCPartyPoolDummy) {
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
    return party;
}
