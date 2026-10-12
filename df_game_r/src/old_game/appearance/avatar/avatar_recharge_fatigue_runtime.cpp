#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/config.h"
#include "common/manager/arad_print_error.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"

#include <stdint.h>

#include <vector>


namespace {

// IDA_EA: 0x8190699 in the original Global unit.  The helper has local
// linkage there; retaining it local here preserves its five packed socket
// records without creating a cross-domain synthetic symbol.
int GetSocketCount(const stAvatarEmblemInfo_t& emblem) {
    int count = 0;
    while (count <= 4 && emblem.sockets[count].socket_type != 0u) {
        ++count;
    }
    return count;
}

}  // namespace


// IDA_EA: 0x81902E8
// IDA_NAME: _ZN20AvatarRechargeServer20CalcDurabilityDetailEP5CUseriRSt6vectorIiSaIiEE
// DEMANGLED: AvatarRechargeServer::CalcDurabilityDetail(CUser *,int,std::vector<int> &)
int AvatarRechargeServer::CalcDurabilityDetail(
    CUser* user,
    int slot,
    std::vector<int>& changed_slots) {
    CInventory* const inventory =
        static_cast<CInventory*>(user->getCurCharacInvenRefW());
    Inven_Item* const item =
        inventory->GetInvenRef(0, slot);
    if (item->isEmpty() || item->avatar_select_ == 0u) {
        return 0;
    }

    WongWork::CAvatarItemMgr* const avatar_manager =
        user->getCurCharacInvenW()->GetAvatarItemMgrW();
    const int reduce_rate =
        value_58_
            ? value_54_
            : avatar_manager->GetEmblemReduceRate(item->get_add_info());
    const uint16_t endurance = item->avatar_select_;
    if (reduce_rate > 0 && endurance != 0u &&
        static_cast<int>(get_rand_int(1000)) < reduce_rate) {
        const uint16_t next =
            static_cast<uint16_t>(endurance - 1u);
        item->avatar_select_ = next;
        avatar_manager->UpdateEndurance(item->get_add_info(), next);
        changed_slots.push_back(slot);
    }
    return 1;
}

// IDA_EA: 0x81906E2
// IDA_NAME: _ZN20AvatarRechargeServer25CheckConditionUseRechargeEP5CUserPN4arad24SigAradUseAvatarRechargeE
// DEMANGLED: AvatarRechargeServer::CheckConditionUseRecharge(CUser *,arad::SigAradUseAvatarRecharge *)
bool AvatarRechargeServer::CheckConditionUseRecharge(
    CUser* user, arad::SigAradUseAvatarRecharge* signal) {
    CInventory* const inventory = user->getCurCharacInvenR();
    Inven_Item* const recharge_stone = inventory->GetInvenRef(
        1, static_cast<int>(signal->recharge_stone_slot));
    if (!recharge_stone || recharge_stone->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 0x16u);
        ARAD::PRINT_ERROR::isEmpty(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            signal->recharge_stone_slot);
        return false;
    }

    if (!script_.IsEmblemRechargeStone(recharge_stone->item_id_)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 0x11u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarRechargeServer::CheckConditionUseRecharge(CUser*, arad::SigAradUseAvatarRecharge*)",
            0xAD,
            "USE AVATAR RECHANGE : [item_id:%d] isn't RechargeStone [charac_no:%d]",
            recharge_stone->item_id_, user->getCurCharacNo());
        return false;
    }

    if (user->CheckItemLock(1, signal->recharge_stone_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 0xD5u);
        ARAD::PRINT_ERROR::CheckItemLock(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            signal->recharge_stone_slot);
        return false;
    }

    Inven_Item* const avatar = inventory->GetInvenRef(
        2, static_cast<int>(signal->avatar_slot));
    if (!avatar || avatar->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 0x16u);
        ARAD::PRINT_ERROR::isEmpty(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            signal->avatar_slot);
        return false;
    }

    // The old body intentionally queries the lock map using inventory type
    // one for both supplied slots; retain that ABI-level policy exactly.
    if (user->CheckItemLock(1, signal->avatar_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 0xD5u);
        ARAD::PRINT_ERROR::CheckItemLock(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            signal->avatar_slot);
        return false;
    }

    const WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrR();
    const stAvatarEmblemInfo_t* const emblem =
        avatar_manager->getJewelSocketData(avatar->add_info_);
    signal->added_endurance =
        GetSocketCount(*emblem) * script_.getAvatarRecharageLimit();
    if (avatar->avatar_select_ < signal->added_endurance) {
        return true;
    }

    user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN, 0xD8u);
    LogManager::logFormat(
        1,
        "localjapan/Arad_EmblemReformServer.cpp",
        "bool AvatarRechargeServer::CheckConditionUseRecharge(CUser*, arad::SigAradUseAvatarRecharge*)",
        0xCD,
        "USE AVATAR RECHANGE : already max endurance [ui_id:%d] [charac_no:%d]",
        avatar->add_info_, user->getCurCharacNo());
    return false;
}

// IDA_EA: 0x8190A70
// IDA_NAME: _ZN20AvatarRechargeServer8RechargeEP5CUserPN4arad24SigAradUseAvatarRechargeE
// DEMANGLED: AvatarRechargeServer::Recharge(CUser *,arad::SigAradUseAvatarRecharge *)
bool AvatarRechargeServer::Recharge(
    CUser* user, arad::SigAradUseAvatarRecharge* signal) {
    const int recharge_stone_slot =
        static_cast<int>(signal->recharge_stone_slot);
    CInventory* const readable_inventory = user->getCurCharacInvenR();
    Inven_Item* const recharge_stone = readable_inventory->GetInvenRef(
        static_cast<INVEN_TYPE>(1), recharge_stone_slot);
    if (recharge_stone == 0 || recharge_stone->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 22);
        ARAD::PRINT_ERROR::isEmpty(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            static_cast<unsigned short>(recharge_stone_slot));
        return false;
    }

    // The old source indexes AvatarRecharge::emblem_recharge_stone_info with
    // operator[].  Do not turn this into find(): insertion of a missing key
    // is part of the original state transition before the endurance change.
    const int recharge_item_id = recharge_stone->item_id_;
    CInventory* const writable_inventory = user->getCurCharacInvenW();
    if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                        recharge_stone_slot,
                                        1,
                                        static_cast<eItemDelReason>(3),
                                        true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 19);
        ARAD::PRINT_ERROR::delete_item(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            static_cast<unsigned short>(recharge_stone_slot));
        return false;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             recharge_stone_slot);
    cMyTrace delete_trace(
        "bool AvatarRechargeServer::Recharge(CUser*, arad::SigAradUseAvatarRecharge*)",
        232,
        0);
    delete_trace(
        "USE AVATAR RECHANGE : delete_item() success. [slot:%d] [charac_no:%d]",
        recharge_stone_slot,
        user->getCurCharacNo());

    const int avatar_slot = static_cast<int>(signal->avatar_slot);
    Inven_Item* const avatar = writable_inventory->GetInvenRef(
        static_cast<INVEN_TYPE>(2), avatar_slot);
    if (avatar == 0 || avatar->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN,
                                 22);
        ARAD::PRINT_ERROR::isEmpty(
            const_cast<char*>("USE AVATAR RECHANGE"), user,
            static_cast<unsigned short>(avatar_slot));
        return false;
    }

    std::map<int, int>* const recharge_values =
        script_.getEmblemRechargeStoneInfo();
    const int prior_endurance = static_cast<int>(avatar->avatar_select_);
    int next_endurance = prior_endurance + (*recharge_values)[recharge_item_id];
    if (signal->endurance_limit < next_endurance) {
        next_endurance = signal->endurance_limit;
    }

    signal->added_endurance = next_endurance - prior_endurance;
    avatar->avatar_select_ = static_cast<uint16_t>(next_endurance);
    signal->current_endurance = static_cast<int>(avatar->avatar_select_);

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             avatar_slot);
    cMyTrace success_trace(
        "bool AvatarRechargeServer::Recharge(CUser*, arad::SigAradUseAvatarRecharge*)",
        254,
        0);
    success_trace(
        "USE AVATAR RECHARGE : success [slot:%d] [item_id:%d] [Endurance:%d] [charac_no:%d]",
        avatar_slot,
        avatar->item_id_,
        signal->current_endurance,
        user->getCurCharacNo());

    arad::SigAradAvatarEnduranceSave save;
    save.ui_id = static_cast<uint32_t>(avatar->add_info_);
    save.endurance = signal->current_endurance;
    SaveAvatarEnduranceUnit(save);
    return true;
}

// IDA_EA: 0x819049A
// IDA_NAME: _ZN20AvatarRechargeServer14CalcDurabilityEP5CUseri
// DEMANGLED: AvatarRechargeServer::CalcDurability(CUser *,int)
bool AvatarRechargeServer::CalcDurability(CUser* user, int fatigue) {
    for (int use_count = 0; use_count < fatigue; ++use_count) {
        std::vector<int> changed_slots;
        changed_slots.reserve(oldsrv::config::kAvatarEquipmentTypeCount);
        for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
             slot <= oldsrv::config::kAvatarEquipmentTypeLast;
             ++slot) {
            CalcDurabilityDetail(user, slot, changed_slots);
        }
        for (std::vector<int>::const_iterator it =
                 changed_slots.begin();
             it != changed_slots.end(); ++it) {
            UpdateAvatarEndurance(2, user,
                                  static_cast<short>(*it));
        }
    }
    // The original source result is caller-ignored; EAX is the terminating
    // loop comparison, which is false when the loop exits.
    return false;
}

// IDA_EA: 0x81913F6
// IDA_NAME: _ZN20AvatarRechargeServer21UpdateAvatarEnduranceEN5CUser11eSendTargetEPS0_s
// DEMANGLED: AvatarRechargeServer::UpdateAvatarEndurance(CUser::eSendTarget,CUser *,short)
void AvatarRechargeServer::UpdateAvatarEndurance(int target,
                                                 CUser* user,
                                                 short slot) {
    if (static_cast<uint16_t>(slot) > 9u) {
        return;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const item =
        inventory->GetInvenRef(0, static_cast<int>(slot));
    if (!item || item->isEmpty()) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_UPDATE_AVATAR_ENDURANCE);
    packet.put_short(static_cast<short>(user->get_unique_id()));
    packet.put_short(slot);
    packet.put_int(item->item_id_);
    packet.put_short(static_cast<short>(item->avatar_select_));
    packet.finalize(true);
    user->SendPacket(static_cast<CUser::eSendTarget>(target), packet);
}
