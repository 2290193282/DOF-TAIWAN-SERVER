#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kCUserPoolDummy = 36641u;

typedef StaticPool<CUser, 600> CUserPool;

#if defined(__i386__)
static_assert(sizeof(CUserPool::CNode) == sizeof(CUserStaticPool600CNodeLayout),
              "StaticPool<CUser,600>::CNode must retain the observed ABI");
#endif

}  // namespace

// IDA_EA: 0x80EE400
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE4SizeEv
template <>
int CUserPool::Size() {
    return 600;
}

// IDA_EA: 0x80EE40A
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE6RemainEv
template <>
int CUserPool::Remain() {
    return static_cast<int>(free_nodes.size());
}

// IDA_EA: 0x82B040A
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE16GetLastErrorCodeEv
// DEMANGLED: StaticPool<CUser,600>::GetLastErrorCode(void)
template <>
int CUserPool::GetLastErrorCode() {
    return error_state;
}

// IDA_EA: 0x82B8D42
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE5CNodeC2Ev
template <>
CUserPool::CNode::CNode()
    : CUser(),
      dummy(kCUserPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82AE37C
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE5CNodeD2Ev
template <>
CUserPool::CNode::~CNode() {}

// IDA_EA: 0x82AE390
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EEC2Ev
template <>
CUserPool::StaticPool()
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

// IDA_EA: 0x82AE53E
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EED2Ev
template <>
CUserPool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82B02DC
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE7AcquireEv
template <>
CUser* CUserPool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }
    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kCUserPoolDummy) {
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
    return static_cast<CUser*>(node);
}

// IDA_EA: 0x82B0622
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE8GetIndexEPS0_
template <>
int CUserPool::GetIndex(CUser* user) {
    CNode* const node = static_cast<CNode*>(user);
    if (node < nodes) {
        return -1;
    }
    const int index = static_cast<int>(node - nodes);
    if (node->dummy != kCUserPoolDummy) {
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

// IDA_EA: 0x82B082A
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE4FreeEPS0_
template <>
CUser* CUserPool::Free(CUser* user) {
    CNode* const node = static_cast<CNode*>(user);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }
    free_nodes.push(node);
    if (node->dummy != kCUserPoolDummy) {
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
    return user;
}

// IDA_EA: 0x82B050C
// IDA_NAME: _ZN10StaticPoolI5CUserLi600EE3GetEi
// DEMANGLED: StaticPool<CUser,600>::Get(int)
template <>
CUser* StaticPool<CUser, 600>::Get(int index) {
    if (index < 0 || index >= 600) {
        return 0;
    }

    CNode* const node = nodes + index;
    if (node->dummy != kCUserPoolDummy) {
        cMyTrace trace("Get", 195, 0);
        trace("STATIC MEMORY_POOL Get error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 7;
    }
    if (node->reference_count >= 2u) {
        cMyTrace trace("Get", 202, 0);
        trace("STATIC MEMORY_POOL Get error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state = 8;
    }
    return static_cast<CUser*>(node);
}
