#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/global_data.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x830DC9D
// IDA_NAME: _Z19addUserHackCount149P5CUser
// DEMANGLED: addUserHackCount149(CUser *)
bool addUserHackCount149(CUser* user) {
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(0x198), 1u, 0u, 0u);
    return true;
}

// IDA_EA: 0x830DCE4
// IDA_NAME: _Z19addUserHackCount150P5CUser
// DEMANGLED: addUserHackCount150(CUser *)
bool addUserHackCount150(CUser* user) {
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(0x199), 1u, 0u, 0u);
    return true;
}

// IDA_EA: 0x830DD2C
// IDA_NAME: _ZN13CBattle_Field10onClearMapEb
// DEMANGLED: CBattle_Field::onClearMap(bool)
int CBattle_Field::onClearMap(bool cleared) {
    MapInfo* const map_info = GetCurrentMapInfo();
    const int tick = GlobalData::s_systemTime_.getCurTickCount();
    map_info->play_info_.end_tick = static_cast<uint32_t>(tick);

    if (cleared && get_dungeon_index() == 41) {
        unsigned int permitted_tick = 120000u;
        if (party_->get_member_count() > 1) {
            permitted_tick = 120001u;
        }

        if (party_->checkBossRoom() &&
            map_info->play_info_.getPlayTick() <=
                static_cast<int>(permitted_tick)) {
            party_->enumPartyMember(addUserHackCount149);
        }

        if (party_->checkBossRoom() && map_info->play_info_.field_08 == 0u &&
            map_info->play_info_.field_0c == 0u) {
            return party_->enumPartyMember(addUserHackCount150);
        }
    }

    return tick;
}
