#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "common/config.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "common/manager/arad_print_error.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/avatar_aura/db_update_avatar_history.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kAvatarConvertContext[] = "AVATAR CONVERT";
const char kAvatarConvertFunction[] =
    "bool AvatarConvertServer::Convert(CUser*, arad::SigAradAvatarConvert*)";
const char kAvatarConvertSource[] = "localjapan/Arad_EmblemReformServer.cpp";

// The original sends the 24-byte all-zero buffer rooted at unk_8B9A913 both
// to avatar history and to insert_event_items.  Keep a full fixed-width
// buffer: DB_UpdateAvatarHistory serializes all 24 bytes, so a one-byte
// string literal would not preserve the observed request payload.
char kEmptyIpgAgencyNo[0x18] = {};

}  // namespace

// IDA_EA: 0x81926DE
// IDA_NAME: _ZN19AvatarConvertServer7ConvertEP5CUserPN4arad20SigAradAvatarConvertE
// DEMANGLED: AvatarConvertServer::Convert(CUser *,arad::SigAradAvatarConvert *)
bool AvatarConvertServer::Convert(CUser* user,
                                  arad::SigAradAvatarConvert* signal) {
    CItem* const source_item =
        G_CDataManager()->find_item(static_cast<int>(signal->input_item_index));
    if (source_item == 0 ||
        source_item->vtable_->is_avatar_item(source_item) != 1) {
        ARAD::PRINT_ERROR::find_item(
            const_cast<char*>(kAvatarConvertContext), user,
            static_cast<int>(signal->input_item_index));
        return false;
    }

    int source_grade = source_item->get_grade();
    if (source_item->vtable_->get_item_type(source_item) == 9) {
        source_grade = 3;
    }

    int selected_result = static_cast<int>(signal->select_no);
    ARAD::SCRIPT::AvatarConvert::ConvertResult* const convert_result =
        script_.getConvertResult(source_grade);
    if (convert_result->is_select != 1) {
        const int random =
            static_cast<int>(get_rand_int(convert_result->total_random_weight));
        selected_result = static_cast<int>(
            std::upper_bound(convert_result->cumulative_random_weights.begin(),
                             convert_result->cumulative_random_weights.end(),
                             random) -
            convert_result->cumulative_random_weights.begin());
    }

    const ARAD::SCRIPT::AvatarConvert::GetInfo& selected =
        convert_result->result_items[static_cast<size_t>(selected_result)];
    signal->result_item_index = static_cast<uint32_t>(selected.item_index);
    signal->result_count = static_cast<uint16_t>(selected.item_count);

    CItem* const result_item =
        G_CDataManager()->find_item(static_cast<int>(signal->result_item_index));
    if (result_item == 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x11u);
        ARAD::PRINT_ERROR::find_item(
            const_cast<char*>(kAvatarConvertContext), user,
            static_cast<int>(signal->input_item_index));
        return false;
    }

    Inven_Item prospective_item;
    prospective_item.item_id_ = static_cast<int>(signal->result_item_index);
    if (!result_item->is_stackable()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 0x11u);
        LogManager::logFormat(
            1, kAvatarConvertSource, kAvatarConvertFunction, 0x322,
            "AVATAR CONVERT : is_stackable() fail [item_id:%d] [charac_no:%d]",
            signal->result_item_index, user->getCurCharacNo());
        return false;
    }

    prospective_item.add_info_ =
        static_cast<int>(static_cast<int16_t>(signal->result_count));
    result_item->vtable_->make_item(result_item, prospective_item);
    if (user->getCurCharacInvenW()->tryInsertItemIntoInventory(
            prospective_item) < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 4);
        LogManager::logFormat(
            1, kAvatarConvertSource, kAvatarConvertFunction, 0x329,
            "AVATAR CONVERT : tryInsertItemIntoInventory() fail [item_id:%d] [charac_no:%d]",
            signal->result_item_index, user->getCurCharacNo());
        return false;
    }

    const int avatar_slot = static_cast<int>(signal->avatar_slot);
    const Inven_Item* const source_avatar =
        user->getCurCharacInvenR()->GetInvenRef(2, avatar_slot);
    const int avatar_ui_id = source_avatar->add_info_;
    CInventory* const inventory = user->getCurCharacInvenW();
    if (inventory->delete_item(static_cast<INVEN_TYPE>(2), avatar_slot, 1,
                               static_cast<eItemDelReason>(6), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 22);
        ARAD::PRINT_ERROR::delete_item(
            const_cast<char*>(kAvatarConvertContext), user,
            static_cast<unsigned short>(avatar_slot));
        return false;
    }

    if (inventory->GetAvatarItemMgrW()->UnRegistItem(avatar_ui_id) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_CONVERT_JPN, 22);
        LogManager::logFormat(
            1, kAvatarConvertSource, kAvatarConvertFunction, 0x33C,
            "AVATAR CONVERT : UnRegistItem() fail [ui_id:%d] [charac_no:%d]",
            avatar_ui_id, user->getCurCharacNo());
        return false;
    }

    inventory->SendAvatarEvent(
        54,
        avatar_ui_id,
        5,
        avatar_slot + oldsrv::config::kAvatarInventorySlotBase,
        0);
    DB_UpdateAvatarHistory::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()), avatar_ui_id,
        kEmptyIpgAgencyNo);

    std::vector<std::pair<int, int> > created_items;
    std::vector<int> inserted_slots;
    created_items.push_back(std::make_pair(
        static_cast<int>(signal->result_item_index),
        static_cast<int>(static_cast<int16_t>(signal->result_count))));
    const int inserted = inventory->insert_event_items(
        created_items, inserted_slots, static_cast<eMoneyAddReason>(0),
        static_cast<eItemAddReason>(10), false, kEmptyIpgAgencyNo,
        kEmptyIpgAgencyNo);

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_AVATAR_CONVERT_JPN);
    response.put_byte(1);
    response.put_short(avatar_slot);
    response.put_short(inserted);
    for (std::vector<int>::const_iterator value = inserted_slots.begin();
         value != inserted_slots.end();) {
        response.put_short(*value++);
        response.put_int(*value++);
        response.put_int(*value++);

        cMyTrace trace(kAvatarConvertFunction, 872, 0);
        trace(
            "AVATAR CONVERT : success [slot:%d] [selectNo:%d] [resultIdx:%d] [charac_no:%d]",
            avatar_slot, static_cast<int>(signal->select_no),
            static_cast<int>(signal->result_item_index), user->getCurCharacNo());
    }
    response.finalize(true);
    user->Send(response);

    const int result_is_recharge_stone =
        ARAD::Singleton<AvatarRechargeServer>::Get()
            ->script_.IsEmblemRechargeStone(
                static_cast<int>(signal->result_item_index));
    StatisticsAvatarDisjoint(source_item, result_is_recharge_stone);
    StatisticsEmblemCreate(created_items);
    return true;
}

// IDA_EA: 0x8193136
// IDA_NAME: _ZN19AvatarConvertServer24StatisticsAvatarDisjointEPK5CItemi
// DEMANGLED: AvatarConvertServer::StatisticsAvatarDisjoint(CItem const*,int)
int AvatarConvertServer::StatisticsAvatarDisjoint(
    const CItem* source_item, int is_emblem_recharge_stone) {
    Packet_Avater_Disjoint_Statistic packet;
    packet.action_0a = 1;
    packet.first_grade_bucket_0e = source_item->get_rarity() == 2
                                        ? 2
                                        : source_item->get_grade() - 1;
    packet.first_item_type_12 = source_item->vtable_->get_item_type(source_item);
    packet.first_succeeded_16 = 1;
    packet.second_grade_bucket_1b = is_emblem_recharge_stone;
    return GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}
