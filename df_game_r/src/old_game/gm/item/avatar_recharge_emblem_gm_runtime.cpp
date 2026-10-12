#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/config.h"
#include "common/manager/arad_print_error.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

namespace {

// IDA_EA: 0x8190699
// IDA_NAME: _ZL14getSocketCountPK20stAvatarEmblemInfo_t
int GetSocketCount(const stAvatarEmblemInfo_t* emblem) {
    int count = 0;
    while (count <= 4 && emblem->sockets[count].socket_type != 0u) {
        ++count;
    }
    return count;
}

// IDA_EA: 0x8197D00
// IDA_NAME: _ZN4ARAD19notifySystemMessageEP5CUserPc
// The global helper only builds a chat-notification packet and sends it to
// this one user.  Keep that behavior local while the global ARAD helper has
// no production provider.
int NotifySystemMessage(CUser* user, const char* text) {
    if (!user || !text) {
        return 0;
    }

    PacketGuard packet;
    G_GameWorld()->make_packet_chat_msg(
        packet,
        user,
        const_cast<char*>(text),
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_NOTIPACKET>(12));
    return user->Send(packet);
}

}  // namespace

// IDA_EA: 0x8190ED8
// IDA_NAME: _ZN20AvatarRechargeServer9UseEmblemEP5CUserRSt6vectorIiSaIiEEP10Inven_Items
// DEMANGLED: AvatarRechargeServer::UseEmblem(CUser *,std::vector<int,std::allocator<int>> &,Inven_Item *,short)
bool AvatarRechargeServer::UseEmblem(CUser* user,
                                     std::vector<int>& emblem_grades,
                                     Inven_Item* emblem_item,
                                     short avatar_slot) {
    const CInventory* const readable_inventory = user->getCurCharacInvenR();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        readable_inventory->GetAvatarItemMgrR();
    const stAvatarEmblemInfo_t* const socket_data =
        avatar_manager->getJewelSocketData(emblem_item->add_info_);
    const int endurance_limit =
        GetSocketCount(socket_data) * script_.getAvatarRecharageLimit();

    int added_endurance = 0;
    for (std::vector<int>::const_iterator it = emblem_grades.begin();
         it != emblem_grades.end(); ++it) {
        const std::map<int, int>::const_iterator recharge =
            script_.recharge_use_emblem_info.find(*it);
        if (recharge != script_.recharge_use_emblem_info.end()) {
            added_endurance += recharge->second;
        }
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    Inven_Item* const avatar = writable_inventory->GetInvenRef(
        2, static_cast<int>(avatar_slot));
    if (!avatar || avatar->isEmpty()) {
        ARAD::PRINT_ERROR::isEmpty(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            static_cast<unsigned short>(avatar_slot));
        return false;
    }

    int next_endurance = static_cast<int>(avatar->avatar_select_) +
                         added_endurance;
    if (next_endurance > endurance_limit) {
        next_endurance = endurance_limit;
    }
    avatar->avatar_select_ = static_cast<uint16_t>(next_endurance);

    UpdateAvatarEndurance(1, user, avatar_slot);

    cMyTrace success_trace(
        "bool AvatarRechargeServer::UseEmblem(CUser*, std::vector<int, "
        "std::allocator<int> >&, Inven_Item*, short int)",
        322,
        0);
    success_trace(
        "USE EMBLEM + RECHARGE : success [slot:%d] [item_id:%d] "
        "[Endurance:%d] [charac_no:%d]",
        avatar_slot,
        avatar->item_id_,
        emblem_item->avatar_select_,
        user->getCurCharacNo());

    arad::SigAradAvatarEnduranceSave save;
    save.ui_id = static_cast<uint32_t>(avatar->add_info_);
    save.endurance = static_cast<int>(avatar->avatar_select_);
    SaveAvatarEnduranceUnit(save);
    return true;
}

// IDA_EA: 0x81911B6
// IDA_NAME: _ZN20AvatarRechargeServer17GmReduceEnduranceEP5CUserss
// DEMANGLED: AvatarRechargeServer::GmReduceEndurance(CUser *,short,short)
int AvatarRechargeServer::GmReduceEndurance(CUser* user,
                                            short avatar_slot,
                                            short endurance_delta) {
    if (avatar_slot < oldsrv::config::kAvatarEquipmentTypeFirst ||
        avatar_slot > oldsrv::config::kAvatarEquipmentTypeLast) {
        return endurance_delta;
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    Inven_Item* const equipment = writable_inventory->GetInvenRef(
        0, static_cast<int>(avatar_slot));
    if (!equipment || equipment->isEmpty()) {
        return 1;
    }

    const WongWork::CAvatarItemMgr* const avatar_manager =
        user->getCurCharacInvenR()->GetAvatarItemMgrR();
    const stAvatarEmblemInfo_t* const socket_data =
        avatar_manager->getJewelSocketData(equipment->add_info_);
    AvatarRechargeServer* const recharge_server =
        ARAD::Singleton<AvatarRechargeServer>::Get();
    const int endurance_limit =
        GetSocketCount(socket_data) *
        recharge_server->script_.getAvatarRecharageLimit();

    int next_endurance = static_cast<int>(equipment->avatar_select_) +
                         static_cast<int>(endurance_delta);
    if (next_endurance < 0) {
        next_endurance = 0;
    } else if (next_endurance > endurance_limit) {
        next_endurance = endurance_limit;
    }
    equipment->avatar_select_ = static_cast<uint16_t>(next_endurance);

    recharge_server->UpdateAvatarEndurance(1, user, avatar_slot);
    SaveAvatarEnduranceUnit(equipment->add_info_,
                            static_cast<int>(equipment->avatar_select_));
    return NotifySystemMessage(user, "succ avatar Endurance change");
}

// IDA_EA: 0x8191318
// IDA_NAME: _ZN20AvatarRechargeServer27GmChangeEnduranceReduceRateEP5CUserbi
// DEMANGLED: AvatarRechargeServer::GmChangeEnduranceReduceRate(CUser *,bool,int)
int AvatarRechargeServer::GmChangeEnduranceReduceRate(CUser* user,
                                                       bool enabled,
                                                       int percentage) {
    const int denied = user->isGMUser() ^ 1;
    if (denied) {
        return denied;
    }

    return ARAD::Singleton<AvatarRechargeServer>::Get()
        ->GmSetEnduranceReduceRate(user, enabled, percentage);
}

// IDA_EA: 0x8191364
// IDA_NAME: _ZN20AvatarRechargeServer24GmSetEnduranceReduceRateEP5CUserbi
// DEMANGLED: AvatarRechargeServer::GmSetEnduranceReduceRate(CUser *,bool,int)
int AvatarRechargeServer::GmSetEnduranceReduceRate(CUser* user,
                                                    bool enabled,
                                                    int percentage) {
    value_58_ = enabled;
    if (enabled) {
        if (percentage < 0) {
            percentage = 0;
        } else if (percentage > 100) {
            percentage = 1000;
        } else {
            percentage *= 10;
        }
        value_54_ = percentage;
        return NotifySystemMessage(
            user,
            "succ avatar Endurance reduce rate mode on! change rate");
    }

    value_54_ = 0;
    return NotifySystemMessage(user, "avatar Endurance reduce rate mode off");
}
