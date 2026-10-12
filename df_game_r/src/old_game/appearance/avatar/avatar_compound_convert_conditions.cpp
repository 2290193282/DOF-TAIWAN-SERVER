#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/manager/arad_print_error.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"


namespace {

typedef int (*AvatarFunctionSlot)(const CItem*, int);

bool IsDisjointEnabled(const CItem* item) {
    const AvatarFunctionSlot enabled = reinterpret_cast<AvatarFunctionSlot>(
        item->vtable_->slots_6_through_9[3]);
    return enabled != 0 && enabled(item, 2) == 1;
}

}  // namespace


// IDA_EA: 0x8191818
// IDA_NAME: _ZN20EmblemCompoundServer13GetEmblemInfoEP5CUserPN4arad21SigAradEmblemCompoundEi
// DEMANGLED: EmblemCompoundServer::GetEmblemInfo(CUser *,arad::SigAradEmblemCompound *,int)
bool EmblemCompoundServer::GetEmblemInfo(
    CUser* user, arad::SigAradEmblemCompound* signal, int material_index) {
    arad::SigAradEmblemCompound::materialInfo& material =
        signal->materials[material_index];
    CInventory* const inventory = user->getCurCharacInvenR();
    Inven_Item* const item_record = inventory->GetInvenRef(1, material.slot);
    if (!item_record || item_record->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0x16u);
        ARAD::PRINT_ERROR::isEmpty(const_cast<char*>("EMBLEM COMPOUND"),
                                   user, material.slot);
        return false;
    }
    if (user->CheckItemLock(1, material.slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0xD5u);
        ARAD::PRINT_ERROR::CheckItemLock(
            const_cast<char*>("EMBLEM COMPOUND"), user, material.slot);
        return false;
    }

    CItem* const prototype = G_CDataManager()->find_item(item_record->item_id_);
    if (!prototype || prototype->vtable_->get_item_type(prototype) != 20) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0x11u);
        ARAD::PRINT_ERROR::find_item(const_cast<char*>("EMBLEM COMPOUND"),
                                     user, item_record->item_id_);
        return false;
    }
    material.grade = static_cast<uint16_t>(prototype->get_grade());
    material.item_index = static_cast<uint32_t>(item_record->item_id_);
    return true;
}


// IDA_EA: 0x8191636
// IDA_NAME: _ZN20EmblemCompoundServer14CheckConditionEP5CUserPN4arad21SigAradEmblemCompoundE
// DEMANGLED: EmblemCompoundServer::CheckCondition(CUser *,arad::SigAradEmblemCompound *)
bool EmblemCompoundServer::CheckCondition(
    CUser* user, arad::SigAradEmblemCompound* signal) {
    if (!GetEmblemInfo(user, signal, 0) || !GetEmblemInfo(user, signal, 1)) {
        return false;
    }

    const uint16_t grade = signal->materials[0].grade;
    if (signal->materials[1].grade != grade || grade > 6u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0xD8u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool EmblemCompoundServer::CheckCondition(CUser*, arad::SigAradEmblemCompound*)",
            0x1E8,
            "EMBLEM COMPOUND : [grade1:%d] [grade2:%d] [charac_no:%d]",
            signal->materials[0].grade, signal->materials[1].grade,
            user->getCurCharacNo());
        return false;
    }

    const int required_money = script_.getCost(static_cast<int>(grade));
    const int available_money = user->getCurCharacInvenR()->get_money();
    if (available_money >= required_money) {
        return true;
    }

    user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 0x0Au);
    LogManager::logFormat(
        1,
        "localjapan/Arad_EmblemReformServer.cpp",
        "bool EmblemCompoundServer::CheckCondition(CUser*, arad::SigAradEmblemCompound*)",
        0x1F0,
        "EMBLEM COMPOUND : [money:%d] [need:%d] [charac_no:%d]",
        available_money, required_money, user->getCurCharacNo());
    return false;
}


// IDA_EA: 0x8192212
// IDA_NAME: _ZN19AvatarConvertServer14CheckConditionEP5CUserPN4arad20SigAradAvatarConvertE
// DEMANGLED: AvatarConvertServer::CheckCondition(CUser *,arad::SigAradAvatarConvert *)
bool AvatarConvertServer::CheckCondition(
    CUser* user, arad::SigAradAvatarConvert* signal) {
    CInventory* const inventory = user->getCurCharacInvenR();
    Inven_Item* const avatar = inventory->GetInvenRef(2, signal->avatar_slot);
    if (!avatar || avatar->isEmpty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x16u);
        ARAD::PRINT_ERROR::isEmpty(const_cast<char*>("AVATAR CONVERT"), user,
                                   signal->avatar_slot);
        return false;
    }
    if (static_cast<uint32_t>(avatar->item_id_) != signal->input_item_index) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x11u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2B5,
            "AVATAR CONVERT : [item_id1:%d] [item_id2:%d] [charac_no:%d]",
            avatar->item_id_, signal->input_item_index, user->getCurCharacNo());
        return false;
    }
    if (user->CheckItemLock(2, signal->avatar_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0xD5u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2BC,
            "AVATAR CONVERT : lock [item_id1:%d] [item_id2:%d] [charac_no:%d]",
            avatar->item_id_, signal->input_item_index, user->getCurCharacNo());
        return false;
    }

    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(signal->input_item_index));
    if (!prototype || prototype->vtable_->is_avatar_item(prototype) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x11u);
        ARAD::PRINT_ERROR::find_item(const_cast<char*>("AVATAR CONVERT"),
                                     user, signal->input_item_index);
        return false;
    }
    if (!IsDisjointEnabled(prototype)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x17u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2D2,
            "AVATAR CONVERT : isEnableAvatarFunc(AVATAR_FUNC_FILTER_DISJOINT) fail [item_id:%d] [charac_no:%d]",
            prototype->get_index(), user->getCurCharacNo());
        return false;
    }

    const WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrR();
    if (avatar_manager->GetExpireDate(avatar->add_info_) != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x17u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2D9,
            "AVATAR CONVERT : GetExpireDate() fail [ui_id:%d] [charac_no:%d]",
            avatar->add_info_, user->getCurCharacNo());
        return false;
    }

    ARAD::SCRIPT::AvatarConvert::ConvertResult* const result =
        script_.getConvertResult(prototype->get_grade());
    if (!result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x11u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2E2,
            "AVATAR CONVERT : getConvertResult() fail [item_id:%d] [charac_no:%d]",
            signal->input_item_index, user->getCurCharacNo());
        return false;
    }
    if (!result->is_select &&
        result->result_items.size() <= signal->select_no) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x17u);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool AvatarConvertServer::CheckCondition(CUser*, arad::SigAradAvatarConvert*)",
            0x2E9,
            "AVATAR CONVERT : [isSelect:%d] [selectNo:%d] [item_id:%d] [charac_no:%d]",
            result->is_select, signal->select_no, signal->input_item_index,
            user->getCurCharacNo());
        return false;
    }
    return true;
}
