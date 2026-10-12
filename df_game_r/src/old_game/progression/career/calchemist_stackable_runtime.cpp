#include "old_game/progression/quest/quest_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/recipe_info.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <stdint.h>

namespace expert_job {

// IDA_EA: 0x85D1424
// IDA_NAME: _ZN10expert_job10CAlchemist19OnTeleportCharacterEP5CUseriiiib
// DEMANGLED: expert_job::CAlchemist::OnTeleportCharacter(
//     CUser *,int,int,int,int,bool)
int CAlchemist::OnTeleportCharacter(CUser* user,
                                    int village_index,
                                    int area_index,
                                    int position_x,
                                    int position_y,
                                    bool use_gate_center) {
    if (use_gate_center) {
        area_index = G_GameWorld()->get_gate_area(village_index);
        if (area_index < 0) {
            return 0;
        }
        if (!G_GameWorld()->get_area_center_point(
                village_index, area_index, position_x, position_y)) {
            cMyTrace trace(
                "static bool expert_job::CAlchemist::OnTeleportCharacter(CUser*, int, int, int, int, bool)",
                139,
                5);
            trace("get_area_center_point() call failed");
            return 0;
        }
    }

    if (user->getCurCharacVill() != village_index ||
        user->get_area(0) != area_index) {
        if (G_GameWorld()->move_area(user,
                                     village_index,
                                     area_index,
                                     position_x,
                                     position_y,
                                     0,
                                     false,
                                     0,
                                     0,
                                     0) != 1) {
            return 0;
        }
    } else {
        user->set_position(position_x, position_y, 0);
    }

    PacketGuard packet;
    packet.put_header(0, 138);
    packet.put_short(user->get_unique_id());
    packet.put_short(position_x);
    packet.put_short(position_y);
    packet.put_byte(0);
    packet.finalize(true);
    if (village_index != 0 || area_index != 0) {
        G_GameWorld()->send_to_area(village_index, area_index, packet);
    }
    return 1;
}

// IDA_EA: 0x85D1694
// IDA_NAME: _ZN10expert_job10CAlchemist14UseRevivalItemEP5CUser
// DEMANGLED: expert_job::CAlchemist::UseRevivalItem(CUser *)
int CAlchemist::UseRevivalItem(CUser* user) {
    const int revival_time = GlobalData::s_systemTime_.getCurSec() + 1200;
    user->SetRevivalTime(revival_time);
    return G_TimerQueue()->InsertTimer(
        0, user->GetUID(), 130, 1200, user->GetRevivalTime(), 0);
}

// IDA_EA: 0x85D18AE
// IDA_NAME: _ZN10expert_job10CAlchemist10UseDelilahEP5CUser
// DEMANGLED: expert_job::CAlchemist::UseDelilah(CUser *)
int CAlchemist::UseDelilah(CUser* user) {
    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }
    return static_cast<int>(reinterpret_cast<uintptr_t>(
        party->battle_field().SetAssignNPCByPotion(true)));
}

// IDA_EA: 0x85D18E2
// IDA_NAME: _ZN10expert_job10CAlchemist24CheckUsableStackableItemEP5CUserPK5CItem
// DEMANGLED: expert_job::CAlchemist::CheckUsableStackableItem(CUser *,CItem const *)
int CAlchemist::CheckUsableStackableItem(CUser* user,
                                         const CItem* item) {
    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(item);
    if (stackable->get_sub_type() != 2) {
        return 1;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 1;
    }

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member && member->GetUseAPCPotionTime() > current_second) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85D1712
// IDA_NAME: _ZN10expert_job10CAlchemist18OnUseStackableItemEP5CUsermPK5CItem
// DEMANGLED: expert_job::CAlchemist::OnUseStackableItem(CUser *,unsigned long,CItem const *)
int CAlchemist::OnUseStackableItem(CUser* user,
                                   unsigned long item_index,
                                   const CItem* item) {
    if (item->is_stackable()) {
        const CStackableItem* const stackable =
            static_cast<const CStackableItem*>(item);
        if (stackable->GetItemType() == 3 &&
            stackable->recipe_info().flag_25 != 0) {
            STExpertJobScript* const script =
                G_CDataManager()->GetExpertJobScript(
                    user->GetCurCharacExpertJobType());
            if (script) {
                const int expert_job_level = user->GetCurExpertJobLevel(
                    user->GetCurCharacExpertJobExp());
                CCharacExpertJob* const expert_job =
                    static_cast<CCharacExpertJob*>(user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
                if (!expert_job->learnRecipe(
                        item_index, script, expert_job_level)) {
                    return 13;
                }
                expert_job->SendExpertJobInfo(user);
            }
        }

        if (stackable->get_sub_type() == 2) {
            user->SetUseAPCPotionTime(
                GlobalData::s_systemTime_.getCurSec() +
                stackable->continuous_item_state_.cool_time_ms / 1000);
        }
    }

    switch (item_index) {
    case 2600020UL:
    case 2680783UL:
    case 2749063UL:
    case 2749165UL:
        UseRevivalItem(user);
        break;
    case 2600154UL:
        UseDelilah(user);
        break;
    default:
        break;
    }
    return 0;
}

}  // namespace expert_job
