#include "old_game/appearance/avatar/packet_dispatcher_compound_emblem.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstring>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "old_data/db/core/db_types.h"
#include "old_game/appearance/avatar/compound_emblem_statistic.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_CompoundEmblem::dispatch_sig(CUser*, PacketBuf&)";

void ThrowCompoundEmblemError(int error) {
    // The direct IDA body raises ENUM_ERROR; packet-dispatch owns its
    // conversion to the CMD260 error reply.
    throw static_cast<ENUM_ERROR>(error);
}

void SendOverflowMail(CUser* user, const Inven_Item& item) {
    const char* const title =
        g_scriptStringManager_.findString(4, "game_server_msg_122", 0);
    const char* const body =
        g_scriptStringManager_.findString(4, "game_server_msg_123", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        body,
        static_cast<int>(std::strlen(body)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

}  // namespace

// IDA_EA: 0x8220A60
// IDA_NAME: _ZNK25Dispatcher_CompoundEmblem21_selectJewelTypeIndexEjPKt
// DEMANGLED: Dispatcher_CompoundEmblem::_selectJewelTypeIndex(unsigned int,unsigned short const*)const
int Dispatcher_CompoundEmblem::_selectJewelTypeIndex(
    unsigned int count,
    const uint16_t* types) const {
    int type_counts[5] = {0, 0, 0, 0, 0};
    int cumulative[5] = {0, 0, 0, 0, 0};
    int total = 0;
    int cumulative_total = 0;

    for (unsigned int index = 0; index < count; ++index) {
        if (types[index] != 0) {
            getAvatarEmblemTypeIdxForCompountEmblem(
                types[index], type_counts, total);
        }
    }
    if (total == 0) {
        return -1;
    }

    for (int type = 0; type <= 4; ++type) {
        const int weight = 10000 / total * type_counts[type];
        cumulative[type] = weight + cumulative_total;
        cumulative_total += weight;
    }

    const int random = static_cast<int>(get_rand_int(cumulative_total));
    for (int type = 0; type <= 4; ++type) {
        if (cumulative[type] > random) {
            return type;
        }
    }
    return -1;
}

// IDA_EA: 0x8220BB2
// IDA_NAME: _ZN25Dispatcher_CompoundEmblem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompoundEmblem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompoundEmblem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    unsigned long first_item_id = 0;
    uint16_t first_slot = 0;
    unsigned long second_item_id = 0;
    uint16_t second_slot = 0;
    if (packet.get_item_idx(first_item_id) != 1) {
        return LineFunc(55951, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(first_slot) != 1) {
        return LineFunc(55952, kDispatchSignature, 0, 0);
    }
    if (packet.get_item_idx(second_item_id) != 1) {
        return LineFunc(55953, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(second_slot) != 1) {
        return LineFunc(55954, kDispatchSignature, 0, 0);
    }

    const int socket_action_error = user->isEnableAvatarSocketAction();
    if (socket_action_error != 0) {
        ThrowCompoundEmblemError(socket_action_error);
    }

    CItem* const first_item = G_CDataManager()->find_item(
        static_cast<int>(first_item_id));
    CItem* const second_item = G_CDataManager()->find_item(
        static_cast<int>(second_item_id));
    if (!first_item || !second_item) {
        ThrowCompoundEmblemError(17);
    }
    if (first_item->vtable_->get_item_type(first_item) != 20 ||
        second_item->vtable_->get_item_type(second_item) != 20) {
        ThrowCompoundEmblemError(23);
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const Inven_Item first = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), first_slot);
    const Inven_Item second = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), second_slot);
    if (first.isEmpty() ||
        static_cast<unsigned long>(static_cast<uint32_t>(first.item_id_)) !=
            first_item_id ||
        second.isEmpty() ||
        static_cast<unsigned long>(static_cast<uint32_t>(second.item_id_)) !=
            second_item_id) {
        ThrowCompoundEmblemError(17);
    }
    if (first_slot == second_slot && first.item_id_ == second.item_id_ &&
        first.get_add_info() <= 1) {
        ThrowCompoundEmblemError(17);
    }
    if (user->CheckItemLock(1, first_slot) ||
        user->CheckItemLock(1, second_slot)) {
        ThrowCompoundEmblemError(213);
    }

    const int grade = first_item->get_grade();
    if (grade != second_item->get_grade()) {
        ThrowCompoundEmblemError(216);
    }
    const EmblemCompoundInfo* const compound_info =
        G_CDataManager()->get_server_parameter_script()
            ->get_emblem_compound_info(grade);
    if (!compound_info || compound_info->money_cost == -1) {
        ThrowCompoundEmblemError(grade < 0 || grade > 6 ? 17 : 7);
    }

    const uint16_t types[2] = {
        static_cast<uint16_t>(
            static_cast<const CStackableItem*>(first_item)->getJewelTargetSocket()),
        static_cast<uint16_t>(
            static_cast<const CStackableItem*>(second_item)->getJewelTargetSocket())};
    const int jewel_type = _selectJewelTypeIndex(2, types);
    if (jewel_type == -1) {
        ThrowCompoundEmblemError(23);
    }

    const int result_item_id = compound_info->result_item_ids[jewel_type];
    CItem* const result_item = G_CDataManager()->find_item(result_item_id);
    if (!result_item || result_item->vtable_->get_item_type(result_item) != 16) {
        ThrowCompoundEmblemError(22);
    }

    if (inventory->use_money(compound_info->money_cost,
                             static_cast<eMoneySubReason>(35),
                             true) != 1) {
        ThrowCompoundEmblemError(10);
    }
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1), first_slot, 1,
                               static_cast<eItemDelReason>(8), true) != 1) {
        ThrowCompoundEmblemError(1);
    }
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1), second_slot, 1,
                               static_cast<eItemDelReason>(8), true) != 1) {
        ThrowCompoundEmblemError(1);
    }

    std::vector<stStackableBoosterItemInfo_t> booster_items;
    static_cast<const CStackableItem*>(result_item)->getBoosterItem(
        booster_items);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_COMPOUND_EMBLEM);
    response.put_byte(1);
    response.put_byte(static_cast<int>(booster_items.size()));
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator it =
             booster_items.begin();
         it != booster_items.end();
         ++it) {
        response.put_item_idx(static_cast<unsigned long>(it->item_index));
        response.put_int(it->item_count);
    }
    response.finalize(true);
    user->Send(response);

    std::vector<std::pair<int, int> > created_items;
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator it =
             booster_items.begin();
         it != booster_items.end();
         ++it) {
        CItem* const generated_item =
            G_CDataManager()->find_item(it->item_index);
        if (!generated_item) {
            continue;
        }

        Inven_Item generated;
        generated.item_id_ = it->item_index;
        generated_item->vtable_->make_item(generated_item, generated);
        generated.set_add_info(it->item_count);
        const int inserted = inventory->insertItemIntoInventory(
            generated, static_cast<eItemAddReason>(9), true, true);
        if (inserted < 0) {
            SendOverflowMail(user, generated);
        } else {
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), inserted);
        }
        created_items.push_back(std::make_pair(
            it->item_index, static_cast<int>(it->item_count)));
    }

    getCompoundEmblemStatistic()->increaseCount(
        static_cast<unsigned int>(grade));
    SendCreateEmblemInfo(created_items);
    return 0;
}

// IDA_EA: 0x8221DDE
// IDA_NAME: _ZN25Dispatcher_CompoundEmblem20SendCreateEmblemInfoERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: Dispatcher_CompoundEmblem::SendCreateEmblemInfo(std::vector<std::pair<int,int>> const&)
char Dispatcher_CompoundEmblem::SendCreateEmblemInfo(
    const std::vector<std::pair<int, int> >& items) {
    if (items.size() > 5u) {
        return 0;
    }

    Packet_Emblem_Create_Statistic statistic;
    int emitted = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (!item) {
            return 0;
        }
        statistic.emblem_grades_0e[emitted] = item->get_grade();
        statistic.emblem_counts_22[emitted] = it->second;
        if (++emitted > 5) {
            break;
        }
    }
    if (emitted != static_cast<int>(items.size())) {
        return 0;
    }
    statistic.emblem_count_0a = emitted;
    return static_cast<char>(
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(SERVER_GROUP_DEFAULT)
            ->SendPacket(reinterpret_cast<char*>(&statistic),
                         sizeof(statistic)));
}

// IDA_EA: 0x8233F6A
// IDA_NAME: _ZN25Dispatcher_CompoundEmblem4readER9PacketBufR8MSG_BASE
int Dispatcher_CompoundEmblem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F74
// IDA_NAME: _ZN25Dispatcher_CompoundEmblem7processEP5CUserR8MSG_BASER9ParamBase
int Dispatcher_CompoundEmblem::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596D90
// IDA_NAME: _ZN25Dispatcher_CompoundEmblemC2Ev
Dispatcher_CompoundEmblem::Dispatcher_CompoundEmblem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240F3C
// IDA_NAME: _ZN25Dispatcher_CompoundEmblemD2Ev
// DEMANGLED: Dispatcher_CompoundEmblem::~Dispatcher_CompoundEmblem()
// IDA_EA: 0x8240F6C
// IDA_NAME: _ZN25Dispatcher_CompoundEmblemD0Ev
// DEMANGLED: Dispatcher_CompoundEmblem::~Dispatcher_CompoundEmblem()
Dispatcher_CompoundEmblem::~Dispatcher_CompoundEmblem() {}
