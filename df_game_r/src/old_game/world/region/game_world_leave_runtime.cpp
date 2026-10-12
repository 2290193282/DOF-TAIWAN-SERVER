#include <map>
#include <stdint.h>

#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"

extern const char* NumberToString(unsigned int value, int slot)
    __asm__("_Z14NumberToStringji");


// IDA_EA: 0x86C5288
// IDA_NAME: _ZN9GameWorld16leave_game_worldEP5CUser
// DEMANGLED: GameWorld::leave_game_world(CUser *)
int GameWorld::leave_game_world(CUser* user) {
    GameWorld* const world = this;
    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    if (static_cast<unsigned char>(
            GlobalData::s_GM_Manager->GetCurrentGmMode(account_id))) {
        user->SetGameMasterMode(false);
        GlobalData::s_GM_Manager->TurnGmMode(
            static_cast<unsigned int>(user->get_acc_id()));
    }

    GameWorldUserMap& users = users_in_world_;
    const uint16_t unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    GameWorldUserMap::iterator user_it = users.find(unique_id);
    if (user_it == users.end()) {
        LogManager::logFormat(1,
                              "world.cpp",
                              "bool GameWorld::leave_game_world(CUser*)",
                              0x56A,
                              "m_UsersInWorld.find(%d)",
                              unique_id);
        return 0;
    }

    users.erase(unique_id);

    const int area = user->get_area(false);
    const int village = user->getCurCharacVill();
    if (world->check_valid_area(village, area)) {
        Village* user_village = world->getUserVillage(user);
        if (user_village) {
            Area* user_area = user_village->getArea(user->get_area(false));
            if (user_area) {
                user_area->erase_user(user);
            }
        }
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 6);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);
    world->send_all(packet);

    GameWorldSessionMap& sessions = sessions_by_account_;
    const uint32_t session_key =
        static_cast<uint32_t>(user->get_acc_id());
    GameWorldSessionMap::iterator session_it = sessions.find(session_key);
    if (session_it == sessions.end()) {
        LogManager::logFormat(1,
                              "world.cpp",
                              "bool GameWorld::leave_game_world(CUser*)",
                              0x588,
                              "session_list.find(%s)",
                              NumberToString(
                                  static_cast<unsigned int>(
                                      user->get_acc_id()),
                                  0));
        return 0;
    }

    sessions.erase(session_key);
    g_schoolMgr.DelUser(user);

    const unsigned int guild_key =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (guild_key) {
        g_guildAgitMgr.ReleaseGuildAgitArea(guild_key);
    }

    if (static_cast<unsigned char>(user->isJoinPowerWar())) {
        GlobalData::s_power_manager->DecreasePowerUserCount(
            static_cast<char>(user->getPowerSide()));
    }

    if (static_cast<unsigned char>(user->isHangameUser())) {
        --hangame_user_count_;
    } else {
        --other_user_count_;
    }
    return 1;
}
