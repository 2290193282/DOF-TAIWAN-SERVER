#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/session/online_preliminary_team_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

namespace {

const uint32_t kOnlinePreliminaryTeamPoolDummy = 36641u;

typedef StaticPool<online_preliminary::COnlinePreliminaryTeam, 600>
    OnlinePreliminaryTeamPool;

#if defined(__i386__)
static_assert(sizeof(OnlinePreliminaryTeamPool::CNode) == 0x48u,
              "online-preliminary team pool node must retain the old ABI");
#endif

}  // namespace

// IDA_EA: 0x858A108
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EE5CNodeC2Ev
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::CNode::CNode(void)
template <>
StaticPool<online_preliminary::COnlinePreliminaryTeam, 600>::CNode::CNode()
    : online_preliminary::COnlinePreliminaryTeam(),
      dummy(kOnlinePreliminaryTeamPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x85894DA
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EE5CNodeD2Ev
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::CNode::~CNode()
template <>
StaticPool<online_preliminary::COnlinePreliminaryTeam, 600>::CNode::~CNode() {}

// IDA_EA: 0x85894EE
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EEC2Ev
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::StaticPool(void)
template <>
StaticPool<online_preliminary::COnlinePreliminaryTeam, 600>::StaticPool()
    : nodes(0),
      free_nodes(),
      error_state(0) {
    nodes = new CNode[600];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
        return;
    }

    for (int index = 0; index < 600; ++index) {
        free_nodes.push(nodes + index);
    }
}

// IDA_EA: 0x85896A0
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EED2Ev
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::~StaticPool()
template <>
StaticPool<online_preliminary::COnlinePreliminaryTeam, 600>::~StaticPool() {
    delete[] nodes;
    nodes = 0;
}

// IDA_EA: 0x8588E56
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EE7AcquireEv
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::Acquire(void)
template <>
online_preliminary::COnlinePreliminaryTeam*
OnlinePreliminaryTeamPool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kOnlinePreliminaryTeamPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 3;
    }
    if (node->reference_count != 0u) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              static_cast<int>(static_cast<int8_t>(node->reference_count)),
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<online_preliminary::COnlinePreliminaryTeam*>(node);
}

// IDA_EA: 0x8588F72
// IDA_NAME: _ZN10StaticPoolIN18online_preliminary22COnlinePreliminaryTeamELi600EE4FreeEPS1_
// DEMANGLED: StaticPool<online_preliminary::COnlinePreliminaryTeam,600>::Free(online_preliminary::COnlinePreliminaryTeam*)
template <>
online_preliminary::COnlinePreliminaryTeam*
OnlinePreliminaryTeamPool::Free(
    online_preliminary::COnlinePreliminaryTeam* team) {
    CNode* const node = static_cast<CNode*>(team);
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
        return 0;
    }

    free_nodes.push(node);
    if (node->dummy != kOnlinePreliminaryTeamPoolDummy) {
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
    return team;
}
