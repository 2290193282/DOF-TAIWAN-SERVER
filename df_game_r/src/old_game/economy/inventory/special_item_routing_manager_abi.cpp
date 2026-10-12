#include "old_game/economy/inventory/special_item_routing_manager_abi.h"

#include <stdint.h>

#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistc_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

// processRouting @0x860AC6A passes the low byte of the 16-bit avatar selector
// at Inven_Item +0x0b to CUser::addAvatarItem.
char RoutingAvatarItemFlag(const Inven_Item& item) {
    return static_cast<char>(item.avatar_select_);
}

}  // namespace

// IDA_EA: 0x860A9C8
// IDA_NAME: _ZN26CSpecialItemRoutingManagerC2Ev
// DEMANGLED: CSpecialItemRoutingManager::CSpecialItemRoutingManager(void)
CSpecialItemRoutingManager::CSpecialItemRoutingManager() {}

// IDA_EA: 0x860A9CE
// IDA_NAME: _ZN26CSpecialItemRoutingManagerD2Ev
// DEMANGLED: CSpecialItemRoutingManager::~CSpecialItemRoutingManager()
CSpecialItemRoutingManager::~CSpecialItemRoutingManager() {}

// IDA_EA: 0x860A9D4
// IDA_NAME: _ZN26CSpecialItemRoutingManager12StartRoutingEP5CUserR10Inven_Itemj
// DEMANGLED: CSpecialItemRoutingManager::StartRouting(CUser *,Inven_Item &,unsigned int)
int CSpecialItemRoutingManager::StartRouting(CUser* user,
                                             Inven_Item& item,
                                             unsigned int unique_id) {
    const int state = user->get_state();
    if (state != 5 && state != 12) {
        return 1;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 1;
    }
    if (!G_CDataManager()->find_item(item.getKey())) {
        return 1;
    }
    if (party->IsRoutingState()) {
        return 18;
    }

    party->SetStartRouting();
    party->SetRoutingItem(item, unique_id);
    insertTimer(party);

    PacketGuard packet;
    packet.put_header(0, 281);
    packet.put_int(item.getKey());
    packet.finalize(true);
    party->send_to_party(packet);
    return 0;
}

// IDA_EA: 0x860B45C
// IDA_NAME: _ZN26CSpecialItemRoutingManager11insertTimerEP6CParty
// DEMANGLED: CSpecialItemRoutingManager::insertTimer(CParty *)
int CSpecialItemRoutingManager::insertTimer(CParty* party) {
    const int timer_key = party->gen_timer_key(
        static_cast<TIMER_MESSAGE>(43));
    return G_TimerQueue()->InsertTimer(1, party->GetPartyIndex(), 43, 35,
                                       static_cast<unsigned int>(timer_key),
                                       0);
}

// IDA_EA: 0x860AC6A
// IDA_NAME: _ZN26CSpecialItemRoutingManager14processRoutingEP6CParty
// DEMANGLED: CSpecialItemRoutingManager::processRouting(CParty *)
void CSpecialItemRoutingManager::processRouting(CParty* party) {
    Inven_Item* const routing_item = party->GetRoutingInvenItem();
    if (routing_item->getKey() == 0) {
        ProcessFailEtc(party, 0);
        return;
    }

    CItem* const item = G_CDataManager()->find_item(party->GetRoutingItemID());
    if (!item) {
        ProcessFailEtc(party, 0);
        return;
    }

    std::vector<std::pair<int, int> > dice_members;
    dice_members.clear();
    party->GetRoutingDiceMemberList(dice_members, item);

    CUser* user = 0;
    int selected_seat = 0;
    PacketGuard packet;
    packet.put_header(0, 282);

    if (dice_members.empty()) {
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_int(party->GetRoutingItemUniqueID());
    } else {
        if (dice_members.size() == 1u) {
            selected_seat = dice_members[0].first;
            user = party->get_user(selected_seat);
        } else {
            int best_roll = -1;
            for (std::vector<std::pair<int, int> >::const_iterator it =
                     dice_members.begin();
                 it != dice_members.end();
                 ++it) {
                const int roll = static_cast<int>(get_rand_int(100));
                if (roll > best_roll) {
                    best_roll = roll;
                    selected_seat = it->first;
                    user = party->get_user(selected_seat);
                }
            }
        }

        if (user) {
            int item_space = 0;
            if (item->is_stackable()) {
                if (item->vtable_->get_item_type(item) == 9) {
                    item_space = 7;
                }
            } else if (item->vtable_->is_creature_item(item)) {
                item_space = 7;
            } else {
                item_space = item->vtable_->is_avatar_item(item) ? 1 : 0;
            }

            int inserted = -1;
            if (item->vtable_->is_avatar_item(item)) {
                char ipg_no[24];
                WongWork::CGenUniqueNo::genIPGNo(
                    static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
                    user->getCurCharacNo(), ipg_no);
                inserted = user->addAvatarItem(
                    static_cast<unsigned long>(routing_item->getKey()),
                    0,
                    RoutingAvatarItemFlag(*routing_item),
                    ipg_no,
                    static_cast<eAvatarItemAddReason>(15));
            } else {
                inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
                    *routing_item,
                    static_cast<eItemAddReason>(4),
                    true,
                    true);
            }

            if (inserted == -1) {
                packet.put_byte(0);
                packet.put_byte(1);
                packet.put_int(party->GetRoutingItemUniqueID());
            } else {
                if (item->vtable_->is_random_option(item)) {
                    statistc_proxy::add(
                        user,
                        "log_random_option",
                        "pick_cnt",
                        1u,
                        "%d,'%s'",
                        user->get_charac_10_level_section(),
                        GlobalData::s_systemTime_.getCurDate());
                }

                const int routing_unique_id = party->GetRoutingItemUniqueID();
                int delete_result = 0;
                const int state = user->get_state();
                if (state == 5) {
                    delete_result = party->DeleteDungeonDropItem(
                        routing_unique_id);
                } else if (state == 12) {
                    WongWork::CBossTower* const boss_tower =
                        user->getBossTower();
                    if (boss_tower) {
                        boss_tower->map_info_.items_.erase(routing_unique_id);
                    }
                }

                if (delete_result > 0) {
                    LogManager::logFormat(
                        1,
                        "SpecialItemRoutingManager.cpp",
                        "void CSpecialItemRoutingManager::processRouting(CParty*)",
                        0x165,
                        "[EPIC ROUTING] DELETE DUNGEON ITEM FAIL");
                }

                packet.put_byte(1);
                packet.put_byte(static_cast<char>(selected_seat));
                packet.put_int(routing_unique_id);
                packet.put_byte(static_cast<char>(item_space));
                packet.put_short(static_cast<short>(inserted));
                user->SendUpdateItem(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(item_space),
                    inserted);
            }
        }
    }

    packet.finalize(true);
    party->send_to_party(packet);
    party->SetEndRouting();
}

// IDA_EA: 0x860B332
// IDA_NAME: _ZN26CSpecialItemRoutingManager14ProcessFailEtcEP6CPartyi
// DEMANGLED: CSpecialItemRoutingManager::ProcessFailEtc(CParty *,int)
void CSpecialItemRoutingManager::ProcessFailEtc(CParty* party, int reason) {
    if (party) {
        if (party->IsRoutingState() &&
            (static_cast<unsigned char>(
                 party->GetFirstValidMemberState()) == 5 ||
             static_cast<unsigned char>(
                 party->GetFirstValidMemberState()) == 12)) {
            PacketGuard packet;
            packet.put_header(0, 282);
            packet.put_byte(0);
            packet.put_byte(reason);
            packet.put_int(party->GetRoutingItemUniqueID());
            packet.finalize(true);
            party->send_to_party(packet);
            cancelTimer(party);
            party->SetEndRouting();
        }
    }
}

// IDA_EA: 0x860B4C2
// IDA_NAME: _ZN26CSpecialItemRoutingManager11cancelTimerEP6CParty
// DEMANGLED: CSpecialItemRoutingManager::cancelTimer(CParty *)
int CSpecialItemRoutingManager::cancelTimer(CParty* party) {
    return party->gen_timer_key(TIMER_MESSAGE_ROUTING_ITEM_WAIT);
}

// IDA_EA: 0x860AB2E
// IDA_NAME: _ZN26CSpecialItemRoutingManager21ArriveMemberSelectionEP5CUserP6CPartyc
// DEMANGLED: CSpecialItemRoutingManager::ArriveMemberSelection(CUser *,CParty *,char)
int CSpecialItemRoutingManager::ArriveMemberSelection(CUser* user,
                                                       CParty* party,
                                                       char selection) {
    if (static_cast<unsigned char>(party->GetMemberRoutingState(user)) != 0u) {
        return 0;
    }

    if (selection == 0) {
        party->SetMemberRoutingState(user, 1);
    } else if (selection == 1) {
        party->SetMemberRoutingState(user, 2);
    }

    if (static_cast<unsigned char>(party->HasAllMemberSelection()) != 0u) {
        cancelTimer(party);
        processRouting(party);
    }
    return 1;
}

// IDA_EA: 0x860B2C2
// IDA_NAME: _ZN26CSpecialItemRoutingManager13ProcessGiveUpEP6CParty
// DEMANGLED: CSpecialItemRoutingManager::ProcessGiveUp(CParty *)
void CSpecialItemRoutingManager::ProcessGiveUp(CParty* party) {
    if (party && party->IsRoutingState() && party->HasAllMemberSelection() &&
        (static_cast<unsigned char>(party->GetFirstValidMemberState()) == 5 ||
         static_cast<unsigned char>(party->GetFirstValidMemberState()) == 12)) {
        processRouting(party);
    }
}
