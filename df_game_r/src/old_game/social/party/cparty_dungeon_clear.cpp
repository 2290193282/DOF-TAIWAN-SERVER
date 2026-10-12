#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/Login/inter_returns/statistic_manager_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "common/manager/user.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

extern int IsCollectDungeonStatistic()
    __asm__("_Z25IsCollectDungeonStatisticv");

// IDA_EA: 0x85B5518
// IDA_NAME: _ZN6CParty14OnDungeonClearEP5CUserb
// DEMANGLED: CParty::OnDungeonClear(CUser *,bool)
int CParty::OnDungeonClear(CUser* user, bool cleared) {
    const CDungeon* const dungeon = battle_field_.dungeon_;
    const bool dimension_party =
        battle_field_.instance_dungeon_mode_ == 1;

    if (static_cast<unsigned char>(IsCollectDungeonStatistic()) != 0u &&
        !dimension_party && user && user->getCurCharacR() &&
        user->character_statistic_.statistic_received_ && dungeon) {
        SendDungeonCharacInfo(user, cleared);
    }

    if (!dimension_party && dungeon) {
        const int charac_level = user->get_charac_level();
        const bool dimension_possible =
            static_cast<signed char>(reinterpret_cast<const CDungeon*>(dungeon)->get_dimension_possible()) >
            0;
        const int standard_level = dungeon->get_standard_level();
        if (dimension_possible ||
            (standard_level - 5 <= charac_level &&
             charac_level <= standard_level + 3)) {
            GlobalData::s_statisticMgr->dungeon_exp_.IncreaseDungeonExp(
                charac_level,
                static_cast<int>(user->character_.dungeon_exp_),
                static_cast<int>(user->character_.dungeon_bonus_exp_));
        }
    }

    user->onDungeonClear(cleared);
    user->setDungeonClearHackCheck(cleared);

    const int elapsed_seconds = GlobalData::system_time()->getCurSec() -
                                static_cast<int>(game_start_sec_);
    user->history_log().DungeonClearInfo(cleared ? 1 : 0,
                                         elapsed_seconds);
    battle_field_.onClearMap(false);

    if (user && dungeon && user->getCurCharacR() &&
        dungeon->blood_dungeon_type_ > 0u) {
        Packet_Blood_dungeon_statistic packet;
        const int charac_level = user->get_charac_level();
        std::memcpy(packet.raw_0a, &charac_level, sizeof(charac_level));
        packet.raw_0a[4] = cleared ? 1u : 0u;
        packet.raw_0a[5] = 1u;
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }

    if (battle_field_.village_monster_uid_ != 0u && !cleared) {
        const int now = GlobalData::system_time()->getCurSec();
        const int ghost_time = 30;
        user->SetCurCharacChaosDieTime(now);
        user->SetCurCharacChaosResponTime(now + ghost_time);
        pvp_assault::CAssaultMgr::NotifyGhostTime(
            user, ghost_time, ghost_time);
        user->SetCurCharacGhost(true);
        user->character_view().enableSaveCharacView();
    }
    return 1;
}

// IDA_EA: 0x85BF7DA
// IDA_NAME: _ZN6CParty20OnFailedDungeonClearEP5CUser
// DEMANGLED: CParty::OnFailedDungeonClear(CUser *)
int CParty::OnFailedDungeonClear(CUser* user) {
    return OnDungeonClear(user, false);
}

// IDA_EA: 0x85BF7FC
// IDA_NAME: _ZN6CParty23OnSuccessedDungeonClearEP5CUser
// DEMANGLED: CParty::OnSuccessedDungeonClear(CUser *)
int CParty::OnSuccessedDungeonClear(CUser* user) {
    return OnDungeonClear(user, true);
}
