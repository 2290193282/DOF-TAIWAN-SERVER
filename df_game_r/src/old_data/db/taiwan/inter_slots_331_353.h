#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_331_353_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_331_353_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"


class CUser;

// These packet/reply records are consumed by the original outer
// InterDispatcher slots 331, 334, 343, 348, and 351--353.  Their fields are
// kept named at the offsets actually read by the original consumers; the
// opaque routing portions are retained as one envelope rather than guessed
// into unrelated business fields.
#pragma pack(push, 1)

struct SIG_LOAD_CHARAC_LINK_BONUS {
    uint32_t reward_exp;       // +0x00
    uint32_t mail_item_id;     // +0x04
};

struct SIG_LOAD_DIMENSION {
    uint32_t charac_no;        // +0x00
    uint8_t dimension[6];      // +0x04
    // 140828 DungeonInCountInfo adds dungeon 122 after the six Otherverse
    // slots.  The schema's existing dungeon7 byte is carried at +0x0a.
    uint8_t tournament_inout;  // +0x0a, remaining Black Dragon entries
    uint8_t limited_inout;     // +0x0b, packed 3054/3056/3057 remaining
    uint8_t blood_inout;       // +0x0c
    uint8_t reserved_0d[3];    // +0x0d
};

struct SIG_BLACK_IP_MONITOR_DELETE_ENTRY {
    char ip[16];               // outer reply +0x0e
    uint32_t punish_type;      // outer reply +0x1e
};

struct SIG_BLACK_IP_MONITOR_DELETE {
    uint8_t dispatcher_envelope[10];  // +0x00, not inspected by this slot
    uint32_t count;                    // +0x0a
    SIG_BLACK_IP_MONITOR_DELETE_ENTRY entries[1];  // +0x0e, variable tail
};

// DB_UpdateOneDayLetheSkill::dispatch @0x8433ECA emits this exact 0x1a0
// response.  Its canonical producer-side declaration remains in
// db_update_one_day_lethe_skill.h; this duplicate declaration gives the
// inter dispatcher a standalone, strongly typed receipt surface.
struct SIG_INTER_PREMIUM_LETHE_FLAG {
    uint32_t success;          // +0x000
    uint8_t state;             // +0x004
    uint8_t skill_slot[408];   // +0x005
    uint8_t lethe_flag;        // +0x19d
    uint8_t skill_tree_kind;   // +0x19e
    uint8_t reserved_19f;      // +0x19f
};

struct SIG_DISCONNECT_MONITOR_SERVER {
    uint32_t reconnect_delay_seconds;  // +0x00
};

// The PC-room TCP payloads retain the ten-byte internal PacketHeader and its
// four-byte route key before the character id.  Slots 352/353 deliberately
// use only character_id and the following response fields.
struct SIG_PCROOM_RESPONSE_PREFIX {
    PacketHeader header;       // +0x00
    uint32_t routing_key;      // +0x0a
    uint32_t character_id;     // +0x0e
};

struct SIG_PCROOM_PLAYER_LIST_ENTRY {
    uint8_t field_00;          // response record +0x00
    uint8_t field_01;          // response record +0x01
    uint16_t field_02;         // response record +0x02
    char character_name[30];   // response record +0x04
    uint8_t field_22;          // response record +0x22
    uint8_t field_23;          // response record +0x23
    uint8_t field_24;          // response record +0x24
};

struct SIG_PCROOM_PLAYER_LIST_RESPONSE {
    SIG_PCROOM_RESPONSE_PREFIX prefix; // +0x00
    uint8_t count;                     // +0x12
    SIG_PCROOM_PLAYER_LIST_ENTRY entries[1];  // +0x13, variable tail
};

struct SIG_PCROOM_PLAYER_COUNT_RESPONSE {
    SIG_PCROOM_RESPONSE_PREFIX prefix; // +0x00
    uint8_t player_count;              // +0x12
};

#pragma pack(pop)

static_assert(sizeof(SIG_LOAD_CHARAC_LINK_BONUS) == 0x08u,
              "Inter_LoadCharacLinkBonus reply ABI changed");
static_assert(sizeof(SIG_LOAD_DIMENSION) == 0x10u,
              "Inter_LoadDimensionInout reply ABI changed");
static_assert(offsetof(SIG_BLACK_IP_MONITOR_DELETE, count) == 0x0au,
              "Black-IP delete count must remain at +0x0a");
static_assert(offsetof(SIG_BLACK_IP_MONITOR_DELETE, entries) == 0x0eu,
              "Black-IP delete entries must remain at +0x0e");
static_assert(sizeof(SIG_BLACK_IP_MONITOR_DELETE_ENTRY) == 0x14u,
              "Black-IP delete entry ABI changed");
static_assert(sizeof(SIG_INTER_PREMIUM_LETHE_FLAG) == 0x1a0u,
              "Inter_UpdateOneDayLethe reply ABI changed");
static_assert(offsetof(SIG_INTER_PREMIUM_LETHE_FLAG, lethe_flag) == 0x19du,
              "Lethe flag must remain at +0x19d");
static_assert(offsetof(SIG_INTER_PREMIUM_LETHE_FLAG, skill_tree_kind) ==
                  0x19eu,
              "Lethe skill tree must remain at +0x19e");
static_assert(sizeof(SIG_PCROOM_RESPONSE_PREFIX) == 0x12u,
              "PC-room reply prefix must remain 18 bytes");
static_assert(sizeof(SIG_PCROOM_PLAYER_LIST_ENTRY) == 0x25u,
              "PC-room list record must remain 37 bytes");
static_assert(offsetof(SIG_PCROOM_PLAYER_LIST_RESPONSE, count) == 0x12u,
              "PC-room list count must remain at +0x12");
static_assert(offsetof(SIG_PCROOM_PLAYER_LIST_RESPONSE, entries) == 0x13u,
              "PC-room list entries must remain at +0x13");


class Inter_LoadCharacLinkBonus : public ISigDispatcher {
public:
    Inter_LoadCharacLinkBonus();
    virtual ~Inter_LoadCharacLinkBonus();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadDimensionInout : public ISigDispatcher {
public:
    Inter_LoadDimensionInout();
    virtual ~Inter_LoadDimensionInout();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_BlackIPMonitorDeleteIP : public ISigDispatcher {
public:
    Inter_BlackIPMonitorDeleteIP();
    virtual ~Inter_BlackIPMonitorDeleteIP();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_UpdateOneDayLethe : public ISigDispatcher {
public:
    Inter_UpdateOneDayLethe();
    virtual ~Inter_UpdateOneDayLethe();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_DisconnectMonitorServer : public ISigDispatcher {
public:
    Inter_DisconnectMonitorServer();
    virtual ~Inter_DisconnectMonitorServer();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ResposncePCRoomPlyerList : public ISigDispatcher {
public:
    Inter_ResposncePCRoomPlyerList();
    virtual ~Inter_ResposncePCRoomPlyerList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ResposncePCRoomPlyerCount : public ISigDispatcher {
public:
    Inter_ResposncePCRoomPlyerCount();
    virtual ~Inter_ResposncePCRoomPlyerCount();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
// Every original receiver carries only the ISigDispatcher vptr on the
// 32-bit server ABI.  Keeping this explicit prevents a later data member from
// silently changing the allocator/registration object contract.
static_assert(sizeof(Inter_LoadCharacLinkBonus) == 0x04u,
              "Inter_LoadCharacLinkBonus must remain vptr-only on i386");
static_assert(sizeof(Inter_LoadDimensionInout) == 0x04u,
              "Inter_LoadDimensionInout must remain vptr-only on i386");
static_assert(sizeof(Inter_BlackIPMonitorDeleteIP) == 0x04u,
              "Inter_BlackIPMonitorDeleteIP must remain vptr-only on i386");
static_assert(sizeof(Inter_UpdateOneDayLethe) == 0x04u,
              "Inter_UpdateOneDayLethe must remain vptr-only on i386");
static_assert(sizeof(Inter_DisconnectMonitorServer) == 0x04u,
              "Inter_DisconnectMonitorServer must remain vptr-only on i386");
static_assert(sizeof(Inter_ResposncePCRoomPlyerList) == 0x04u,
              "Inter_ResposncePCRoomPlyerList must remain vptr-only on i386");
static_assert(sizeof(Inter_ResposncePCRoomPlyerCount) == 0x04u,
              "Inter_ResposncePCRoomPlyerCount must remain vptr-only on i386");
#endif


#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_331_353_H_
