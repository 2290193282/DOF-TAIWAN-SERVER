#include "old_game/appearance/pet/creature.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <stdint.h>

namespace {

const uint32_t kDynamicPoolDummy = 36641u;

#if defined(__i386__)
static_assert(sizeof(DynamicPool<WongWork::Avatar_Item>) == 0x5cu,
              "DynamicPool must retain the old i386 object extent");
static_assert(sizeof(DynamicPool<WongWork::Avatar_Item>::CNode) == 0x4cu,
              "Avatar dynamic-pool node must retain the old i386 extent");
static_assert(sizeof(DynamicPool<user_creature::CCreature>::CNode) == 0x5cu,
              "CCreature dynamic-pool node must retain the old i386 extent");
static_assert(sizeof(DynamicPool<user_creature::CEgg>::CNode) == 0x24u,
              "CEgg dynamic-pool node must retain the old i386 extent");
#endif

}  // namespace

// IDA_EA: 0x82FB4DE
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE5CNodeC2Ev
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::CNode::CNode(void)
template <>
DynamicPool<WongWork::Avatar_Item>::CNode::CNode()
    : WongWork::Avatar_Item(),
      dummy(kDynamicPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82FAC3A
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE5CNodeD2Ev
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::CNode::~CNode(void)
template <>
DynamicPool<WongWork::Avatar_Item>::CNode::~CNode() {}

// IDA_EA: 0x8344880
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE5CNodeC2Ev
// DEMANGLED: DynamicPool<user_creature::CCreature>::CNode::CNode(void)
template <>
DynamicPool<user_creature::CCreature>::CNode::CNode()
    : user_creature::CCreature(),
      dummy(kDynamicPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x8342496
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE5CNodeD2Ev
// DEMANGLED: DynamicPool<user_creature::CCreature>::CNode::~CNode(void)
template <>
DynamicPool<user_creature::CCreature>::CNode::~CNode() {}

// IDA_EA: 0x83449A2
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE5CNodeC2Ev
// DEMANGLED: DynamicPool<user_creature::CEgg>::CNode::CNode(void)
template <>
DynamicPool<user_creature::CEgg>::CNode::CNode()
    : user_creature::CEgg(),
      dummy(kDynamicPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x83421EA
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE5CNodeD2Ev
// DEMANGLED: DynamicPool<user_creature::CEgg>::CNode::~CNode(void)
template <>
DynamicPool<user_creature::CEgg>::CNode::~CNode() {}

template <typename T>
DynamicPool<T>::DynamicPool(int initial_count, int growth_count)
    : exhausted_callback_(0),
      error_state_(0),
      free_nodes_(),
      allocated_blocks_(),
      total_node_count_(0),
      cleared_bookkeeping_(0),
      initial_block_count_(static_cast<uint16_t>(initial_count)),
      growth_block_count_(static_cast<uint16_t>(growth_count)),
      allocated_node_map_() {
    allocated_blocks_.clear();
    MoreAlloc(initial_count);
    error_state_ = 0;
    exhausted_callback_ = 0;
}

template <typename T>
DynamicPool<T>::DynamicPool(int initial_count)
    : DynamicPool(initial_count, initial_count) {}

template <typename T>
DynamicPool<T>::~DynamicPool() {
    for (typename std::list<CNode*>::iterator it = allocated_blocks_.begin();
         it != allocated_blocks_.end();
         ++it) {
        if (*it) {
            delete[] *it;
        }
    }
    allocated_blocks_.clear();
    exhausted_callback_ = 0;
}

template <typename T>
int DynamicPool<T>::MoreAlloc(int count) {
    CNode* const block = new CNode[count];
    if (!block) {
        cMyTrace trace("MoreAlloc", 678, 5);
        trace("DYNAMIC MEMORY_POOL Acquire error [%s][%d]", "MoreAlloc", 678);
        return 0;
    }

    allocated_blocks_.push_back(block);
    for (int index = 0; index < count; ++index) {
        CNode* const node = block + index;
        free_nodes_.push(node);
        allocated_node_map_[node] = 1;
    }
    total_node_count_ += count;
    return 1;
}

template <typename T>
T* DynamicPool<T>::Acquire() {
    if (free_nodes_.empty()) {
        MoreAlloc(growth_block_count_);
        if (free_nodes_.size() > growth_block_count_) {
            cMyTrace trace("Acquire", 412, 0);
            trace("DYNAMIC MEMORY_POOL More Alloc Size(%d)",
                  static_cast<int>(free_nodes_.size()));
        }
        if (exhausted_callback_) {
            exhausted_callback_();
        }
    }

    if (free_nodes_.empty()) {
        LogManager::logFormat(1,
                              "Container.h",
                              "Acquire",
                              0x1A6,
                              "[Pool Err] Insufficient memory!");
        return 0;
    }

    CNode* const node = free_nodes_.front();
    if (node->dummy != kDynamicPoolDummy) {
        cMyTrace trace("Acquire", 435, 0);
        trace("DYNAMIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state_ = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 441, 0);
        trace("DYNAMIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state_ = 4;
    }
    ++node->reference_count;
    if (!node) {
        cMyTrace trace("Acquire", 452, 5);
        trace("DYNAMIC MEMORY_POOL Acquire error [%s][%d]", "Acquire", 452);
    }
    free_nodes_.pop();
    return static_cast<T*>(node);
}

// The three explicit providers below are the original template instances.
// They share this canonical body; no forwarding or raw-storage shim is used.
template <typename T>
int DynamicPool<T>::Free(T* object) {
    if (!object) {
        return 0;
    }

    CNode* const node = static_cast<CNode*>(object);
    free_nodes_.push(node);
    if (node->dummy != kDynamicPoolDummy) {
        cMyTrace trace("Free", 617, 0);
        trace("DYNAMIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state_ = 5;
    }
    if (node->reference_count != 1u) {
        cMyTrace trace("Free", 623, 0);
        trace("DYNAMIC MEMORY_POOL Free error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state_ = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return 1;
}

template <typename T>
int DynamicPool<T>::Remain() {
    return static_cast<int>(free_nodes_.size());
}

template <typename T>
int DynamicPool<T>::Size() {
    return total_node_count_;
}

// IDA_EA: 0x82FA3FC
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEEC2Eii
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::DynamicPool(int,int)
template DynamicPool<WongWork::Avatar_Item>::DynamicPool(int, int);

// IDA_EA: 0x82FD1A2
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEED2Ev
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::~DynamicPool(void)
template DynamicPool<WongWork::Avatar_Item>::~DynamicPool();

// IDA_EA: 0x82FAC4E
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE9MoreAllocEi
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::MoreAlloc(int)
template int DynamicPool<WongWork::Avatar_Item>::MoreAlloc(int);

// IDA_EA: 0x82FA51C
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE7AcquireEv
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::Acquire(void)
template WongWork::Avatar_Item* DynamicPool<WongWork::Avatar_Item>::Acquire();

// IDA_EA: 0x82FA8A0
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE4FreeEPS1_
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::Free(WongWork::Avatar_Item*)
template int DynamicPool<WongWork::Avatar_Item>::Free(WongWork::Avatar_Item*);

// IDA_EA: 0x80EE4C0
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE6RemainEv
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::Remain(void)
template int DynamicPool<WongWork::Avatar_Item>::Remain();

// IDA_EA: 0x80EE4D6
// IDA_NAME: _ZN11DynamicPoolIN8WongWork11Avatar_ItemEE4SizeEv
// DEMANGLED: DynamicPool<WongWork::Avatar_Item>::Size(void)
template int DynamicPool<WongWork::Avatar_Item>::Size();

// IDA_EA: 0x8342374
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEEC2Ei
// DEMANGLED: DynamicPool<user_creature::CCreature>::DynamicPool(int)
template DynamicPool<user_creature::CCreature>::DynamicPool(int);

// IDA_EA: 0x83424B2
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEED2Ev
// DEMANGLED: DynamicPool<user_creature::CCreature>::~DynamicPool(void)
template DynamicPool<user_creature::CCreature>::~DynamicPool();

// IDA_EA: 0x8343358
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE9MoreAllocEi
// DEMANGLED: DynamicPool<user_creature::CCreature>::MoreAlloc(int)
template int DynamicPool<user_creature::CCreature>::MoreAlloc(int);

// IDA_EA: 0x83419F8
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE7AcquireEv
// DEMANGLED: DynamicPool<user_creature::CCreature>::Acquire(void)
template user_creature::CCreature*
DynamicPool<user_creature::CCreature>::Acquire();

// IDA_EA: 0x83419B4
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE6RemainEv
// DEMANGLED: DynamicPool<user_creature::CCreature>::Remain(void)
template int DynamicPool<user_creature::CCreature>::Remain();

// IDA_EA: 0x83419E0
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE4SizeEv
// DEMANGLED: DynamicPool<user_creature::CCreature>::Size(void)
template int DynamicPool<user_creature::CCreature>::Size();

// IDA_EA: 0x83420C8
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEEC2Ei
// DEMANGLED: DynamicPool<user_creature::CEgg>::DynamicPool(int)
template DynamicPool<user_creature::CEgg>::DynamicPool(int);

// IDA_EA: 0x8342206
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEED2Ev
// DEMANGLED: DynamicPool<user_creature::CEgg>::~DynamicPool(void)
template DynamicPool<user_creature::CEgg>::~DynamicPool();

// IDA_EA: 0x8343526
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE9MoreAllocEi
// DEMANGLED: DynamicPool<user_creature::CEgg>::MoreAlloc(int)
template int DynamicPool<user_creature::CEgg>::MoreAlloc(int);

// IDA_EA: 0x8341C3C
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE7AcquireEv
// DEMANGLED: DynamicPool<user_creature::CEgg>::Acquire(void)
template user_creature::CEgg* DynamicPool<user_creature::CEgg>::Acquire();

// IDA_EA: 0x83419CA
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE6RemainEv
// DEMANGLED: DynamicPool<user_creature::CEgg>::Remain(void)
template int DynamicPool<user_creature::CEgg>::Remain();

// IDA_EA: 0x83419EC
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE4SizeEv
// DEMANGLED: DynamicPool<user_creature::CEgg>::Size(void)
template int DynamicPool<user_creature::CEgg>::Size();

// IDA_EA: 0x8341E80
// IDA_NAME: _ZN11DynamicPoolIN13user_creature9CCreatureEE4FreeEPS1_
// DEMANGLED: DynamicPool<user_creature::CCreature>::Free(user_creature::CCreature*)
template int DynamicPool<user_creature::CCreature>::Free(
    user_creature::CCreature*);

// IDA_EA: 0x8341F90
// IDA_NAME: _ZN11DynamicPoolIN13user_creature4CEggEE4FreeEPS1_
// DEMANGLED: DynamicPool<user_creature::CEgg>::Free(user_creature::CEgg*)
template int DynamicPool<user_creature::CEgg>::Free(user_creature::CEgg*);

// IDA data: _ZN8WongWork14CAvatarItemMgr16m_AvatarItemPoolE at 0x943BD80.
// The old global constructor at 0x82FA1A3 invokes C2(15000, 1000) and then
// registers D2, which this static definition reproduces through the C++ ABI.
DynamicPool<WongWork::Avatar_Item> WongWork::CAvatarItemMgr::m_AvatarItemPool(
    15000,
    1000);
