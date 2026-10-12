#include "old_game/dungeon/instance/compound_extream_dun_item_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/dungeon/instance/packet_dispatcher_compound_extream_item.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "infra/log/format/trace.h"

// IDA_DATA: GlobalData::s_pCompoundBindCube
// IDA_NAME: _ZN10GlobalData19s_pCompoundBindCubeE
// Storage is owned by global_data_public_globals_abi.cpp.  This typed consumer
// declaration must not create a second bind-cube lifetime.

// IDA_EA: 0x8597244
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItemC2Ev
// DEMANGLED: Dispatcher_Compound_ExtreamItem::Dispatcher_Compound_ExtreamItem(void)
Dispatcher_Compound_ExtreamItem::Dispatcher_Compound_ExtreamItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {
    // The original C2 constructs only the three Inven_Item subobjects.  The
    // scalar request fields are set by _reset() immediately after user_ is
    // assigned in dispatch_sig(); do not invent an alternate constructor
    // state here.
}

// IDA_EA: 0x824057C
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItemD2Ev
// DEMANGLED: Dispatcher_Compound_ExtreamItem::~Dispatcher_Compound_ExtreamItem()
// IDA_EA: 0x82405AC
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItemD0Ev
// DEMANGLED: Dispatcher_Compound_ExtreamItem::~Dispatcher_Compound_ExtreamItem()
Dispatcher_Compound_ExtreamItem::~Dispatcher_Compound_ExtreamItem() {}

// IDA_EA: 0x822577E
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem6_resetEv
// DEMANGLED: Dispatcher_Compound_ExtreamItem::_reset(void)
Dispatcher_Compound_ExtreamItem* Dispatcher_Compound_ExtreamItem::_reset() {
    for (int index = 0; index <= 1; ++index) {
        source_slots_[index] = 0;
        input_items_[index].reset();
        input_prototypes_[index] = 0;
    }
    result_slot_ = 0;
    output_item_.reset();
    cube_slot_ = 0;
    selection_type_ = 2;
    sphere_type_ = 2;
    return this;
}

// IDA_EA: 0x8225806
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem12_read_packetER9PacketBuf
// DEMANGLED: Dispatcher_Compound_ExtreamItem::_read_packet(PacketBuf &)
int Dispatcher_Compound_ExtreamItem::_read_packet(PacketBuf& packet) {
    if (!user_) {
        return 0;
    }

    _reset();
    if (packet.get_short(cube_slot_) != 1) return 0;
    for (int index = 0; index <= 1; ++index) {
        if (packet.get_short(source_slots_[index]) != 1) return 0;
        input_items_[index] =
            user_->getCurCharacInvenR()->GetInvenSlot(1, source_slots_[index]);
        if (!input_items_[index].isEquipableItemType()) {
            return 0;
        }
        input_prototypes_[index] = reinterpret_cast<const CEquipItem*>(
            G_CDataManager()->find_item(input_items_[index].item_id_));
        if (!input_prototypes_[index]) return 0;
    }

    if (packet.get_byte(selection_type_) != 1) return 0;
    if (source_slots_[0] == source_slots_[1] ||
        cube_slot_ == source_slots_[0] || cube_slot_ == source_slots_[1]) return 0;
    if (selection_type_ > 2) {
        return 0;
    }

    const Inven_Item cube =
        user_->getCurCharacInvenR()->GetInvenSlot(1, cube_slot_);
    if (cube.isEmpty()) {
        return 0;
    }
    switch (cube.item_id_) {
        case 1251:
            sphere_type_ = 0;
            break;
        case 1252:
            sphere_type_ = 1;
            break;
        case 2670287:
            sphere_type_ = 3;
            break;
        default:
            sphere_type_ = 2;
            break;
    }
    return 1;
}

// IDA_EA: 0x8225A88
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem14_remove_victimEv
// DEMANGLED: Dispatcher_Compound_ExtreamItem::_remove_victim(void)
bool Dispatcher_Compound_ExtreamItem::_remove_victim() {
    CInventory* const inventory = user_->getCurCharacInvenW();
    for (int index = 0; index <= 1; ++index) {
        if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                   source_slots_[index],
                                   1,
                                   static_cast<eItemDelReason>(42),
                                   true) != 1) {
            return false;
        }
    }
    return inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                  cube_slot_,
                                  1,
                                  static_cast<eItemDelReason>(42),
                                  true) == 1;
}

// IDA_EA: 0x8225B54
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem13_push_newitemEv
// DEMANGLED: Dispatcher_Compound_ExtreamItem::_push_newitem(void)
bool Dispatcher_Compound_ExtreamItem::_push_newitem() {
    result_slot_ = static_cast<short>(
        user_->getCurCharacInvenW()->insertItemIntoInventory(
            output_item_, static_cast<eItemAddReason>(48), true, true));
    return result_slot_ >= 0;
}

// IDA_EA: 0x8225C54
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem12_send_resultEv
// DEMANGLED: Dispatcher_Compound_ExtreamItem::_send_result(void)
void Dispatcher_Compound_ExtreamItem::_send_result() {
    PacketGuard result;
    result.put_header(1, ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE);
    result.put_byte(1);
    for (int index = 0; index <= 1; ++index) {
        result.put_byte(0);
        result.put_short(source_slots_[index]);
        result.put_int(1);
    }
    result.put_byte(0);
    result.put_short(cube_slot_);
    result.put_int(1);
    result.put_short(result_slot_);
    result.put_int(output_item_.item_id_);
    result.put_int(output_item_.add_info_);
    result.put_byte(output_item_.GetItemAttr());
    result.put_short(output_item_.avatar_select_);
    result.put_byte(output_item_.seal_or_state_);
    result.put_int(G_GameWorld()->IsEnchantRevisionChannel()
                       ? 0
                       : output_item_.extend_info_);
    result.put_byte(output_item_.amplify_option_.getAbilityType());
    result.put_short(output_item_.amplify_option_.getAbilityValue());
    result.put_packet(output_item_);
    result.finalize(true);
    user_->Send(result);
}

// IDA_EA: 0x8225EC4
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Compound_ExtreamItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Compound_ExtreamItem::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }
    user_ = user;

    const int parsed = _read_packet(packet);
    const unsigned char error = parsed == 1 && GlobalData::s_pCompoundBindCube
        ? static_cast<unsigned char>(GlobalData::s_pCompoundBindCube->checkCompoundable(
              input_items_, input_prototypes_, static_cast<char>(sphere_type_)))
        : 17;
    LogManager::logFormat(1, "CompoundBindCube.cpp", __FUNCTION__, __LINE__,
        "[BIND_CUBE][CHECK] char=%d parsed=%d cube_slot=%u sphere=%u selection=%u "
        "slot0=%u item0=%d level0=%d kind0=%d slot1=%u item1=%d level1=%d kind1=%d error=%u",
        user_->getCurCharacNo(), parsed, static_cast<unsigned int>(cube_slot_),
        static_cast<unsigned int>(sphere_type_), static_cast<unsigned int>(selection_type_),
        static_cast<unsigned int>(source_slots_[0]), input_items_[0].item_id_,
        input_prototypes_[0] ? input_prototypes_[0]->getUsableLevel() : -1,
        input_prototypes_[0] ? input_prototypes_[0]->GetUsableEquipmentType() : -1,
        static_cast<unsigned int>(source_slots_[1]), input_items_[1].item_id_,
        input_prototypes_[1] ? input_prototypes_[1]->getUsableLevel() : -1,
        input_prototypes_[1] ? input_prototypes_[1]->GetUsableEquipmentType() : -1,
        static_cast<unsigned int>(error));
    if (error != 0) {
        user_->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE, error);
        return 0;
    }

    if (user_->CheckItemLock(1, source_slots_[0]) ||
        user_->CheckItemLock(1, source_slots_[1])) {
        user_->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE, 213);
        return 0;
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user_, RestrictCategory::AuthMobile, 13)) {
        user_->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE, 209);
        return 0;
    }

    // The raw sphere discriminator is used by checkCompoundable, but the
    // original 0x8225EC4 call passes only `(sphere_type_ == 3)` to the level
    // bonus table.  bindsphere.etc has two tables (ordinary/special), not four;
    // forwarding raw values 1 or 3 selects an empty/out-of-domain table.
    const ExtreamDungeon::eBindSphereType level_bonus_type =
        static_cast<ExtreamDungeon::eBindSphereType>(sphere_type_ == 3 ? 1 : 0);
    if (GlobalData::s_pCompoundBindCube->makeCompoundItem(
            input_prototypes_, static_cast<char>(selection_type_), output_item_,
            level_bonus_type) == 1) {
        const int reseal_count =
            static_cast<unsigned char>(input_items_[0].GetReSealCount()) +
            static_cast<unsigned char>(input_items_[1].GetReSealCount());
        output_item_.SetReSealCount(
            static_cast<unsigned char>(reseal_count / 2.0 + 0.5));
        if (output_item_.item_id_ != 0 && _remove_victim()) {
            // Original order: result is sent even when post-delete insertion
            // reports no slot. Keep that behavior rather than invent rollback.
            _push_newitem();
            _send_result();
            return 0;
        }
    }

    LogManager::logFormat(1, "CompoundBindCube.cpp", __FUNCTION__, __LINE__,
        "[BIND_CUBE][GENERATE_FAIL] char=%d item0=%d item1=%d sphere=%u selection=%u output=%d",
        user_->getCurCharacNo(), input_items_[0].item_id_, input_items_[1].item_id_,
        static_cast<unsigned int>(sphere_type_), static_cast<unsigned int>(selection_type_),
        output_item_.item_id_);
    user_->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE, 17);
    return 0;
}

// IDA_EA: 0x823419A
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Compound_ExtreamItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Compound_ExtreamItem::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82341A4
// IDA_NAME: _ZN31Dispatcher_Compound_ExtreamItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Compound_ExtreamItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Compound_ExtreamItem::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
