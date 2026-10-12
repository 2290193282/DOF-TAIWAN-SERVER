#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/war_room.h"

#include <cstddef>
#include <cstdint>

static const uint32_t kStaticPoolDummy = 36641u;

// The original three checks and error-state transitions are identical for
// both manager-owned pools.  This macro only emits concrete specializations;
// it does not introduce a second generic StaticPool implementation.
#define DEFINE_STATIC_POOL_SPECIALIZATION(TYPE, COUNT)                         \
template <>                                                                     \
int StaticPool<TYPE, COUNT>::Size() {                                           \
    return COUNT;                                                                \
}                                                                                 \
                                                                                  \
template <>                                                                     \
StaticPool<TYPE, COUNT>::CNode::CNode()                                         \
    : TYPE(),                                                                    \
      dummy(kStaticPoolDummy),                                                   \
      reference_count(0) {                                                       \
}                                                                                 \
                                                                                  \
template <>                                                                     \
StaticPool<TYPE, COUNT>::CNode::~CNode() {                                      \
}                                                                                 \
                                                                                  \
template <>                                                                     \
StaticPool<TYPE, COUNT>::StaticPool()                                           \
    : free_nodes() {                                                             \
    nodes = new CNode[COUNT];                                                    \
    if (!nodes) {                                                                \
        cMyTrace trace("StaticPool", 108, 5);                                  \
        trace("[%s][%d]", "StaticPool");                                     \
    }                                                                             \
    for (int index = 0; index != COUNT; ++index) {                              \
        free_nodes.push(nodes + index);                                          \
    }                                                                             \
    error_state = 0;                                                             \
}                                                                                 \
                                                                                  \
template <>                                                                     \
StaticPool<TYPE, COUNT>::~StaticPool() {                                        \
    if (nodes) {                                                                 \
        delete[] nodes;                                                          \
    }                                                                             \
}                                                                                 \
                                                                                  \
template <>                                                                     \
TYPE* StaticPool<TYPE, COUNT>::Free(TYPE* node) {                               \
    if (!node) {                                                                 \
        cMyTrace trace("Free", 160, 5);                                        \
        trace("[%s][%d]", "Free");                                           \
    }                                                                             \
    CNode* const pool_node = static_cast<CNode*>(node);                         \
    free_nodes.push(pool_node);                                                  \
    if (pool_node->dummy != kStaticPoolDummy) {                                 \
        cMyTrace trace("Free", 170, 0);                                        \
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",            \
              pool_node->dummy, pool_node);                                     \
        error_state = 5;                                                         \
    }                                                                             \
    if (pool_node->reference_count != 1) {                                      \
        cMyTrace trace("Free", 176, 0);                                        \
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",  \
              pool_node->reference_count, pool_node);                           \
        error_state = 6;                                                         \
        PrintBackTrace();                                                        \
    }                                                                             \
    --pool_node->reference_count;                                                \
    return node;                                                                 \
}                                                                                 \
                                                                                  \
template <>                                                                     \
int StaticPool<TYPE, COUNT>::Remain() {                                         \
    return static_cast<int>(free_nodes.size());                                 \
}                                                                                 \
                                                                                  \
template <>                                                                     \
TYPE* StaticPool<TYPE, COUNT>::Acquire() {                                      \
    if (free_nodes.empty()) {                                                    \
        return 0;                                                                \
    }                                                                             \
    CNode* const node = free_nodes.front();                                     \
    free_nodes.pop();                                                            \
    if (node->dummy != kStaticPoolDummy) {                                      \
        cMyTrace trace("Acquire", 137, 0);                                     \
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",         \
              node->dummy, node);                                                \
        error_state = 3;                                                         \
    }                                                                             \
    if (node->reference_count) {                                                 \
        cMyTrace trace("Acquire", 143, 0);                                     \
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",\
              node->reference_count, node);                                     \
        error_state = 4;                                                         \
    }                                                                             \
    ++node->reference_count;                                                     \
    return static_cast<TYPE*>(node);                                            \
}                                                                                 \
                                                                                  \
template <>                                                                     \
TYPE* StaticPool<TYPE, COUNT>::Get(int index) {                                 \
    if (index < 0 || index >= COUNT) {                                          \
        return 0;                                                                \
    }                                                                             \
    return nodes + index;                                                        \
}                                                                                 \
                                                                                  \
template <>                                                                     \
int StaticPool<TYPE, COUNT>::GetIndex(TYPE* node) {                             \
    if (reinterpret_cast<std::uintptr_t>(node) <                                \
        reinterpret_cast<std::uintptr_t>(nodes)) {                              \
        return -1;                                                               \
    }                                                                             \
    const std::size_t byte_offset =                                              \
        reinterpret_cast<std::uintptr_t>(node) -                                \
        reinterpret_cast<std::uintptr_t>(nodes);                                \
    const int index = byte_offset == 0                                           \
        ? 0                                                                       \
        : static_cast<int>(byte_offset / sizeof(CNode));                        \
    CNode* const pool_node = static_cast<CNode*>(node);                         \
    if (pool_node->dummy != kStaticPoolDummy) {                                 \
        cMyTrace trace("GetIndex", 80, 0);                                     \
        trace("STATIC MEMORY_POOL GetIndex error dummy(%x), node(%x)",        \
              pool_node->dummy, pool_node);                                     \
        error_state = 1;                                                         \
    }                                                                             \
    if (pool_node->reference_count != 1) {                                      \
        cMyTrace trace("GetIndex", 86, 0);                                     \
        trace("STATIC MEMORY_POOL GetIndex error reference count(%d), node(%x)",\
              pool_node->reference_count, pool_node);                           \
        error_state = 2;                                                         \
    }                                                                             \
    return index <= COUNT - 1 ? index : -1;                                     \
}

// The following explicit instantiation emits these three PvP-room-specific
// old-server providers; the generic macro above is deliberately shared with
// the independent WarRoom pool and must not be credited to that ABI.
//
// IDA_EA: 0x80EE480
// IDA_NAME: _ZN10StaticPoolI8PvP_RoomLi600EE4SizeEv
// DEMANGLED: StaticPool<PvP_Room,600>::Size(void)
//
// IDA_EA: 0x80EE48A
// IDA_NAME: _ZN10StaticPoolI8PvP_RoomLi600EE6RemainEv
// DEMANGLED: StaticPool<PvP_Room,600>::Remain(void)
//
// IDA_EA: 0x82AEA88
// IDA_NAME: _ZN10StaticPoolI8PvP_RoomLi600EEC2Ev
// DEMANGLED: StaticPool<PvP_Room,600>::StaticPool(void)
DEFINE_STATIC_POOL_SPECIALIZATION(PvP_Room, 600)
// WarRoom is an independent old-server pool with the same typed implementation
// shape.  These markers belong only to this explicit `WarRoom,40`
// instantiation; they do not claim any PvP_Room identity above.
//
// IDA_EA: 0x80EE440
// IDA_NAME: _ZN10StaticPoolI7WarRoomLi40EE4SizeEv
// DEMANGLED: StaticPool<WarRoom,40>::Size(void)
//
// IDA_EA: 0x80EE44A
// IDA_NAME: _ZN10StaticPoolI7WarRoomLi40EE6RemainEv
// DEMANGLED: StaticPool<WarRoom,40>::Remain(void)
//
// IDA_EA: 0x82AECE6
// IDA_NAME: _ZN10StaticPoolI7WarRoomLi40EEC2Ev
// DEMANGLED: StaticPool<WarRoom,40>::StaticPool(void)
//
// IDA_EA: 0x82B49BA
// IDA_NAME: _ZN10StaticPoolI7WarRoomLi40EE7AcquireEv
// DEMANGLED: StaticPool<WarRoom,40>::Acquire(void)
//
// IDA_EA: 0x82B4AE8
// IDA_NAME: _ZN10StaticPoolI7WarRoomLi40EE8GetIndexEPS0_
// DEMANGLED: StaticPool<WarRoom,40>::GetIndex(WarRoom*)
DEFINE_STATIC_POOL_SPECIALIZATION(WarRoom, 40)

#undef DEFINE_STATIC_POOL_SPECIALIZATION

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
typedef StaticPool<PvP_Room, 600> PvpRoomPool;
typedef StaticPool<WarRoom, 40> WarRoomPool;
static_assert(sizeof(PvpRoomPool::CNode) == 0x6f0u,
              "StaticPool<PvP_Room,600>::CNode must stay 0x6f0");
static_assert(sizeof(WarRoomPool::CNode) == 0x1f8u,
              "StaticPool<WarRoom,40>::CNode must stay 0x1f8");
static_assert(sizeof(PvpRoomPool) == 0x30u,
              "StaticPool<PvP_Room,600> must stay 0x30");
static_assert(sizeof(WarRoomPool) == 0x30u,
              "StaticPool<WarRoom,40> must stay 0x30");
#endif
