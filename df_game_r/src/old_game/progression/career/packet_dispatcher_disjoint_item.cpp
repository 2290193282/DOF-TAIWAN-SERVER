#include "old_game/progression/career/packet_dispatcher_disjoint_item.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/progression/career/expert_job_disjointer.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"

#include <vector>

enum ENUM_ERROR : int {};

namespace {

void SendDisjointError(CUser* user, int command, int error) {
    if (user) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(command),
                                 static_cast<unsigned char>(error));
    }
}

// IDA_EA: 0x81F8A7C
// IDA_NAME: _Z30_check_disjointable_user_stateP5CUser14ENUM_CMDPACKETcs
bool CheckDisjointableUserState(CUser* user,
                                int command,
                                int item_space,
                                int item_slot) {
    if (!user || user->get_state() <= 2) {
        return false;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager && restrict_manager->isRestricted(
                                user, RestrictCategory::AuthMobile, 10)) {
        SendDisjointError(user, command, 209);
        return false;
    }

    CSecu_ProtectionField* const protection =
        GlobalData::s_pSecuProtectionField;
    if (protection) {
        const char wire_space = static_cast<char>(item_space);
        const short wire_slot = static_cast<short>(item_slot);
        const int error = protection->CheckItemRarity(
            user,
            static_cast<SECURITY_PROTCTION>(40),
            protection->GetItemPtr(user, wire_space, wire_slot),
            protection->GetInvenItemPtr(user, wire_space, wire_slot));
        if (error) {
            SendDisjointError(user, command, error);
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x81F8FE3 (NPC branch reached through GetPredictDisjointItems)
// IDA_NAME: _Z25_check_disjoint_need_slotP5CUserP5CItemR10Inven_ItemPN10expert_job11CDisjointerE
bool CheckNpcDisjointOutputCapacity(CUser* user, CItem* source_item) {
    if (!user || !source_item) {
        return false;
    }
    DisJoint* const disjoint = G_CDataManager()->get_disjoint();
    std::vector<int32_t>* const predicted =
        disjoint ? disjoint->GetPredictResult(source_item->get_rarity()) : 0;
    const CInventory* const inventory = user->getCurCharacInvenR();
    if (!predicted || !inventory) {
        return false;
    }

    // ELF _check_disjoint_need_slot @0x81F8FE3 keeps three eleven-entry
    // tables: required item-type slots, outputs which can reuse an existing
    // stack, and current empty slots.  Type 0 is the common/fallback space.
    int required[11] = {0};
    int reusable_stacks[11] = {0};
    int empty_slots[11] = {0};
    for (std::vector<int32_t>::const_iterator it = predicted->begin();
         it != predicted->end(); ++it) {
        if (*it == 0) {
            continue;
        }
        CItem* const output_data = G_CDataManager()->find_item(*it);
        if (!output_data || !output_data->vtable_ ||
            !output_data->vtable_->make_item) {
            continue;
        }

        Inven_Item output;
        output.item_id_ = *it;
        output.set_add_info(1);
        output_data->vtable_->make_item(output_data, output);
        const int item_type = static_cast<int>(output.item_category_);
        if (item_type < 0 || item_type > 10) {
            return false;
        }

        int reusable_type = 11;
        if (inventory->IsExistStackable(
                *it, static_cast<Inven_Item::ITEM_TYPE>(item_type))) {
            reusable_type = item_type;
        } else if (inventory->IsExistStackable(
                       *it, static_cast<Inven_Item::ITEM_TYPE>(0))) {
            reusable_type = 0;
        }
        if (reusable_type != 11) {
            ++reusable_stacks[reusable_type];
            ++required[reusable_type];
        } else {
            ++required[item_type];
        }
    }

    for (int item_type = 0; item_type <= 10; ++item_type) {
        empty_slots[item_type] = inventory->InvenEmptycount(
            static_cast<Inven_Item::ITEM_TYPE>(item_type));
    }
    int fallback_slots_needed = 0;
    for (int item_type = 0; item_type <= 10; ++item_type) {
        const int local_capacity =
            empty_slots[item_type] + reusable_stacks[item_type];
        if (local_capacity < required[item_type]) {
            fallback_slots_needed += required[item_type] - local_capacity;
        }
    }
    return empty_slots[0] >= fallback_slots_needed;
}

// IDA_EA: 0x81F8BA1 (portable-material branch)
// IDA_NAME: _Z30_check_disjointable_item_stateP5CUseriiR10Inven_ItemPP5CItemS0_t
int CheckPortableDisjointMaterial(CInventory* inventory,
                                  uint16_t material_slot,
                                  CItem* source_data) {
    if (material_slot == 0xffffu) {
        return 0;
    }
    const Inven_Item* const material = inventory
        ? inventory->GetInvenRef(1, static_cast<int>(material_slot))
        : 0;
    if (!material || material->item_id_ == 0 || material->get_add_info() < 1) {
        return 22;
    }
    CItem* const material_data =
        G_CDataManager()->find_item(material->item_id_);
    if (!material_data || material_data->is_stackable() != 1) {
        return 21;
    }
    const int portable_index =
        static_cast<CStackableItem*>(material_data)
            ->private_store_info_.expert_job_value_1;
    if (portable_index == -1) {
        return 1;
    }
    STExpertJobScript* const script = expert_job::CDisjointer::GetScript();
    const int maximum_level =
        script ? script->GetPortableDisjointPossibleLevel(portable_index) : -1;
    if (maximum_level == -1) {
        return 1;
    }
    if (!source_data) {
        return 17;
    }
    const int source_level = source_data->getUsableLevel() != 0
                                 ? source_data->getUsableLevel()
                                 : source_data->get_grade();
    return source_level > maximum_level ? 13 : 0;
}

}  // namespace

// IDA_EA: 0x859567C
// IDA_NAME: _ZN23DisPatcher_DisJointItemC2Ev
// DEMANGLED: DisPatcher_DisJointItem::DisPatcher_DisJointItem(void)
DisPatcher_DisJointItem::DisPatcher_DisJointItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82333B0
// IDA_NAME: _ZN23DisPatcher_DisJointItemD2Ev
// DEMANGLED: DisPatcher_DisJointItem::~DisPatcher_DisJointItem()
// IDA_EA: 0x82333E0
// IDA_NAME: _ZN23DisPatcher_DisJointItemD0Ev
// DEMANGLED: DisPatcher_DisJointItem::~DisPatcher_DisJointItem()
DisPatcher_DisJointItem::~DisPatcher_DisJointItem() {}

// IDA_EA: 0x81FA0F6
// IDA_NAME: _ZN23DisPatcher_DisJointItem22_checkDisjointableItemERK10Inven_ItemP5CUser
// DEMANGLED: DisPatcher_DisJointItem::_checkDisjointableItem(Inven_Item const&,CUser *)
int DisPatcher_DisJointItem::check_disjointable_item(
    const Inven_Item& inventory_item,
    CUser* machine_owner,
    CItem*& item_data) {
    if (inventory_item.amplify_option_.isIdentified() != 1 &&
        !machine_owner) {
        return 174;
    }

    item_data = G_CDataManager()->find_item(inventory_item.item_id_);
    if (!item_data) return 17;
    if (item_data->vtable_ && item_data->vtable_->is_impossible_disjoint &&
        item_data->vtable_->is_impossible_disjoint(item_data)) {
        return 19;
    }
    return item_data->GetAttachType() == 2 ? 17 : 0;
}

// IDA_EA: 0x81F92CA
// IDA_NAME: _ZN23DisPatcher_DisJointItem9_disjointEP5CUserii14ENUM_CMDPACKETS1_t
// DEMANGLED: DisPatcher_DisJointItem::_disjoint(CUser *,int,int,ENUM_CMDPACKET,CUser *,unsigned short)
int DisPatcher_DisJointItem::_disjoint(CUser* user,
                                       int item_slot,
                                       int item_space,
                                       int command,
                                       CUser* machine_owner,
                                       uint16_t material_slot) {
    // ELF symbol _check_disjointable_user_state @0x81F8A7C owns the mobile
    // restriction and item-rarity protection gates before any mutation.
    if (!user || !user->getCurCharacR() || user->CheckInTrade() ||
        !CheckDisjointableUserState(
            user, command, item_space, item_slot)) {
        return 0;
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(item_space));

    expert_job::CDisjointer* disjointer = 0;
    if (machine_owner) {
        expert_job::CExpertJob* const expert =
            machine_owner->GetCurCharacExpertJob();
        if (!expert || expert->GetType() != 3) {
            SendDisjointError(user, command, 19);
            return 0;
        }
        disjointer = static_cast<expert_job::CDisjointer*>(expert);
        // The original virtual call enforces fee/endurance and may close the
        // owner machine before it throws its command error.
        try {
            // Original vcall @0x81F944F passes the machine owner first and
            // the requester second.  The former must be checked for money
            // receive capacity; the latter must be able to pay the fee.
            disjointer->OnStartMission(machine_owner, user);
        } catch (ENUM_ERROR error) {
            SendDisjointError(user, command, static_cast<int>(error));
            return 0;
        }
    }

    if (user->CheckItemLock(inventory_type, item_slot)) {
        SendDisjointError(user, command, 213);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const source_ref = inventory->GetInvenRef(
        inventory_type, item_slot);
    if (!source_ref || source_ref->item_id_ == 0 ||
        source_ref->item_category_ == 5 ||
        !source_ref->isEquipableItemType()) {
        SendDisjointError(user, command, 17);
        return 0;
    }
    Inven_Item source = *source_ref;

    if (material_slot != 0xffffu && G_GameWorld()->IsPVPChannel()) {
        SendDisjointError(user, command, 96);
        return 0;
    }

    CItem* source_data = 0;
    const int source_error = check_disjointable_item(
        source, machine_owner, source_data);
    if (source_error != 0) {
        SendDisjointError(user, command, source_error);
        return 0;
    }

    const int material_error = CheckPortableDisjointMaterial(
        inventory, material_slot, source_data);
    if (material_error) {
        SendDisjointError(user, command, material_error);
        return 0;
    }

    if (!disjointer && !CheckNpcDisjointOutputCapacity(user, source_data)) {
        SendDisjointError(user, command, 4);
        return 0;
    }

    DisJointResult result;
    const int result_error = disjointer
        ? disjointer->get_disjoint_result(source_data, &source, result, user)
        : G_CDataManager()->get_disjoint()->GetResult(source.item_id_, result);
    if (result_error > 0) {
        SendDisjointError(user, command, result_error);
        return 0;
    }

    CItem* output_data[5] = {0, 0, 0, 0, 0};
    Inven_Item output_items[5];
    for (int index = 0; index <= 4; ++index) {
        if (result.values[index + 5] == 0) continue;
        output_data[index] = G_CDataManager()->find_item(result.values[index + 5]);
        if (!output_data[index]) continue;

        output_items[index].item_id_ = result.values[index + 5];
        output_items[index].set_add_info(result.values[index]);
        output_data[index]->vtable_->make_item(output_data[index],
                                                output_items[index]);
        if (inventory->tryInsertItemIntoInventory(output_items[index]) == -1) {
            SendDisjointError(user, command, 4);
            return 0;
        }
    }

    if (material_slot != 0xffffu) {
        if (inventory->use_item(static_cast<int>(material_slot), 1, 1) != 0) {
            SendDisjointError(user, command, 22);
            return 0;
        }
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             static_cast<int>(material_slot));
    }

    if (inventory->delete_item(static_cast<INVEN_TYPE>(inventory_type),
                               item_slot,
                               1,
                               static_cast<eItemDelReason>(9),
                               true) != 1) {
        SendDisjointError(user, command, 17);
        return 0;
    }

    int source_sell_price = 0;
    G_Store()->GetSellItemPrice(source,
                                source_data,
                                static_cast<short>(source.get_add_info()),
                                false,
                                source_sell_price);
    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(19), user,
        static_cast<unsigned int>(source_sell_price));

    // The raw call supplies the action-group argument through its former
    // register context.  Group 7 is the disjoint item action used by this
    // command family; the current AP provider intentionally has an empty body.
    APSystem::CUserProc::ClearActionAndSendtoUser(
        user,
        static_cast<APSystem::_ActionGroupIndex>(7),
        0,
        ENUM_PACKETCLASS_NOTIPACKET);

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET, command);
    response.put_byte(1);
    response.put_short(item_slot);
    response.put_byte(item_space);
    int output_count_position = response.get_index();
    response.put_byte(0);
    int output_count = 0;
    for (int index = 0; index <= 4; ++index) {
        if (!output_data[index]) continue;
        const int inserted_slot = inventory->insertItemIntoInventory(
            output_items[index],
            static_cast<eItemAddReason>(10),
            true,
            false);
        if (inserted_slot < 0) continue;

        response.put_short(inserted_slot);
        response.put_int(output_items[index].item_id_);
        response.put_int(output_items[index].get_add_info());

        GetInstanceCubeStatistic()->collectCubeStatistics(
            output_items[index].item_id_,
            output_items[index].get_add_info(),
            user,
            static_cast<CUBE_STATISTIC_FIELD>(machine_owner ? 2 : 0));
        int output_sell_price = 0;
        G_Store()->GetSellItemPrice(output_items[index],
                                    output_data[index],
                                    static_cast<short>(
                                        output_items[index].get_add_info()),
                                    false,
                                    output_sell_price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(8), user,
            static_cast<unsigned int>(output_sell_price));
        ++output_count;
    }
    response.put_byte(output_count_position, output_count);
    if (disjointer) disjointer->complete_disjoint(user, response);
    response.finalize(true);
    user->Send(response);
    HistoryLog::WriteDisJoint(user->history_stream(), source.item_id_);
    return 0;
}

// IDA_EA: 0x81F9FB8
// IDA_NAME: _ZN23DisPatcher_DisJointItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DisJointItem::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_DisJointItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->CheckInTrade()) return 0;

    short item_slot = 0;
    unsigned char item_space = 0;
    unsigned short material_slot = 0xffffu;
    if (packet.get_short(item_slot) != 1) {
        return LineFunc(35024,
                        "virtual int DisPatcher_DisJointItem::dispatch_sig(CUser*, PacketBuf&)",
                        0, 0);
    }
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(35026,
                        "virtual int DisPatcher_DisJointItem::dispatch_sig(CUser*, PacketBuf&)",
                        0, 0);
    }
    if (packet.get_short(material_slot) != 1) {
        return LineFunc(35030,
                        "virtual int DisPatcher_DisJointItem::dispatch_sig(CUser*, PacketBuf&)",
                        0, 0);
    }
    return _disjoint(user,
                     static_cast<int>(item_slot),
                     static_cast<int>(item_space),
                     ENUM_CMDPACKET_DISJOINT_ITEM,
                     0,
                     material_slot);
}

// CMD28's state-0 template never calls these overridden no-op members; they
// exist as the concrete vtable slots retained by the original class.
// IDA_EA: 0x823339C
// IDA_NAME: _ZN23DisPatcher_DisJointItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DisJointItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DisJointItem::read(PacketBuf&, MSG_BASE&) { return 0; }
// IDA_EA: 0x82333A6
// IDA_NAME: _ZN23DisPatcher_DisJointItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DisJointItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DisJointItem::process(CUser*, MSG_BASE&, ParamBase&) { return 0; }
