#include "old_game/social/party/quick_party_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <cstdint>

namespace {

const std::uint32_t kQuickPartyPoolDummy = 36641u;
typedef StaticPool<QuickParty::CQuickParty, 300> QuickPartyPool;

#if defined(__i386__)
static_assert(sizeof(QuickPartyPool::CNode) == 0x30u,
              "StaticPool<QuickParty::CQuickParty,300>::CNode IDA size");
static_assert(sizeof(QuickPartyPool) == 0x30u,
              "StaticPool<QuickParty::CQuickParty,300> IDA size");
#endif

}  // namespace

// IDA_EA: 0x82B960C
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE5CNodeC2Ev
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::CNode::CNode(void)
template <>
QuickPartyPool::CNode::CNode()
    : QuickParty::CQuickParty(),
      dummy(kQuickPartyPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AF666
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE5CNodeD2Ev
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::CNode::~CNode()
template <>
QuickPartyPool::CNode::~CNode() {}

// IDA_EA: 0x826BA40
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE4SizeEv
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::Size(void)
template <>
int QuickPartyPool::Size() {
    return 300;
}

// IDA_EA: 0x826BA4A
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE6RemainEv
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::Remain(void)
template <>
int QuickPartyPool::Remain() {
    return static_cast<int>(free_nodes.size());
}

// IDA_EA: 0x82AF67A
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EEC2Ev
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::StaticPool(void)
template <>
QuickPartyPool::StaticPool()
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

// IDA_EA: 0x82AF82A
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EED2Ev
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::~StaticPool()
template <>
QuickPartyPool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B17B0
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE7AcquireEv
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::Acquire(void)
template <>
QuickParty::CQuickParty* QuickPartyPool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }
    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kQuickPartyPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              static_cast<int>(static_cast<std::int8_t>(node->reference_count)),
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<QuickParty::CQuickParty*>(node);
}

// IDA_EA: 0x82B18CC
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE8GetIndexEPS1_
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::GetIndex(QuickParty::CQuickParty*)
template <>
int QuickPartyPool::GetIndex(QuickParty::CQuickParty* quick_party) {
    const std::uintptr_t node_address =
        reinterpret_cast<std::uintptr_t>(quick_party);
    const std::uintptr_t nodes_address =
        reinterpret_cast<std::uintptr_t>(nodes);
    if (node_address < nodes_address) {
        return -1;
    }

    // 0x82B18DB--0x82B191C compares raw i386 addresses, then divides their
    // nonnegative byte distance by the observed CNode stride (0x30).
    const std::uintptr_t byte_offset = node_address - nodes_address;
    const int index = byte_offset == 0
        ? 0
        : static_cast<int>(byte_offset / sizeof(CNode));
    CNode* const node = static_cast<CNode*>(quick_party);
    if (node->dummy != kQuickPartyPoolDummy) {
        cMyTrace trace("GetIndex", 80, 0);
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 1;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("GetIndex", 86, 0);
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",
              static_cast<int>(static_cast<std::int8_t>(node->reference_count)),
              node);
        error_state = 2;
    }
    return index <= 299 ? index : -1;
}

// IDA_EA: 0x82B1AE2
// IDA_NAME: _ZN10StaticPoolIN10QuickParty11CQuickPartyELi300EE4FreeEPS1_
// DEMANGLED: StaticPool<QuickParty::CQuickParty,300>::Free(QuickParty::CQuickParty*)
template <>
QuickParty::CQuickParty* QuickPartyPool::Free(
    QuickParty::CQuickParty* quick_party) {
    CNode* const node = static_cast<CNode*>(quick_party);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }
    free_nodes.push(node);
    if (node->dummy != kQuickPartyPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 5;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              static_cast<int>(static_cast<std::int8_t>(node->reference_count)),
              node);
        error_state = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return quick_party;
}
