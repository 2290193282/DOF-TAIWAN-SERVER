#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_CACHE_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_CACHE_DATA_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8492262 in the companion source.


#include <stddef.h>
#include <stdint.h>

#include <queue>

#include "old_data/db/anti_cheat/db_req_black_count.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_data/db/inventory/load_ticket_payload.h"
#include "old_data/db/legacy_dispatch/action_point_payload.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/character_db_payloads.h"
#include "infra/network/packet/stream.h"

enum ENUM_INTERNALPACKET : int;

// Several embedded wire payloads have a C2 but no matching old-server D2.
// This typed union stores one such payload without synthesizing an extra
// automatic constructor/destructor edge.  CACHE_CHARACTER_TYPE invokes only
// the C2/D2 calls observed in IDA.
template <typename T>
union CacheCharacterPayloadSlot {
    T value;

    CacheCharacterPayloadSlot() {}
    ~CacheCharacterPayloadSlot() {}
};

// IDA_EA: CACHE_CHARACTER_TYPE::GetMemberPosition @0x848FD7C.  The indices
// are also proved by the matching DisableCharacInfo calls in the character
// selection request chain.  Slot 6 has no observed GetMemberPosition branch,
// so it deliberately retains only its physical section identity.
enum CacheCharacterLoadSection : unsigned int {
    CACHE_CHARACTER_LOAD_CHARAC = 0,
    CACHE_CHARACTER_LOAD_INVENTORY = 1,
    CACHE_CHARACTER_LOAD_SKILL = 2,
    CACHE_CHARACTER_LOAD_QUEST = 3,
    CACHE_CHARACTER_LOAD_TICKET = 4,
    CACHE_CHARACTER_LOAD_DUNGEON_CLEAR = 5,
    CACHE_CHARACTER_LOAD_SECTION_6 = 6,
    CACHE_CHARACTER_LOAD_BLACK_COUNT = 7,
    CACHE_CHARACTER_LOAD_ACTION_POINT = 8,
    CACHE_CHARACTER_LOAD_KILL_MONSTER = 9,
    CACHE_CHARACTER_LOAD_ADVANCE_ALTAR = 10,
    CACHE_CHARACTER_LOAD_ETC = 11,
    CACHE_CHARACTER_LOAD_SECTION_COUNT = 12
};

// IDA_EA: CACHE_CHARACTER_TYPE::GetMemberPosition @0x848FD7C.  These are
// physical per-payload completion flags copied from expanded CUser+0x8D44D by
// CUser::CacheCharacterMemory @0x864C85A.  Packet sections 7, 8, 10, 11,
// 12, 179, 17, 642, 25, and 32 map to the explicitly named bytes below.
// Section 6 has no observed GetMemberPosition branch, so its semantic owner
// cannot be inferred beyond its physical cache-section identity.
struct CacheCharacterLoadState {
    // EnableCharacInfo/DisableCharacInfo index this table directly by the
    // original int-backed CACHE_SECTION.  The packet identities documented
    // above describe indices in one table, not twelve unrelated members.
    bool is_loaded(CacheCharacterLoadSection section) const {
        return section_loaded[static_cast<unsigned int>(section)] != 0;
    }

    uint8_t section_loaded[CACHE_CHARACTER_LOAD_SECTION_COUNT];  // +0x00
};

static_assert(sizeof(CacheCharacterLoadState) == 0x0cu,
              "cache payload completion flags must stay 12 bytes");
static_assert(offsetof(CacheCharacterLoadState, section_loaded) == 0x00u,
              "cache section table must begin at zero");

// IDA unit: 04_character_select_create/CACHE_CHARACTER_TYPE.
// C2 @0x8492262 constructs the seven non-trivial payload subobjects at the
// exact offsets asserted below.  The four-byte access timestamp is written by
// exchange_server::CCacheCharacterMgr::AcquireCharacter @0x8490A8A before the
// kill-monster payload; it is not anonymous padding.
// The target C2 places SIG_LOAD_INVENTORY immediately after the 0x740-byte
// SIG_LOAD_CHARAC at +0x740.  Therefore only this enclosing aggregate uses
// byte packing; every embedded payload retains its own canonical ABI.
#pragma pack(push, 1)
struct CACHE_CHARACTER_TYPE {
    CACHE_CHARACTER_TYPE();
    ~CACHE_CHARACTER_TYPE();

    uint8_t* GetMemberPosition(ENUM_INTERNALPACKET internal_packet,
                               int* size,
                               bool& loaded);

    CacheCharacterPayloadSlot<SIG_LOAD_CHARAC> charac; // +0x000000
    CacheCharacterPayloadSlot<SIG_LOAD_INVENTORY>
        inventory;                                  // +0x000740
    CacheCharacterPayloadSlot<SIG_LOAD_QUEST> quest; // +0x01662c
    SIG_LOAD_TICKET ticket;                         // +0x01dc34
    SIG_LOAD_DUNGEON_CLEAR dungeon_clear;           // +0x01ddd0
    SIG_REQ_BLACK_COUNT black_count;                // +0x01f5e0
    CacheCharacterPayloadSlot<APSystem::_SIG_LOAD_ACTION_POINT>
        action_point;                               // +0x01f5f0
    CacheCharacterPayloadSlot<advancealtar::_CharacAdvanceAltarDbData>
        advance_altar;                              // +0x0202e1
    CacheCharacterPayloadSlot<SIG_LOAD_ETC> etc;    // +0x0207fd
    uint32_t last_access_time;                      // +0x03535d
    CacheCharacterPayloadSlot<SIG_LOAD_KILL_MONSTER_INFO>
        kill_monster;                               // +0x035361
    CacheCharacterLoadState load_state;             // +0x04032d
};
#pragma pack(pop)

static_assert(offsetof(CACHE_CHARACTER_TYPE, charac) == 0x000000u,
              "cache character payload must begin at zero");
static_assert(offsetof(CACHE_CHARACTER_TYPE, inventory) == 0x000740u,
              "cache inventory offset must stay 0x740");
static_assert(offsetof(CACHE_CHARACTER_TYPE, quest) == 0x01662cu,
              "cache quest offset must stay 0x1662c");
static_assert(offsetof(CACHE_CHARACTER_TYPE, ticket) == 0x01dc34u,
              "cache ticket offset must stay 0x1dc34");
static_assert(offsetof(CACHE_CHARACTER_TYPE, dungeon_clear) == 0x01ddd0u,
              "cache dungeon-clear offset must stay 0x1ddd0");
static_assert(offsetof(CACHE_CHARACTER_TYPE, black_count) == 0x01f5e0u,
              "cache blacklist offset must stay 0x1f5e0");
static_assert(offsetof(CACHE_CHARACTER_TYPE, action_point) == 0x01f5f0u,
              "cache action-point offset must stay 0x1f5f0");
static_assert(offsetof(CACHE_CHARACTER_TYPE, advance_altar) == 0x0202e1u,
              "cache advance-altar offset must stay 0x202e1");
static_assert(offsetof(CACHE_CHARACTER_TYPE, etc) == 0x0207fdu,
              "cache ETC offset must stay 0x207fd");
static_assert(offsetof(CACHE_CHARACTER_TYPE, last_access_time) == 0x03535du,
              "cache last-access offset must stay 0x3535d");
static_assert(offsetof(CACHE_CHARACTER_TYPE, kill_monster) == 0x035361u,
              "cache kill-monster offset must stay 0x35361");
static_assert(offsetof(CACHE_CHARACTER_TYPE, load_state) == 0x04032du,
              "cache completion-state offset must stay 0x4032d");
static_assert(sizeof(CACHE_CHARACTER_TYPE) == 0x040339u,
              "CACHE_CHARACTER_TYPE target ABI size must stay 0x40339");

// IDA unit: 04_character_select_create/StaticPool_CACHE_CHARACTER_TYPE_1000.
// The original exported template spelling is ABI-significant: keeping the
// specialization instead of a descriptive replacement class preserves the
// StaticPool<CACHE_CHARACTER_TYPE,1000> symbols consumed by exchange_server.
template <typename T, int Count>
class StaticPool;

template <>
class StaticPool<CACHE_CHARACTER_TYPE, 1000> {
public:
    class CNode {
    public:
        CNode();
        ~CNode();

        // The old CNode first clears the whole node and then explicitly runs
        // CACHE_CHARACTER_TYPE C2.  This typed slot prevents C++ from
        // synthesizing an earlier payload lifetime before that observed
        // memset/placement-new sequence.
        CacheCharacterPayloadSlot<CACHE_CHARACTER_TYPE>
            payload;  // CACHE_CHARACTER_TYPE at node +0x00
        uint32_t dummy;           // target +0x4033c
        uint8_t reference_count;  // target +0x40340
    };

    StaticPool();
    ~StaticPool();

    CACHE_CHARACTER_TYPE* Free(CACHE_CHARACTER_TYPE* node);
    int Remain();
    CACHE_CHARACTER_TYPE* Acquire();

private:
    CNode* nodes_;  // IDA +0x00
    std::queue<CNode*> free_nodes_;  // IDA +0x04
    int error_code_;  // IDA +0x2c
};

typedef StaticPool<CACHE_CHARACTER_TYPE, 1000>
    StaticPool_CACHE_CHARACTER_TYPE_1000;


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_CACHE_DATA_H_
