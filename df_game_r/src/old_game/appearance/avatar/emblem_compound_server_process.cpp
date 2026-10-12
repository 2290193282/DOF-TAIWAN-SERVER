#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include <cstring>
#include <utility>
#include <vector>

#include "common/manager/arad_print_error.h"
#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/appearance/avatar/compound_emblem_statistic.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x81919E8
// IDA_NAME: _ZN20EmblemCompoundServer20GetEmblemRandomIndexEit
// DEMANGLED: EmblemCompoundServer::GetEmblemRandomIndex(int,unsigned short)
int EmblemCompoundServer::GetEmblemRandomIndex(int grade, uint16_t select_no) {
    // The old body intentionally uses map::operator[] rather than the
    // checked script accessor.  In particular, a malformed loaded script
    // creates the same null entry and follows the same dereference path as
    // the original server.
    ARAD::SCRIPT::EmblemCompound::Result* const result = script_.results[grade];
    const int list_index =
        result->result_grade <= get_rand_int(1000)
            ? static_cast<int>(result->item_count)
            : static_cast<int>(result->list_index);
    std::vector<unsigned long>& candidates = script_.emblem_lists[list_index];

    if (candidates.empty()) {
        return 0;
    }
    if (result->enabled) {
        return static_cast<int>(candidates[select_no]);
    }
    return static_cast<int>(
        candidates[get_rand_int(static_cast<int>(candidates.size()))]);
}

// IDA_EA: 0x8191AD6
// IDA_NAME: _ZN20EmblemCompoundServer18IsResultSelectableEi
// DEMANGLED: EmblemCompoundServer::IsResultSelectable(int)
int EmblemCompoundServer::IsResultSelectable(int grade) {
    // Keep the original map::operator[] lookup semantics; Process reaches
    // this only after the same grade selected a result above.
    return script_.results[grade]->enabled;
}

// IDA_EA: 0x8191AFA
// IDA_NAME: _ZN20EmblemCompoundServer7ProcessEP5CUserPN4arad21SigAradEmblemCompoundE
// DEMANGLED: EmblemCompoundServer::Process(CUser *,arad::SigAradEmblemCompound *)
bool EmblemCompoundServer::Process(
    CUser* user, arad::SigAradEmblemCompound* signal) {
    const int result_item_index = GetEmblemRandomIndex(
        static_cast<int>(signal->materials[0].grade), signal->select_no);
    if (result_item_index == 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 19);
        return false;
    }

    CItem* const result_item = G_CDataManager()->find_item(result_item_index);
    if (!result_item || result_item->vtable_->get_item_type(result_item) != 20) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 22);
        ARAD::PRINT_ERROR::find_item(
            const_cast<char*>("EMBLEM COMPOUND"), user, result_item_index);
        return false;
    }

    const int cost = script_.costs[signal->materials[0].grade];
    if (user->getCurCharacInvenW()->use_money(
            cost, static_cast<eMoneySubReason>(35), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 10);
        LogManager::logFormat(
            1,
            "localjapan/Arad_EmblemReformServer.cpp",
            "bool EmblemCompoundServer::Process(CUser*, arad::SigAradEmblemCompound*)",
            0x24e,
            "EMBLEM COMPOUND : [money:%d] [need:%d] [charac_no:%d]",
            user->getCurCharacInvenR()->get_money(),
            script_.costs[signal->materials[0].grade],
            user->getCurCharacNo());
        return false;
    }

    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), signal->materials[0].slot, 1,
            static_cast<eItemDelReason>(8), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 1);
        ARAD::PRINT_ERROR::delete_item(const_cast<char*>("EMBLEM COMPOUND"),
                                       user, signal->materials[0].slot);
        return false;
    }

    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), signal->materials[1].slot, 1,
            static_cast<eItemDelReason>(8), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN, 1);
        ARAD::PRINT_ERROR::delete_item(const_cast<char*>("EMBLEM COMPOUND"),
                                       user, signal->materials[1].slot);
        return false;
    }

    Inven_Item created_item;
    created_item.item_id_ = result_item->get_index();
    result_item->vtable_->make_item(result_item, created_item);
    created_item.set_add_info(1);

    const int inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        created_item, static_cast<eItemAddReason>(9), true, true);
    if (inserted_slot < 0) {
        const char* const title =
            g_scriptStringManager_.findString(4, "game_server_msg_122", 0);
        const char* const text =
            g_scriptStringManager_.findString(4, "game_server_msg_123", 0);
        const int server_group = user->GetServerGroup();
        const int text_length = static_cast<int>(std::strlen(text));
        const int charac_no = user->getCurCharacNo();
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            created_item,
            0,
            static_cast<unsigned int>(charac_no),
            text,
            text_length,
            0,
            static_cast<ENUM_SERVER_GROUP>(server_group),
            false,
            false);
    }

    signal->result_item_index = static_cast<uint32_t>(result_item_index);

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN);
    response.put_byte(1);
    response.put_int(user->getCurCharacInvenR()->get_money());
    response.put_short(signal->materials[0].slot);
    response.put_short(signal->materials[1].slot);
    response.put_byte(
        ARAD::Singleton<EmblemCompoundServer>::Get()->IsResultSelectable(
            static_cast<int>(signal->materials[0].grade)));
    response.put_int(signal->result_item_index);
    response.finalize(true);
    user->Send(response);

    if (inserted_slot >= 0) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), inserted_slot);
    }

    getCompoundEmblemStatistic()->increaseCount(signal->materials[0].grade);

    std::vector<std::pair<int, int> > created_items;
    const int created_count = created_item.add_info_;
    created_items.push_back(std::make_pair(
        static_cast<int>(created_item.item_id_), created_count));
    AvatarConvertServer::StatisticsEmblemCreate(created_items);
    return true;
}
