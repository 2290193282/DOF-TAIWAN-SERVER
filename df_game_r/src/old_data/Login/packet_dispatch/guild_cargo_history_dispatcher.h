#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_CARGO_HISTORY_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_CARGO_HISTORY_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/auction_public_values.h"
#include "old_game/player/core/creature_guild_params.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Packet_Guild_Cargo_History_Request.
// This exact 18-byte wire request is constructed locally by CMD266 before it
// is forwarded through the selected CGuildServerProxy TCP endpoint.
#pragma pack(push, 1)
struct Packet_Guild_Cargo_History_Request : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e

    // IDA_EA: 0x822BE8A
    // IDA_NAME: _ZN34Packet_Guild_Cargo_History_RequestC2Ev
    Packet_Guild_Cargo_History_Request();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Guild_Cargo_History_Request) == 0x12u,
              "guild cargo-history request must retain its 18-byte ABI");
static_assert(offsetof(Packet_Guild_Cargo_History_Request,
                       charac_guildkey) == 0x0au &&
                  offsetof(Packet_Guild_Cargo_History_Request, charac_no) ==
                      0x0eu,
              "guild cargo-history request field offsets changed");

// IDA unit: 06_userinfo_stat_inventory/Packet_Guild_Cargo_Requst.  The
// retail spelling is Requst, and it is part of the original C++ ABI.  CMD265
// and CMD266 differ only in the TCP packet identifier and history reason.
#pragma pack(push, 1)
struct Packet_Guild_Cargo_Requst : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e

    // IDA_EA: 0x822BE52
    // IDA_NAME: _ZN25Packet_Guild_Cargo_RequstC2Ev
    Packet_Guild_Cargo_Requst();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Guild_Cargo_Requst) == 0x12u,
              "guild cargo request must retain its 18-byte ABI");
static_assert(offsetof(Packet_Guild_Cargo_Requst, charac_guildkey) == 0x0au &&
                  offsetof(Packet_Guild_Cargo_Requst, charac_no) == 0x0eu,
              "guild cargo request field offsets changed");

// CMD251 verifies a personal inventory item and hands the exact 0x70c
// request to the guild service; the service owns the later cargo mutation.
#pragma pack(push, 1)
struct Packet_Guild_Check_Guild_Cargo_Push_Item : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e
    int32_t item_count;        // +0x12
    int32_t item_id;           // +0x16
    int32_t add_info;          // +0x1a
    int32_t stackable_limit;   // +0x1e
    uint8_t is_stackable;      // +0x22
    uint8_t item_space;        // +0x23
    uint16_t slot;             // +0x24

    // IDA_EA: 0x822BEC2
    // IDA_NAME: _ZN40Packet_Guild_Check_Guild_Cargo_Push_ItemC2Ev
    Packet_Guild_Check_Guild_Cargo_Push_Item();
};

// This is the reciprocal 0x70d service request emitted only after the
// character-inventory deletion has succeeded.  Its embedded DnfItemInfo is
// kept at the original unaligned +0x1b wire offset.
struct Packet_Guild_Guild_Cargo_Push_Item : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e
    uint32_t item_count;       // +0x12
    uint32_t stackable_limit;  // +0x16
    uint8_t is_stackable;      // +0x1a
    DnfItemInfo item;          // +0x1b
    uint8_t item_space;        // +0x50
    uint16_t slot;             // +0x51

    // IDA_EA: 0x84E9434
    // IDA_NAME: _ZN34Packet_Guild_Guild_Cargo_Push_ItemC2Ev
    Packet_Guild_Guild_Cargo_Push_Item();
};

// CMD252 is the reciprocal service request.  Its one-byte item count is
// intentionally retained; it is not the local Inven_Item stack count.
struct Packet_Guild_Guild_Cargo_Pop_Item : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e
    int32_t cargo_slot;        // +0x12
    int32_t item_id;           // +0x16
    int32_t add_info;          // +0x1a
    uint8_t is_stackable;      // +0x1e
    int32_t item_count;        // +0x1f

    // IDA_EA: 0x822BF38
    // IDA_NAME: _ZN33Packet_Guild_Guild_Cargo_Pop_ItemC2Ev
    Packet_Guild_Guild_Cargo_Pop_Item();
};

// CMD253 changes only the guild-service cargo ordering.  The two client
// supplied item keys remain part of the service-side concurrency check.
struct Packet_Guild_Guild_Cargo_Move_Item : PacketHeader {
    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e
    int32_t from_slot;         // +0x12
    int32_t to_slot;           // +0x16
    int32_t from_item_key;     // +0x1a
    int32_t to_item_key;       // +0x1e

    // IDA_EA: 0x822BF9E
    // IDA_NAME: _ZN34Packet_Guild_Guild_Cargo_Move_ItemC2Ev
    Packet_Guild_Guild_Cargo_Move_Item();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Guild_Check_Guild_Cargo_Push_Item) == 0x26u &&
                  sizeof(Packet_Guild_Guild_Cargo_Push_Item) == 0x53u &&
                  sizeof(Packet_Guild_Guild_Cargo_Pop_Item) == 0x23u &&
                  sizeof(Packet_Guild_Guild_Cargo_Move_Item) == 0x22u,
              "guild cargo item service packets must retain their old ABI");
static_assert(offsetof(Packet_Guild_Check_Guild_Cargo_Push_Item, item_count) ==
                      0x12u &&
                  offsetof(Packet_Guild_Check_Guild_Cargo_Push_Item, slot) ==
                      0x24u &&
                  offsetof(Packet_Guild_Guild_Cargo_Push_Item, item) ==
                      0x1bu &&
                  offsetof(Packet_Guild_Guild_Cargo_Push_Item, slot) ==
                      0x51u &&
                  offsetof(Packet_Guild_Guild_Cargo_Pop_Item, cargo_slot) ==
                      0x12u &&
                  offsetof(Packet_Guild_Guild_Cargo_Pop_Item, item_count) ==
                      0x1fu &&
                  offsetof(Packet_Guild_Guild_Cargo_Move_Item, from_slot) ==
                      0x12u &&
                  offsetof(Packet_Guild_Guild_Cargo_Move_Item, to_item_key) ==
                      0x1eu,
              "guild cargo item service packet field offsets changed");

// These packed C2S records are not constructor-owning protocol packets; the
// dispatcher template creates them as local message storage before read().
#pragma pack(push, 1)
struct MSG_GUILD_CARGO_PUSH_ITEM : MSG_BASE {
    uint8_t item_space;  // +0x0d
    uint16_t slot;       // +0x0e
    int32_t item_id;     // +0x10
    int32_t add_info;    // +0x14
    uint16_t item_count; // +0x18
};

struct MSG_GUILD_CARGO_POP_ITEM : MSG_BASE {
    uint16_t cargo_slot; // +0x0d
    int32_t item_id;     // +0x0f
    int32_t add_info;    // +0x13
    uint8_t item_count;  // +0x17
};

struct MSG_GUILD_CARGO_MOVE_ITEM : MSG_BASE {
    uint16_t from_slot;     // +0x0d
    uint16_t to_slot;       // +0x0f
    int32_t from_item_key;  // +0x11
    int32_t to_item_key;    // +0x15
};
#pragma pack(pop)

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_GUILD_CARGO_PUSH_ITEM) == 0x1au &&
                  sizeof(MSG_GUILD_CARGO_POP_ITEM) == 0x18u &&
                  sizeof(MSG_GUILD_CARGO_MOVE_ITEM) == 0x19u,
              "guild cargo C2S messages must retain their old packed ABI");
static_assert(offsetof(MSG_GUILD_CARGO_PUSH_ITEM, item_space) == 0x0du &&
                  offsetof(MSG_GUILD_CARGO_PUSH_ITEM, item_count) == 0x18u &&
                  offsetof(MSG_GUILD_CARGO_POP_ITEM, cargo_slot) == 0x0du &&
                  offsetof(MSG_GUILD_CARGO_POP_ITEM, item_count) == 0x17u &&
                  offsetof(MSG_GUILD_CARGO_MOVE_ITEM, from_slot) == 0x0du &&
                  offsetof(MSG_GUILD_CARGO_MOVE_ITEM, to_item_key) == 0x15u,
              "guild cargo C2S message field offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// CMD266.  The original state-0 template merely gates the generic packet
// pipeline; the handler then requires exactly state 3, a selected character,
// a guild key, and an existing guild agit before sending the real service
// request.  Registration remains owned by the un-restored 606-slot root.
class Dispatcher_GuildCargoHistory
    : public IPacketDispatcher<MSG_BASE,
                               stGuildCargoHistoryParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596C78
    Dispatcher_GuildCargoHistory();
    // IDA_EA: 0x824585C / 0x824588C
    virtual ~Dispatcher_GuildCargoHistory();

    // IDA_EA: 0x81D0C32 / 0x81D0CB4 / 0x81D0DC6
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D0C3C.  This is a concrete helper, not a second dispatcher
    // interface slot.
    int check_error(CUser* user) const;
};

// CMD265 is the guild cargo open request.  Its gate and service ownership are
// the same as CMD266, but the negative-state LineFunc keeps the original
// account-id diagnostic member identification number.
class Dispatcher_GuildCargo
    : public IPacketDispatcher<MSG_BASE,
                               stGuildCargoParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596C5C
    Dispatcher_GuildCargo();
    // IDA_EA: 0x82458AA / 0x82458DA
    virtual ~Dispatcher_GuildCargo();

    // IDA_EA: 0x81D0A60 / 0x81D0AE2 / 0x81D0BFA
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D0A6A; this is not an additional virtual table slot.
    int check_error(CUser* user) const;
};

class Dispatcher_GuildCargoPushItem
    : public IPacketDispatcher<MSG_GUILD_CARGO_PUSH_ITEM,
                               stGuildCargoPushItemParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596C94
    Dispatcher_GuildCargoPushItem();
    // IDA_EA: 0x824580E / 0x824583E
    virtual ~Dispatcher_GuildCargoPushItem();

    // IDA_EA: 0x81D0DFE / 0x81D10D4 / 0x81D1316
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D0F64
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_GuildCargoPopItem
    : public IPacketDispatcher<MSG_GUILD_CARGO_POP_ITEM,
                               stGuildCargoPopItemParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596CB0
    Dispatcher_GuildCargoPopItem();
    // IDA_EA: 0x82457C0 / 0x82457F0
    virtual ~Dispatcher_GuildCargoPopItem();

    // IDA_EA: 0x81D1388 / 0x81D15B2 / 0x81D1748
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D14AA
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_GuildCargoMoveItem
    : public IPacketDispatcher<MSG_GUILD_CARGO_MOVE_ITEM,
                               stGuildCargoMoveItemParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596CCC
    Dispatcher_GuildCargoMoveItem();
    // IDA_EA: 0x8245772 / 0x82457A2
    virtual ~Dispatcher_GuildCargoMoveItem();

    // IDA_EA: 0x81D17BA / 0x81D1954 / 0x81D1A96
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D18DC
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GuildCargoHistory) == 0x04u,
              "CMD266 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(Dispatcher_GuildCargo) == 0x04u,
              "CMD265 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(Dispatcher_GuildCargoPushItem) == 0x04u &&
                  sizeof(Dispatcher_GuildCargoPopItem) == 0x04u &&
                  sizeof(Dispatcher_GuildCargoMoveItem) == 0x04u,
              "guild cargo item dispatchers must retain their old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_CARGO_HISTORY_DISPATCHER_H_
