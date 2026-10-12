#include "common/manager/user.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <map>
#include <stdint.h>


// IDA_EA: 0x86C4B9C
// IDA_NAME: _ZN9GameWorld15find_from_worldEt
// DEMANGLED: GameWorld::find_from_world(unsigned short)
CUser* GameWorld::find_from_world(unsigned short user_uid) {
    GameWorldUserMap::iterator it = users_in_world_.find(user_uid);
    if (it == users_in_world_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x86CF59C
// IDA_NAME: _ZN9GameWorld15InsertLoginUserEP5CUser
// DEMANGLED: GameWorld::InsertLoginUser(CUser *)
int GameWorld::InsertLoginUser(CUser* user) {
    return login_users_.insert(std::make_pair(
                            static_cast<unsigned int>(user->get_acc_id()),
                            user))
        .second;
}

// IDA_EA: 0x86CEBE0
// IDA_NAME: _ZN9GameWorld16GetWorldMapIndexEP5CUser
// DEMANGLED: GameWorld::GetWorldMapIndex(CUser *)
int GameWorld::GetWorldMapIndex(CUser* user) {
    GameWorld* const world = this;
    if (!user) {
        return -1;
    }
    const int village =
        static_cast<signed char>(user->getCurCharacVill());
    const int area = user->get_area(false);
    if (!world->check_valid_area(village, area)) {
        return -1;
    }
    Village* user_village = world->getUserVillage(user);
    if (!user_village) {
        return -1;
    }
    Area* area_obj = user_village->getArea(area);
    if (!area_obj) {
        return -1;
    }
    return area_obj->get_world_map_index();
}

// IDA_EA: 0x86CAD68
// IDA_NAME: _ZN9GameWorld13get_user_infoEP5CUserii
// DEMANGLED: GameWorld::get_user_info(CUser *,int,int)
void GameWorld::get_user_info(CUser* requester,
                              int target_uid,
                              int detail_type) {
    GameWorld* const world = this;
    PacketGuard packet;
    if (detail_type == 2) {
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(2);
        requester->make_basic_info(packet, static_cast<char>(2));
        packet.finalize(1);
        requester->Send(packet);
        return;
    }

    if (target_uid == 0xFFFF) {
        return;
    }

    CUser* target = find_from_world(static_cast<unsigned short>(target_uid));
    if (!target) {
        LogManager::logFormat(1,
                              "world.cpp",
                              "void GameWorld::get_user_info(CUser*, int, int)",
                              0x107E,
                              "[GameWorld::get_user_info] can't find user in world");
        return;
    }

    if (detail_type == 3) {
        const bool requester_gm = GlobalData::s_pGMAccounts_ &&
            GlobalData::s_pGMAccounts_->isGM(requester->get_acc_id());
        const bool target_gm = GlobalData::s_pGMAccounts_ &&
            GlobalData::s_pGMAccounts_->isGM(target->get_acc_id());
        if (!requester_gm && target_gm) {
            return;
        }
        reinterpret_cast<CTitleBook*>(target->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)))
            ->sendListOtherUser(
                requester, static_cast<ENUM_TITLE_BOOK_INFO_TYPE>(2));
    }

    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(static_cast<unsigned char>(detail_type));
    packet.put_short(1);
    target->make_basic_info(packet, static_cast<char>(detail_type));
    packet.finalize(1);
    requester->Send(packet);
}
