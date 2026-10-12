#include "old_game/economy/inventory/packet_dispatcher_mod_item_attr.h"

#include "common/config.h"
#include <algorithm>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/reseal_table.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/server_utility.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const ENUM_CMDPACKET kCmdModItemAttr = ENUM_CMDPACKET_RESET_ITEM_ATTR;

enum {
    kInventoryType = 1,
    kServiceRestrictId = 15,
    kResealProtection = 38,
    kItemLockedError = 213,
    kCandleGoldenWaxAction = 7,
};

const char kModItemAttrDispatchSignature[] =
    "virtual int Dispatcher_ModItemAttr::dispatch_sig(CUser*, PacketBuf&)";

bool IsRandomAttributeMaterial(int item_id) {
    return item_id == 2675022 || item_id == 15 || item_id == 897 ||
           item_id == 690000114;
}

bool IsNormalResealMaterial(const CItem* prototype) {
    if (!prototype || prototype->is_stackable() != 1) {
        return false;
    }
    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(prototype);
    return stackable->verifyStackableAction(
               static_cast<StackableAction::Type>(
                   kCandleGoldenWaxAction)) &&
           stackable->action_values().empty();
}

}  // namespace

// IDA_EA: 0x8595D28
// IDA_NAME: _ZN22Dispatcher_ModItemAttrC2Ev
// DEMANGLED: Dispatcher_ModItemAttr::Dispatcher_ModItemAttr(void)
Dispatcher_ModItemAttr::Dispatcher_ModItemAttr()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243246
// IDA_NAME: _ZN22Dispatcher_ModItemAttrD2Ev
// DEMANGLED: Dispatcher_ModItemAttr::~Dispatcher_ModItemAttr()
// IDA_EA: 0x8243276
// IDA_NAME: _ZN22Dispatcher_ModItemAttrD0Ev
// DEMANGLED: Dispatcher_ModItemAttr::~Dispatcher_ModItemAttr()
Dispatcher_ModItemAttr::~Dispatcher_ModItemAttr() {}

// IDA_EA: 0x8200B08
// IDA_NAME: _ZN22Dispatcher_ModItemAttr12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ModItemAttr::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ModItemAttr::dispatch_sig(CUser* user, PacketBuf& packet) {
  try {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        // The original logs account/state here, then throws the dispatcher
        // int exception consumed by the outer packet loop.
        throw 0;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(
            user, RestrictCategory::AuthMobile, kServiceRestrictId)) {
        user->SendCmdErrorPacket(kCmdModItemAttr,
                                 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(kResealProtection));
    if (security_error != 0) {
        user->SendCmdErrorPacket(kCmdModItemAttr,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }

    if (user->CheckInTrade()) {
        return 0;
    }

    uint16_t target_slot = 0;
    uint32_t target_item_id = 0;
    uint16_t material_slot = 0;
    if (packet.get_short(target_slot) != 1 ||
        packet.get_int(target_item_id) != 1 ||
        packet.get_short(material_slot) != 1) {
        throw -1;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item target = inventory->GetInvenSlot(kInventoryType, target_slot);
    if (target.isEmpty() ||
        static_cast<uint32_t>(target.item_id_) != target_item_id) {
        throw static_cast<ENUM_ERROR>(4);
    }
    if (target.item_category_ != 1) {
        throw static_cast<ENUM_ERROR>(19);
    }

    CItem* const target_prototype = G_CDataManager()->find_item(target.item_id_);
    if (!target_prototype) {
        throw static_cast<ENUM_ERROR>(4);
    }
    const int target_type =
        target_prototype->vtable_->get_item_type(target_prototype);
    if (target_type < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        target_type > oldsrv::config::kOrdinaryEquipmentTypeLast) {
        throw static_cast<ENUM_ERROR>(19);
    }

    const Inven_Item material =
        inventory->GetInvenSlot(kInventoryType, material_slot);
    if (material.isEmpty()) {
        throw static_cast<ENUM_ERROR>(17);
    }
    if (user->CheckItemLock(kInventoryType, target_slot)) {
        user->SendCmdErrorPacket(kCmdModItemAttr,
                                 kItemLockedError);
        return 0;
    }

    const int material_id = material.item_id_;
    const CItem* const material_prototype =
        G_CDataManager()->find_item(material_id);
    uint16_t operation = 0;
    int material_cost = 1;
    if (IsRandomAttributeMaterial(material_id)) {
        operation = 2;
    } else if (IsNormalResealMaterial(material_prototype)) {
        operation = 1;
        if (static_cast<unsigned char>(target.GetReSealCount()) > 6u) {
            throw static_cast<ENUM_ERROR>(13);
        }
        if (target_type == oldsrv::config::kEquipmentTypeTitle ||
            target_prototype->GetAttachType() != 3) {
            throw static_cast<ENUM_ERROR>(19);
        }
        reseal_table_t* const reseal_table =
            G_CDataManager()->get_reseal_table();
        const unsigned char rarity =
            static_cast<unsigned char>(target_prototype->get_rarity());
        const unsigned char grade =
            static_cast<unsigned char>(target_prototype->get_grade());
        const unsigned char reseal_count =
            static_cast<unsigned char>(target.GetReSealCount());
        material_cost =
            reseal_table->get_reseal_cost(rarity, grade, reseal_count);
    } else {
        throw static_cast<ENUM_ERROR>(17);
    }

    if (material_cost < 0) {
        throw -2;
    }
    if (material.get_add_info() < material_cost) {
        throw static_cast<ENUM_ERROR>(22);
    }
    // Validate the complete reseal transaction before consuming its
    // PVF-authorized material.  A repeated request against an already sealed
    // target must be a no-op for both inventory records.
    if (operation == 1 && target.seal_or_state_ != 0) {
        throw static_cast<ENUM_ERROR>(18);
    }
    if (inventory->delete_item(static_cast<INVEN_TYPE>(kInventoryType),
                               material_slot, material_cost,
                               static_cast<eItemDelReason>(3), true) != 1) {
        throw -3;
    }

    if (operation == 1) {
        target.seal_or_state_ = 1;
        const int next_reseal_count = std::min(
            7, static_cast<int>(static_cast<unsigned char>(
                   target.GetReSealCount())) +
                   1);
        target.SetReSealCount(static_cast<uint8_t>(next_reseal_count));
    } else {
        target.set_add_info(static_cast<int>(get_rand_int(0)));
    }

    inventory->update_item(static_cast<INVEN_TYPE>(kInventoryType), target_slot,
                           target);
    _SendResult(user, operation, material_slot, target_slot);

    // The legacy call takes byte +2 of the material Inven_Item record, which
    // is the low byte of the material item index, not the item-category byte.
    const int return_rate = user->GetPremiumInfo()->GetReturnItemRate(
        static_cast<unsigned char>(material.item_id_));
    const int returned_count = return_rate * material_cost / 100;
    if (returned_count > 0) {
        Inven_Item returned = material;
        returned.set_add_info(returned_count);
        const int returned_slot = inventory->insertItemIntoInventory(
            returned, static_cast<eItemAddReason>(14), true, false);
        if (returned_slot >= 0) {
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     returned_slot);
        }
    }
    return 0;
  } catch (ENUM_ERROR error) {
    // The original CMD81 body owns this business-error boundary.  Keeping it
    // local returns the command error without changing every other dispatcher
    // or allowing a normal validation failure to terminate DispatchThread.
    if (user) {
        user->SendCmdErrorPacket(
            kCmdModItemAttr, static_cast<unsigned char>(error));
    }
    return 0;
  } catch (int error) {
    // Preserve the old internal-error contract: negative implementation
    // failures are traced, while non-negative values simply end this command.
    return error < 0
        ? LineFunc(38662, kModItemAttrDispatchSignature, 0, 0)
        : 0;
  }
}

// IDA_EA: 0x8201938
// IDA_NAME: _ZN22Dispatcher_ModItemAttr11_SendResultEP5CUserttt
// DEMANGLED: Dispatcher_ModItemAttr::_SendResult(CUser *,unsigned short,
//             unsigned short,unsigned short)
void Dispatcher_ModItemAttr::_SendResult(CUser* user,
                                         uint16_t operation,
                                         uint16_t material_slot,
                                         uint16_t target_slot) {
    const Inven_Item material = user->getCurCharacInvenR()->GetInvenSlot(
        kInventoryType, material_slot);
    PacketGuard response;
    response.put_header(1, kCmdModItemAttr);
    response.put_byte(1);
    response.put_short(material_slot);
    response.put_int(material.get_add_info());
    response.put_short(operation);
    response.finalize(true);
    user->Send(response);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), target_slot);
}

// IDA_EA: 0x82335D0
// IDA_NAME: _ZN22Dispatcher_ModItemAttr4readER9PacketBufR8MSG_BASE
int Dispatcher_ModItemAttr::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82335DA
// IDA_NAME: _ZN22Dispatcher_ModItemAttr7processEP5CUserR8MSG_BASER9ParamBase
int Dispatcher_ModItemAttr::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
