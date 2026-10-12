#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/economy/inventory/inventory.h"

#include <cstring>
#include <set>


// IDA_EA: 0x850B0C2
// IDA_NAME: _ZN6CCargo5resetEv
// DEMANGLED: CCargo::reset(void)
void* CCargo::reset() {
    item_count_ = 0;
    owner_ = 0;
    capacity_ = 0;
    cargo_sort_flag_ = false;
    if (cargo_) {
        return std::memset(cargo_, 0, sizeof(Inven_Item) * 152u);
    }
    return cargo_;
}

// MIGRATED: WongWork::CDungeonClear::clear(void), IDA_EA 0x8695972.
// The sole member implementation is in old_game/dungeon_pvp/cdungeon_clear_to_string.cpp.

// IDA_EA: 0x85DFA64
// IDA_NAME: _ZN5CUser30set_guildwar_point_per_pvpplayEi
// DEMANGLED: CUser::set_guildwar_point_per_pvpplay(int)
CUser* CUser::set_guildwar_point_per_pvpplay(int value) {
    guild_pvp_point_ = value;
    return this;
}

// IDA_EA: 0x865CF6C
// IDA_NAME: _ZN5CUser32reset_pvp_masterid_walkingout_meEv
// DEMANGLED: CUser::reset_pvp_masterid_walkingout_me(void)
int CUser::reset_pvp_masterid_walkingout_me() {
    pvp_master_ids_.clear();
    return 0;
}

// IDA_EA: 0x865CF42
// IDA_NAME: _ZN5CUser33insert_pvp_masterid_walkingout_meEj
// DEMANGLED: CUser::insert_pvp_masterid_walkingout_me(unsigned int)
int CUser::insert_pvp_masterid_walkingout_me(unsigned int master_id) {
    return pvp_master_ids_.insert(master_id).second;
}

// IDA_EA: 0x868F6FA
// IDA_NAME: _ZN5CUser16setForceDropFlagEb
// DEMANGLED: CUser::setForceDropFlag(bool)
CUser* CUser::setForceDropFlag(bool enabled) {
    force_drop_flag_ = enabled;
    return this;
}


namespace WongWork {

// IDA_EA: 0x8695058
// IDA_NAME: _ZN8WongWork12CMCAPManager17resetExposedCountEv
// DEMANGLED: WongWork::CMCAPManager::resetExposedCount(void)
CMCAPManager* CMCAPManager::resetExposedCount() {
    exposed_count_ = 0;
    return this;
}

// IDA_EA: 0x8694FA0
// IDA_NAME: _ZN8WongWork12CMCAPManager5resetEv
// DEMANGLED: WongWork::CMCAPManager::reset(void)
CMCAPManager* CMCAPManager::reset() {
    point_ = 1u;
    increment_rate_ = 1000;
    snapshot_point_ = point_;
    raw_0c_ = 1u;
    exposed_count_ = 0u;
    save_clean_pad_ = 0u;
    return this;
}

}  // namespace WongWork
