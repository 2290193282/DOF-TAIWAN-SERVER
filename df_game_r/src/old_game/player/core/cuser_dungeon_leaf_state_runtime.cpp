#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85BFDDA
// IDA_NAME: _ZN5CUser26resetDungeonClearHackCheckEv
// DEMANGLED: CUser::resetDungeonClearHackCheck(void)
CUser* CUser::resetDungeonClearHackCheck() {
    dungeon_clear_hack_check_ = 0;
    return this;
}

// IDA_EA: 0x85BFDEA
// IDA_NAME: _ZNK5CUser19IsUsedCoinInDungeonEv
// DEMANGLED: CUser::IsUsedCoinInDungeon(void)const
int CUser::IsUsedCoinInDungeon() const {
    return used_coin_in_dungeon_;
}

// IDA_EA: 0x85BFDFA
// IDA_NAME: _ZNK5CUser30IsDeleteEventDungeonInvitationEv
// DEMANGLED: CUser::IsDeleteEventDungeonInvitation(void)const
int CUser::IsDeleteEventDungeonInvitation() const {
    return delete_event_dungeon_invitation_;
}

// IDA_EA: 0x85BFE0A
// IDA_NAME: _ZN5CUser30getCheckPickUpRandomOptionItemEv
// DEMANGLED: CUser::getCheckPickUpRandomOptionItem(void)
int CUser::getCheckPickUpRandomOptionItem() {
    return check_pick_up_random_option_item_;
}

// IDA_EA: 0x85D1A0A
// IDA_NAME: _ZN5CUser19GetUseAPCPotionTimeEv
// DEMANGLED: CUser::GetUseAPCPotionTime(void)
int CUser::GetUseAPCPotionTime() {
    return static_cast<int>(use_apc_potion_time_);
}

// IDA_EA: 0x863BE54
// IDA_NAME: _ZNK5CUser13getPcRoomAuthEv
// DEMANGLED: CUser::getPcRoomAuth(void)const
int CUser::getPcRoomAuth() const {
    return pc_room_auth_;
}

// IDA_EA: 0x863BE64
// IDA_NAME: _ZN5CUser9IsPowerUpEv
// DEMANGLED: CUser::IsPowerUp(void)
int CUser::IsPowerUp() {
    return power_up_;
}
