#ifndef OLDSRV_OLD_GAME_GROUP_PARTY_MESSAGES_H_
#define OLDSRV_OLD_GAME_GROUP_PARTY_MESSAGES_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/message_base.h"

// IDA unit: 09_dungeon_pvp_party/DisPatcher_SetPartyInfo.
// The dispatcher template reserves this exact packed extent without running
// MSG_BASE::C2.
#pragma pack(push, 1)
struct MSG_PARTY_SET_INFO : public MSG_BASE {
    uint8_t title_index;
    int title_length;
    char title[32];
    uint8_t user_max;
    uint16_t dungeon_index;
    uint8_t dungeon_difficulty;
};
#pragma pack(pop)

#if defined(__i386__)
// MSG_BASE carries a non-standard-layout inherited C++ ABI. GCC 4.9/i386
// still evaluates these packed offsets correctly; suppress only that warning.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_PARTY_SET_INFO, title_index) == 0x0du,
              "MSG_PARTY_SET_INFO title index must stay at +0x0d");
static_assert(offsetof(MSG_PARTY_SET_INFO, title_length) == 0x0eu,
              "MSG_PARTY_SET_INFO title length must stay at +0x0e");
static_assert(offsetof(MSG_PARTY_SET_INFO, title) == 0x12u,
              "MSG_PARTY_SET_INFO title must stay at +0x12");
static_assert(offsetof(MSG_PARTY_SET_INFO, user_max) == 0x32u,
              "MSG_PARTY_SET_INFO user max must stay at +0x32");
static_assert(offsetof(MSG_PARTY_SET_INFO, dungeon_index) == 0x33u,
              "MSG_PARTY_SET_INFO dungeon index must stay at +0x33");
static_assert(offsetof(MSG_PARTY_SET_INFO, dungeon_difficulty) == 0x35u,
              "MSG_PARTY_SET_INFO difficulty must stay at +0x35");
static_assert(sizeof(MSG_PARTY_SET_INFO) == 0x36u,
              "MSG_PARTY_SET_INFO raw stack extent must stay 0x36");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PARTY_MESSAGES_H_
