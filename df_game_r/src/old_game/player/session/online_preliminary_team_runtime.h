#ifndef OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_TEAM_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_TEAM_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"

// The two packets are concrete online-preliminary DB-queue payloads.  Their
// fixed Stream/CStreamGuard reservation sizes are restored by A; C owns the
// original gameplay state and field population below.
struct SIG_ONLINE_PRELIMINARY_TEAM {
    int team_id;          // IDA +0x00
    char team_name[16];   // IDA +0x04
    float rating;         // IDA +0x14
    int win_count;        // IDA +0x18
    int lose_count;       // IDA +0x1c
};

struct SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY {
    int other_team_id;          // IDA +0x00
    unsigned int play_count;    // IDA +0x04
};

struct SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST {
    int team_id;    // IDA +0x000
    unsigned int count;  // IDA +0x004
    SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY entries[100];  // +0x008
};

static_assert(sizeof(SIG_ONLINE_PRELIMINARY_TEAM) == 0x20u,
              "online preliminary team payload ABI must stay 0x20");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_TEAM, rating) == 0x14u,
              "online preliminary team rating ABI offset must stay 0x14");
static_assert(sizeof(SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY) == 0x08u,
              "online preliminary team match-list entry ABI must stay 0x08");
static_assert(sizeof(SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST) == 0x328u,
              "online preliminary team match-list ABI must stay 0x328");

#endif  // OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_TEAM_RUNTIME_H_
