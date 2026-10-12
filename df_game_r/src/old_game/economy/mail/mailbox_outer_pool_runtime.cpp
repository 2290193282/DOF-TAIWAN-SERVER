#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kMailBoxPoolDummy = 36641u;

typedef StaticPool<WongWork::CMailBox, 600> MailBoxPool;

typedef char MailBoxPool_Node_Size_Must_Be_0xB0[
    (sizeof(MailBoxPool::CNode) == 0xB0) ? 1 : -1];
typedef char MailBoxPool_Size_Must_Be_0x30[
    (sizeof(MailBoxPool) == 0x30) ? 1 : -1];
typedef char MailBoxPool_Nodes_Offset_Must_Be_0x00[
    (offsetof(MailBoxPool, nodes) == 0x00) ? 1 : -1];
typedef char MailBoxPool_FreeNodes_Offset_Must_Be_0x04[
    (offsetof(MailBoxPool, free_nodes) == 0x04) ? 1 : -1];
typedef char MailBoxPool_ErrorState_Offset_Must_Be_0x2c[
    (offsetof(MailBoxPool, error_state) == 0x2c) ? 1 : -1];

}  // namespace

// The generic implementation is emitted only for the exact old-server
// StaticPool<CMailBox,600> members explicitly instantiated below.  The
// already-restored CMailBox::CMail,20 specialization has its own provider.
template <typename Type, int Count>
StaticPool<Type, Count>::CNode::CNode()
    : Type(),
      dummy(kMailBoxPoolDummy),
      reference_count(0) {}

template <typename Type, int Count>
StaticPool<Type, Count>::CNode::~CNode() {}

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

template <typename Type, int Count>
Type* StaticPool<Type, Count>::Free(Type* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* pool_node = static_cast<CNode*>(node);
    free_nodes.push(pool_node);
    if (pool_node->dummy != kMailBoxPoolDummy) {
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
Type* StaticPool<Type, Count>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kMailBoxPoolDummy) {
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

// IDA_EA: 0x8558FDE
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EE5CNodeC2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::CNode::CNode(void)
template MailBoxPool::CNode::CNode();

// IDA_EA: 0x8557FD6
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EE5CNodeD2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::CNode::~CNode()
template MailBoxPool::CNode::~CNode();

// IDA_EA: 0x8557FEA
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EEC2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::StaticPool(void)
template MailBoxPool::StaticPool();

// IDA_EA: 0x855C858
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EED2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::~StaticPool()
template MailBoxPool::~StaticPool();

// IDA_EA: 0x8698354
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EE7AcquireEv
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::Acquire(void)
template WongWork::CMailBox* MailBoxPool::Acquire();

// IDA_EA: 0x8698482
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBoxELi600EE4FreeEPS1_
// DEMANGLED: StaticPool<WongWork::CMailBox,600>::Free(WongWork::CMailBox*)
template WongWork::CMailBox* MailBoxPool::Free(WongWork::CMailBox* node);

namespace WongWork {

// IDA data: 0x943E000
// IDA_NAME: _ZN8WongWork14CMailBoxHelper13g_MailBoxPoolE
StaticPool<CMailBox, 600> CMailBoxHelper::g_MailBoxPool;

// IDA_EA: 0x8697838
// IDA_NAME: _ZN8WongWork14CMailBoxHelper13AssignMailBoxEv
// DEMANGLED: WongWork::CMailBoxHelper::AssignMailBox(void)
CMailBox* CMailBoxHelper::AssignMailBox() {
    CMailBox* mailbox = g_MailBoxPool.Acquire();
    if (!mailbox) {
        return 0;
    }

    mailbox->reset();
    return mailbox;
}

// IDA_EA: 0x869786A
// IDA_NAME: _ZN8WongWork14CMailBoxHelper11FreeMailBoxEPNS_8CMailBoxE
// DEMANGLED: WongWork::CMailBoxHelper::FreeMailBox(WongWork::CMailBox *)
CMailBox* CMailBoxHelper::FreeMailBox(CMailBox* mailbox) {
    return g_MailBoxPool.Free(mailbox);
}

}  // namespace WongWork
