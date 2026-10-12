#include "old_game/player/core/cuser.h"

#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x868B20C
// IDA_NAME: _ZN5CUser9rewardExpEiRiS0_S0_S0_13eExpAddReasonb
// DEMANGLED: CUser::rewardExp(int,int &,int &,int &,int &,eExpAddReason,bool)
void CUser::rewardExp(int gained_exp,
                      int& gained_sp,
                      int& gained_sfp,
                      int& previous_sp,
                      int& current_sp,
                      eExpAddReason reason,
                      bool notify) {
    PacketGuard packet;
    const ENUM_SKILL_TREE_KIND tree =
        static_cast<ENUM_SKILL_TREE_KIND>(GetCurCharacSkillTreeIndex());
    SkillSlot* const skill_slot = getCurCharacSkillR();
    previous_sp = skill_slot->get_remain_sp_at_index(tree);

    calcHelpAbuseRatio(gained_exp, false);
    const bool level_changed =
        gain_exp_sp(gained_exp, gained_sp, gained_sfp, reason, 0, notify);

    current_sp = skill_slot->get_remain_sp_at_index(tree);
    if (!level_changed) {
        return;
    }

    if (get_state() == 3) {
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(1);
        make_basic_info(packet, 0);
        packet.finalize(true);
        G_GameWorld()->send_all(packet, this);
    } else if (get_state() == 5 || get_state() == 8) {
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(1);
        packet.put_short(1);
        make_basic_info(packet, 1);
        packet.finalize(true);
        CParty* const party = GetParty();
        if (party) {
            party->send_to_party(packet);
        }
    }

    if (notify) {
        decide_growth_power_reward_system();
    }
}
