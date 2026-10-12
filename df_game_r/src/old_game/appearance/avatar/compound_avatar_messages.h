#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_MESSAGES_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_MESSAGES_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/message_base.h"

// IDA_EA: Dispatcher_Compound_Avatar::dispatch_sig @0x81EAD40 and
// WongWork::CCompoundAvatar::_MakeCompoundInfo @0x8331EFE.
//
// This direct dispatcher reads the nine payload fields in order.  C2S header
// state remains the MSG_BASE prefix (+0x00..+0x0c); the request body starts at
// its proven +0x0d boundary.  The layout has no tail storage.
#pragma pack(push, 1)
struct MSG_COMPOUND_AVATAR : public MSG_BASE {
    uint16_t recipe_slot;             // IDA +0x0d
    uint16_t first_avatar_slot;       // IDA +0x0f
    uint32_t first_avatar_item_id;    // IDA +0x11
    uint16_t second_avatar_slot;      // IDA +0x15
    uint32_t second_avatar_item_id;   // IDA +0x17
    uint32_t requested_result_item;   // IDA +0x1b
    uint16_t requested_avatar_option; // IDA +0x1f
    uint8_t upper_compound;           // IDA +0x21
    uint8_t compound_kind;            // IDA +0x22
};
#pragma pack(pop)

#if defined(__i386__)
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woffsetof"
#endif
static_assert(offsetof(MSG_COMPOUND_AVATAR, recipe_slot) == 0x0du,
              "compound-avatar recipe slot must stay at IDA +0x0d");
static_assert(offsetof(MSG_COMPOUND_AVATAR, first_avatar_slot) == 0x0fu,
              "compound-avatar first slot must stay at IDA +0x0f");
static_assert(offsetof(MSG_COMPOUND_AVATAR, first_avatar_item_id) == 0x11u,
              "compound-avatar first item must stay at IDA +0x11");
static_assert(offsetof(MSG_COMPOUND_AVATAR, second_avatar_slot) == 0x15u,
              "compound-avatar second slot must stay at IDA +0x15");
static_assert(offsetof(MSG_COMPOUND_AVATAR, second_avatar_item_id) == 0x17u,
              "compound-avatar second item must stay at IDA +0x17");
static_assert(offsetof(MSG_COMPOUND_AVATAR, requested_result_item) == 0x1bu,
              "compound-avatar requested item must stay at IDA +0x1b");
static_assert(offsetof(MSG_COMPOUND_AVATAR, requested_avatar_option) == 0x1fu,
              "compound-avatar option must stay at IDA +0x1f");
static_assert(offsetof(MSG_COMPOUND_AVATAR, upper_compound) == 0x21u,
              "compound-avatar upper flag must stay at IDA +0x21");
static_assert(offsetof(MSG_COMPOUND_AVATAR, compound_kind) == 0x22u,
              "compound-avatar kind must stay at IDA +0x22");
static_assert(sizeof(MSG_COMPOUND_AVATAR) == 0x23u,
              "compound-avatar request must stay at IDA size 0x23");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_MESSAGES_H_
