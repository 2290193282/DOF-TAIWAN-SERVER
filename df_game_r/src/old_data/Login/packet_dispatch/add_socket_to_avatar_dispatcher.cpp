#include "old_data/Login/packet_dispatch/add_socket_to_avatar_dispatcher.h"

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/avatar_aura/db_update_avatar_jewel_slot.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kAddSocketDispatchSignature[] =
    "virtual int Dispatcher_AddSocketToAvatar::dispatch_sig(CUser*, PacketBuf&)";

const INVEN_TYPE kSocketMaterialInventory = static_cast<INVEN_TYPE>(1);
const INVEN_TYPE kSocketAvatarInventory = static_cast<INVEN_TYPE>(2);
const eItemDelReason kSocketDeleteReason = static_cast<eItemDelReason>(27);
// 0828 client GetStackableActionTypeByName @0x01541850 maps
// [add avatar socket] to 33.  The 0627 server used 32, so retaining the old
// literal rejects every target-PVF socket machine before its action payload
// can be evaluated.
const StackableAction::Type kAddAvatarSocketAction =
    static_cast<StackableAction::Type>(33);

enum AddSocketMaterialCondition {
    kOrdinaryAvatarSocket = 0,
    kCloneAvatarSocket = 1,
    kAnyLegacyAvatarSocket = 2,
    kSkinAvatarSocket = 3,
};

}  // namespace

// IDA_EA: 0x822DD2C
// IDA_NAME: _ZN24MSG_ADD_SOCKET_TO_AVATARC2Ev
// DEMANGLED: MSG_ADD_SOCKET_TO_AVATAR::MSG_ADD_SOCKET_TO_AVATAR(void)
MSG_ADD_SOCKET_TO_AVATAR::MSG_ADD_SOCKET_TO_AVATAR()
    : MSG_BASE(), avatar_slot(0), avatar_item_id(0), material_slot(0) {}

// IDA_EA: 0x8596914
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatarC2Ev
// DEMANGLED: Dispatcher_AddSocketToAvatar::Dispatcher_AddSocketToAvatar(void)
Dispatcher_AddSocketToAvatar::Dispatcher_AddSocketToAvatar()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241776
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatarD2Ev
// DEMANGLED: Dispatcher_AddSocketToAvatar::~Dispatcher_AddSocketToAvatar()
// IDA_EA: 0x82417A6
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatarD0Ev
// DEMANGLED: Dispatcher_AddSocketToAvatar::~Dispatcher_AddSocketToAvatar()
Dispatcher_AddSocketToAvatar::~Dispatcher_AddSocketToAvatar() {}

// IDA_EA: 0x821A412
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AddSocketToAvatar::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AddSocketToAvatar::dispatch_sig(CUser* user, PacketBuf& packet) {
    MSG_ADD_SOCKET_TO_AVATAR request;
    if (packet.get_short(request.avatar_slot) != 1) {
        return LineFunc(52576, kAddSocketDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.avatar_item_id) != 1) {
        return LineFunc(52578, kAddSocketDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.material_slot) != 1) {
        return LineFunc(52580, kAddSocketDispatchSignature, 0, 0);
    }

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        request.avatar_slot >= oldsrv::config::kAvatarInventoryItemCount ||
        request.material_slot == 0u ||
        request.material_slot >= oldsrv::config::kMainInventoryItemCount) {
        if (user) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        }
        return 0;
    }
    const int socket_action_error = user->isEnableAvatarSocketAction();
    if (socket_action_error != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_ADD_AVATAR_SOCKET,
            static_cast<unsigned char>(socket_action_error));
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }
    const Inven_Item avatar = inventory->GetInvenSlot(
        kSocketAvatarInventory, request.avatar_slot);
    if (avatar.isEmpty() ||
        avatar.item_id_ != static_cast<int32_t>(request.avatar_item_id) ||
        user->CheckItemLock(kSocketAvatarInventory, request.avatar_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 4);
        return 0;
    }

    const Inven_Item material = inventory->GetInvenSlot(
        kSocketMaterialInventory, request.material_slot);
    if (material.isEmpty() ||
        user->CheckItemLock(kSocketMaterialInventory,
                            request.material_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 22);
        return 0;
    }

    CItem* const material_prototype = G_CDataManager()->find_item(
        material.item_id_);
    if (!material_prototype || material_prototype->is_stackable() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }
    const CStackableItem* const socket_material =
        static_cast<const CStackableItem*>(material_prototype);
    if (!socket_material->verifyStackableAction(kAddAvatarSocketAction)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 4);
        return 0;
    }

    if (!avatar.isAvatarItemType()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }
    CItem* const avatar_prototype = G_CDataManager()->find_item(
        static_cast<int>(request.avatar_item_id));
    if (!avatar_prototype ||
        !avatar_prototype->vtable_->is_avatar_item(avatar_prototype)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }

    CEquipItem* const avatar_equipment =
        static_cast<CEquipItem*>(avatar_prototype);
    const std::vector<int>& material_conditions = socket_material->action_values();
    if (material_conditions.size() != 1u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 22);
        return 0;
    }

    const int material_condition = material_conditions[0];
    // The target client classifies action value 1 as a clone-avatar
    // machine.  Clone avatars are the PVF [clear avatar] family; this value
    // is unrelated to creature equipment.  Value 2 is the general non-skin
    // path used by the rare machine and therefore accepts either ordinary or
    // clone avatars of the same grade.
    const bool avatar_is_clone = avatar_equipment->IsClearAvatar() != 0;
    const bool avatar_is_skin = avatar_equipment->check_eType(8) != 0;
    const bool condition_matches =
        (material_condition == kOrdinaryAvatarSocket &&
         !avatar_is_clone && !avatar_is_skin) ||
        (material_condition == kCloneAvatarSocket &&
         avatar_is_clone && !avatar_is_skin) ||
        (material_condition == kAnyLegacyAvatarSocket && !avatar_is_skin) ||
        (material_condition == kSkinAvatarSocket && avatar_is_skin &&
         avatar_equipment->HasSkinEmblemSocket());
    const bool grade_matches =
        avatar_prototype->get_grade() == material_prototype->get_grade();
    if (!condition_matches || !grade_matches ||
        !socket_material->can_add_socket_to_avatar(
            static_cast<int>(request.avatar_item_id))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }

    const int avatar_key = avatar.add_info_;
    if (WongWork::CAvatarItemMgr::IsTempKey(avatar_key)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }

    WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrW();
    if (!avatar_manager || avatar_manager->isSocketAvatar(avatar_key)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 19);
        return 0;
    }

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    if (avatar_equipment->getAvatarSocket(-1, emblem) != 0 ||
        emblem.sockets[0].socket_type == 0u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }

    // Preflight the material mutation in a detached inventory before changing
    // the live avatar manager.  This prevents a socket record from becoming
    // visible when the material cannot actually be consumed.
    InventoryMemory preview_memory;
    CInventory preview;
    preview.SetInventoryMemory(&preview_memory);
    if (!preview.setCopy(*user->getCurCharacInvenR()) ||
        preview.delete_item(kSocketMaterialInventory,
                            request.material_slot,
                            1,
                            kSocketDeleteReason,
                            false) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 4);
        return 0;
    }

    if (inventory->delete_item(kSocketMaterialInventory,
                               request.material_slot,
                               1,
                               kSocketDeleteReason,
                               false) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 4);
        return 0;
    }
    if (avatar_manager->setEmblemSocketData(avatar_key, emblem) != 1) {
        (void)inventory->update_item(kSocketMaterialInventory,
                                     request.material_slot,
                                     material);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_AVATAR_SOCKET, 17);
        return 0;
    }

    cUserHistoryLog* const history = CUser_history_log(user);
    if (history) {
        const int remaining = material.get_add_info() - 1;
        Inven_Item log_item = material;
        if (remaining > 0) {
            log_item.set_add_info(remaining);
        }
        history->ItemDel(kSocketMaterialInventory,
                         remaining,
                         1,
                         log_item,
                         kSocketDeleteReason);
    }

    DB_UpdateAvatarJewelSlot::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        static_cast<unsigned int>(avatar_key), &emblem);
    user->SaveInventory();

    // CMD206 only returns the avatar/material slots.  Publish the avatar
    // record through NOTI14 so the client receives the new 30-byte socket
    // block before it handles the success response and redraws the item.
    user->SendUpdateItemList(
        static_cast<CUser::eSendTarget>(1),
        static_cast<ENUM_ITEMSPACE>(1),
        request.avatar_slot);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_ADD_AVATAR_SOCKET);
    response.put_byte(1);
    response.put_short(request.avatar_slot);
    response.put_short(request.material_slot);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233D4E
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AddSocketToAvatar::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AddSocketToAvatar::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D58
// IDA_NAME: _ZN28Dispatcher_AddSocketToAvatar7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AddSocketToAvatar::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AddSocketToAvatar::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
