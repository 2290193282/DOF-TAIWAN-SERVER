#include "old_game/player/core/character_cache_data.h"

#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/player/core/character_db_payloads.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <cstring>
#include <new>


namespace {

const uint32_t kStaticPoolCacheCharacterDummy = 36641u;
const int kStaticPoolCacheCharacterCount = 1000;

}  // namespace

typedef char CACHE_CHARACTER_TYPE_Size_Must_Be_0x40339[
    (sizeof(CACHE_CHARACTER_TYPE) == 0x40339) ? 1 : -1];
typedef char StaticPool_CACHE_CHARACTER_TYPE_1000_CNode_Size_Must_Be_0x40344[
    (sizeof(StaticPool_CACHE_CHARACTER_TYPE_1000::CNode) == 0x40344)
        ? 1
        : -1];
static_assert(offsetof(StaticPool_CACHE_CHARACTER_TYPE_1000::CNode, dummy) ==
                  0x4033cu,
              "cache pool dummy must follow target-aligned payload");
static_assert(offsetof(StaticPool_CACHE_CHARACTER_TYPE_1000::CNode,
                       reference_count) == 0x40340u,
              "cache pool refcount must stay at target offset 0x40340");
typedef char StaticPool_CACHE_CHARACTER_TYPE_1000_Size_Must_Be_0x30[
    (sizeof(StaticPool_CACHE_CHARACTER_TYPE_1000) == 0x30) ? 1 : -1];

// IDA_EA: 0x8492262
// IDA_NAME: _ZN20CACHE_CHARACTER_TYPEC2Ev
// DEMANGLED: CACHE_CHARACTER_TYPE::CACHE_CHARACTER_TYPE(void)
CACHE_CHARACTER_TYPE::CACHE_CHARACTER_TYPE() {
    new (&charac.value) SIG_LOAD_CHARAC();
    new (&inventory.value) SIG_LOAD_INVENTORY();
    new (&quest.value) SIG_LOAD_QUEST();
    new (&action_point.value) APSystem::_SIG_LOAD_ACTION_POINT();
    new (&advance_altar.value) advancealtar::_CharacAdvanceAltarDbData();
    new (&etc.value) SIG_LOAD_ETC();
    new (&kill_monster.value) SIG_LOAD_KILL_MONSTER_INFO();
}

// IDA_EA: 0x84922D6
// IDA_NAME: _ZN20CACHE_CHARACTER_TYPED2Ev
// DEMANGLED: CACHE_CHARACTER_TYPE::~CACHE_CHARACTER_TYPE()
CACHE_CHARACTER_TYPE::~CACHE_CHARACTER_TYPE() {
    reinterpret_cast<SIG_LOAD_ETC*>((&etc.value))->~SIG_LOAD_ETC();
}

// IDA_EA: 0x848FD7C
// IDA_NAME: _ZN20CACHE_CHARACTER_TYPE17GetMemberPositionE19ENUM_INTERNALPACKETPiRb
// DEMANGLED: CACHE_CHARACTER_TYPE::GetMemberPosition(ENUM_INTERNALPACKET,int *,bool &)
uint8_t* CACHE_CHARACTER_TYPE::GetMemberPosition(ENUM_INTERNALPACKET internal_packet,
                                                  int* size,
                                                  bool& loaded) {
    if (internal_packet == 12) {
        loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_DUNGEON_CLEAR);
        *size = 6160;
        return reinterpret_cast<uint8_t*>(&dungeon_clear);
    }
    if (internal_packet > 12) {
        if (internal_packet == 32) {
            loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_ETC);
            *size = 84832;
            return reinterpret_cast<uint8_t*>(&etc.value);
        }
        if (internal_packet > 32) {
            if (internal_packet == 179) {
                loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_BLACK_COUNT);
                *size = 16;
                return reinterpret_cast<uint8_t*>(&black_count);
            }
            if (internal_packet == 642) {
                loaded = load_state.is_loaded(
                    CACHE_CHARACTER_LOAD_KILL_MONSTER);
                *size = 45004;
                return reinterpret_cast<uint8_t*>(&kill_monster.value);
            }
        } else {
            if (internal_packet == 17) {
                loaded = load_state.is_loaded(
                    CACHE_CHARACTER_LOAD_ACTION_POINT);
                *size = 3313;
                return reinterpret_cast<uint8_t*>(&action_point.value);
            }
            if (internal_packet == 25) {
                loaded = load_state.is_loaded(
                    CACHE_CHARACTER_LOAD_ADVANCE_ALTAR);
                *size = 1308;
                return reinterpret_cast<uint8_t*>(&advance_altar.value);
            }
        }
    } else {
        if (internal_packet == 8) {
            loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_INVENTORY);
            *size = sizeof(SIG_LOAD_INVENTORY);
            return reinterpret_cast<uint8_t*>(&inventory.value);
        }
        if (internal_packet > 8) {
            if (internal_packet == 10) {
                loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_QUEST);
                *size = 30216;
                return reinterpret_cast<uint8_t*>(&quest.value);
            }
            if (internal_packet == 11) {
                loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_TICKET);
                *size = 412;
                return reinterpret_cast<uint8_t*>(&ticket);
            }
        } else if (internal_packet == 7) {
            loaded = load_state.is_loaded(CACHE_CHARACTER_LOAD_CHARAC);
            *size = sizeof(SIG_LOAD_CHARAC);
            return reinterpret_cast<uint8_t*>(&charac.value);
        }
    }
    return 0;
}

// IDA_EA: 0x8495DBA
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EE5CNodeC2Ev
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::CNode::CNode(void)
StaticPool<CACHE_CHARACTER_TYPE, 1000>::CNode::CNode()
    : dummy(kStaticPoolCacheCharacterDummy),
      reference_count(0) {
    std::memset(this, 0, sizeof(payload.value));
    new (&payload.value) CACHE_CHARACTER_TYPE();
    dummy = kStaticPoolCacheCharacterDummy;
    reference_count = 0;
}

// IDA_EA: 0x8494D96
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EE5CNodeD2Ev
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::CNode::~CNode()
StaticPool<CACHE_CHARACTER_TYPE, 1000>::CNode::~CNode() {
    payload.value.~CACHE_CHARACTER_TYPE();
}

// IDA_EA: 0x8494DAA
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EEC2Ev
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::StaticPool(void)
StaticPool<CACHE_CHARACTER_TYPE, 1000>::StaticPool()
    : nodes_(0),
      free_nodes_(),
      error_code_(0) {
    nodes_ = static_cast<CNode*>(operator new[](
        sizeof(uint32_t) +
        sizeof(CNode) * kStaticPoolCacheCharacterCount));
    uint32_t* count = reinterpret_cast<uint32_t*>(nodes_);
    *count = kStaticPoolCacheCharacterCount;
    nodes_ = reinterpret_cast<CNode*>(count + 1);

    for (int index = 0; index < kStaticPoolCacheCharacterCount; ++index) {
        new (nodes_ + index) CNode();
    }

    if (!nodes_) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }

    for (int index = 0; index < kStaticPoolCacheCharacterCount; ++index) {
        free_nodes_.push(nodes_ + index);
    }
}

// IDA_EA: 0x8494F58
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EED2Ev
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::~StaticPool()
StaticPool<CACHE_CHARACTER_TYPE, 1000>::~StaticPool() {
    if (nodes_) {
        uint32_t count = *(reinterpret_cast<uint32_t*>(nodes_) - 1);
        for (uint32_t index = count; index > 0; --index) {
            nodes_[index - 1].~CNode();
        }
        operator delete[](reinterpret_cast<uint32_t*>(nodes_) - 1);
    }
}

// IDA_EA: 0x8494996
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EE4FreeEPS0_
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::Free(CACHE_CHARACTER_TYPE*)
CACHE_CHARACTER_TYPE* StaticPool<CACHE_CHARACTER_TYPE, 1000>::Free(
    CACHE_CHARACTER_TYPE* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* pool_node = reinterpret_cast<CNode*>(node);
    free_nodes_.push(pool_node);
    if (pool_node->dummy != kStaticPoolCacheCharacterDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              pool_node->dummy, pool_node);
        error_code_ = 5;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              pool_node->reference_count, pool_node);
        error_code_ = 6;
        PrintBackTrace();
    }
    --pool_node->reference_count;
    return node;
}

// IDA_EA: 0x8494B90
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EE6RemainEv
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::Remain(void)
int StaticPool<CACHE_CHARACTER_TYPE, 1000>::Remain() {
    return static_cast<int>(free_nodes_.size());
}

// IDA_EA: 0x8494BA6
// IDA_NAME: _ZN10StaticPoolI20CACHE_CHARACTER_TYPELi1000EE7AcquireEv
// DEMANGLED: StaticPool<CACHE_CHARACTER_TYPE,1000>::Acquire(void)
CACHE_CHARACTER_TYPE* StaticPool<CACHE_CHARACTER_TYPE, 1000>::Acquire() {
    if (free_nodes_.empty()) {
        return 0;
    }

    CNode* node = free_nodes_.front();
    free_nodes_.pop();
    if (node->dummy != kStaticPoolCacheCharacterDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy, node);
        error_code_ = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count, node);
        error_code_ = 4;
    }
    ++node->reference_count;
    return &node->payload.value;
}
