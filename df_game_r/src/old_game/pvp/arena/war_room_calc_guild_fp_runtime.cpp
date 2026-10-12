#include "old_game/pvp/battlefield/war_room.h"

#include "old_game/player/core/cuser.h"

#include <stdint.h>

// IDA_EA: 0x86BBEA6
// IDA_NAME: _ZN7WarRoom11CalcGuildFPEv
// DEMANGLED: WarRoom::CalcGuildFP(void)
bool WarRoom::CalcGuildFP() {
    int guild_keys[6] = {0};
    uint8_t guild_checked[6] = {0};

    // First retain every present user's own FinishPoint total.  Guildless
    // users are intentionally marked complete: the original never pools
    // their points under guild key zero.
    for (int slot = 0; slot < 6; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (!user) {
            continue;
        }

        war_field_.guild_finish_points_[slot] = user->GetFinishPointTotal();
        guild_keys[slot] = user->get_charac_guildkey();
        if (!guild_keys[slot]) {
            guild_checked[slot] = 1;
        }
    }

    int slot = 0;
    bool result;
    for (;;) {
        result = slot <= 5;
        if (!result) {
            break;
        }

        if (war_field_.room_users_[slot] && !guild_checked[slot]) {
            int total_finish_point = war_field_.guild_finish_points_[slot];
            int member_count = 1;
            guild_checked[slot] = 1;

            uint8_t same_guild_slots[6] = {0};
            same_guild_slots[slot] = 1;
            for (int candidate = slot + 1; candidate <= 5; ++candidate) {
                if (war_field_.room_users_[candidate] &&
                    !guild_checked[candidate] &&
                    guild_keys[slot] == guild_keys[candidate]) {
                    total_finish_point +=
                        war_field_.guild_finish_points_[candidate];
                    ++member_count;
                    guild_checked[candidate] = 1;
                    same_guild_slots[candidate] = 1;
                }
            }

            // x86 idiv: signed, truncating toward zero.  A singleton keeps
            // its original total; only a real same-guild group is averaged.
            if (member_count > 1) {
                const int shared_finish_point =
                    total_finish_point / member_count;
                for (int target = 0; target <= 5; ++target) {
                    if (war_field_.room_users_[target] &&
                        same_guild_slots[target]) {
                        war_field_.guild_finish_points_[target] =
                            shared_finish_point;
                    }
                }
            }
        }

        ++slot;
    }
    return result;
}
