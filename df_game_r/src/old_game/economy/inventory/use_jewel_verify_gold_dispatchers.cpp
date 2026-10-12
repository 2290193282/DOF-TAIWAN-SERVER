#include "old_game/economy/inventory/use_jewel_verify_gold_dispatchers.h"

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/avatar_aura/db_update_avatar_jewel_slot.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kUseJewelSignature[] =
    "virtual int Dispatcher_UseJewel::dispatch_sig(CUser*, PacketBuf&)";
const INVEN_TYPE kMainInventory = static_cast<INVEN_TYPE>(1);
const INVEN_TYPE kAvatarInventory = static_cast<INVEN_TYPE>(2);
const eItemDelReason kUseEmblemDeleteReason =
    static_cast<eItemDelReason>(8);

struct NativeEmblemUse {
    uint16_t material_slot;
    uint32_t emblem_item_id;
    uint8_t target_socket;
};

struct AggregatedEmblemMaterial {
    uint16_t material_slot;
    uint32_t emblem_item_id;
    int count;
    Inven_Item snapshot;
};

void RestoreEmblemMaterials(
    CInventory* inventory,
    const std::vector<AggregatedEmblemMaterial>& materials,
    std::size_t changed_count) {
    if (!inventory) {
        return;
    }
    for (std::size_t index = 0;
         index < changed_count && index < materials.size();
         ++index) {
        (void)inventory->update_item(kMainInventory,
                                     materials[index].material_slot,
                                     materials[index].snapshot);
    }
}

}  // namespace


// IDA_EA: 0x85968A4
// IDA_NAME: _ZN19Dispatcher_UseJewelC2Ev
// DEMANGLED: Dispatcher_UseJewel::Dispatcher_UseJewel(void)
Dispatcher_UseJewel::Dispatcher_UseJewel()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82418AE
// IDA_NAME: _ZN19Dispatcher_UseJewelD2Ev
// DEMANGLED: Dispatcher_UseJewel::~Dispatcher_UseJewel()
// IDA_EA: 0x82418DE
// IDA_NAME: _ZN19Dispatcher_UseJewelD0Ev
// DEMANGLED: Dispatcher_UseJewel::~Dispatcher_UseJewel()
Dispatcher_UseJewel::~Dispatcher_UseJewel() {}

// IDA_EA: 0x8217BD6
// IDA_NAME: _ZN19Dispatcher_UseJewel12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UseJewel::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UseJewel::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint16_t avatar_slot = 0;
    uint32_t avatar_item_id = 0;
    uint8_t request_count = 0;
    if (packet.get_short(avatar_slot) != 1 ||
        packet.get_int(avatar_item_id) != 1 ||
        packet.get_byte(request_count) != 1 || request_count == 0u ||
        request_count > 5u) {
        return LineFunc(51682, kUseJewelSignature, 0, 0);
    }

    std::vector<NativeEmblemUse> requests;
    requests.reserve(request_count);
    for (unsigned int index = 0; index < request_count; ++index) {
        NativeEmblemUse request = {0u, 0u, 0u};
        if (packet.get_short(request.material_slot) != 1 ||
            packet.get_int(request.emblem_item_id) != 1 ||
            packet.get_byte(request.target_socket) != 1) {
            return LineFunc(51682, kUseJewelSignature, 0, 0);
        }
        requests.push_back(request);
    }
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        avatar_slot >= oldsrv::config::kAvatarInventoryItemCount) {
        if (user) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
        }
        return 0;
    }
    const int socket_action_error = user->isEnableAvatarSocketAction();
    if (socket_action_error != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_EMBLEM,
            static_cast<unsigned char>(socket_action_error));
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
        return 0;
    }
    const Inven_Item avatar =
        inventory->GetInvenSlot(kAvatarInventory, avatar_slot);
    if (avatar.isEmpty() ||
        static_cast<uint32_t>(avatar.item_id_) != avatar_item_id ||
        user->CheckItemLock(kAvatarInventory, avatar_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 4);
        return 0;
    }

    CItem* const avatar_prototype =
        G_CDataManager()->find_item(static_cast<int>(avatar_item_id));
    const int avatar_key = avatar.add_info_;
    WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrW();
    if (!avatar_prototype ||
        !avatar_prototype->vtable_->is_avatar_item(avatar_prototype) ||
        WongWork::CAvatarItemMgr::IsTempKey(avatar_key) || !avatar_manager) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
        return 0;
    }
    const stAvatarEmblemInfo_t* const current_emblem =
        avatar_manager->getJewelSocketData(avatar_key);
    if (!current_emblem) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
        return 0;
    }

    stAvatarEmblemInfo_t next_emblem = *current_emblem;
    const stAvatarEmblemInfo_t previous_emblem = *current_emblem;
    std::set<uint8_t> target_sockets;
    std::vector<AggregatedEmblemMaterial> materials;
    for (std::vector<NativeEmblemUse>::const_iterator request =
             requests.begin();
         request != requests.end();
         ++request) {
        if (request->target_socket >= 5u ||
            !target_sockets.insert(request->target_socket).second ||
            request->material_slot == 0u ||
            request->material_slot >=
                oldsrv::config::kMainInventoryItemCount) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
            return 0;
        }

        const Inven_Item material = inventory->GetInvenSlot(
            kMainInventory, request->material_slot);
        if (material.isEmpty() ||
            static_cast<uint32_t>(material.item_id_) !=
                request->emblem_item_id) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 4);
            return 0;
        }
        if (user->CheckItemLock(kMainInventory, request->material_slot)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 213);
            return 0;
        }
        CItem* const emblem_prototype = G_CDataManager()->find_item(
            static_cast<int>(request->emblem_item_id));
        if (!emblem_prototype || emblem_prototype->is_stackable() != 1 ||
            emblem_prototype->vtable_->get_item_type(emblem_prototype) != 20) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
            return 0;
        }
        const int target_mask =
            static_cast<const CStackableItem*>(emblem_prototype)
                ->getJewelTargetSocket();
        const uint16_t socket_type =
            next_emblem.sockets[request->target_socket].socket_type;
        if (socket_type == 0u ||
            (target_mask & static_cast<int>(socket_type)) == 0) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
            return 0;
        }

        std::vector<AggregatedEmblemMaterial>::iterator aggregate =
            materials.end();
        for (std::vector<AggregatedEmblemMaterial>::iterator candidate =
                 materials.begin();
             candidate != materials.end();
             ++candidate) {
            if (candidate->material_slot == request->material_slot) {
                aggregate = candidate;
                break;
            }
        }
        if (aggregate == materials.end()) {
            AggregatedEmblemMaterial material_use;
            material_use.material_slot = request->material_slot;
            material_use.emblem_item_id = request->emblem_item_id;
            material_use.count = 1;
            material_use.snapshot = material;
            materials.push_back(material_use);
        } else {
            if (aggregate->emblem_item_id != request->emblem_item_id) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
                return 0;
            }
            ++aggregate->count;
        }
        next_emblem.sockets[request->target_socket].emblem_item_index =
            request->emblem_item_id;
    }

    for (std::vector<AggregatedEmblemMaterial>::const_iterator material =
             materials.begin();
         material != materials.end();
         ++material) {
        if (material->snapshot.get_add_info() < material->count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 4);
            return 0;
        }
    }

    InventoryMemory preview_memory;
    CInventory preview;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenR());
    for (std::vector<AggregatedEmblemMaterial>::const_iterator material =
             materials.begin();
         material != materials.end();
         ++material) {
        if (preview.delete_item(kMainInventory,
                                material->material_slot,
                                material->count,
                                kUseEmblemDeleteReason,
                                false) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 4);
            return 0;
        }
    }

    std::size_t changed_count = 0;
    for (; changed_count < materials.size(); ++changed_count) {
        const AggregatedEmblemMaterial& material = materials[changed_count];
        if (inventory->delete_item(kMainInventory,
                                   material.material_slot,
                                   material.count,
                                   kUseEmblemDeleteReason,
                                   false) != 1) {
            RestoreEmblemMaterials(inventory, materials, changed_count);
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 4);
            return 0;
        }
    }
    if (avatar_manager->setEmblemSocketData(avatar_key, next_emblem) != 1) {
        RestoreEmblemMaterials(inventory, materials, changed_count);
        (void)avatar_manager->setEmblemSocketData(avatar_key, previous_emblem);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_EMBLEM, 17);
        return 0;
    }

    cUserHistoryLog* const history = CUser_history_log(user);
    for (std::vector<AggregatedEmblemMaterial>::const_iterator material =
             materials.begin();
         material != materials.end();
         ++material) {
        const int remaining =
            material->snapshot.get_add_info() - material->count;
        Inven_Item log_item = material->snapshot;
        if (remaining > 0) {
            log_item.set_add_info(remaining);
        }
        if (history) {
            history->ItemDel(kMainInventory,
                             remaining,
                             material->count,
                             log_item,
                             kUseEmblemDeleteReason);
        }
    }

    DB_UpdateAvatarJewelSlot::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        static_cast<unsigned int>(avatar_key),
        &next_emblem);
    user->SaveInventory();
    for (std::vector<AggregatedEmblemMaterial>::const_iterator material =
             materials.begin();
         material != materials.end();
         ++material) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 material->material_slot);
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             avatar_slot);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_EMBLEM);
    response.put_byte(1);
    response.put_byte(static_cast<int>(materials.size()));
    for (std::vector<AggregatedEmblemMaterial>::const_iterator material =
             materials.begin();
         material != materials.end();
         ++material) {
        const Inven_Item remaining_item = inventory->GetInvenSlot(
            kMainInventory, material->material_slot);
        const int remaining =
            remaining_item.item_id_ ==
                    static_cast<int32_t>(material->emblem_item_id)
                ? remaining_item.get_add_info()
                : 0;
        response.put_short(material->material_slot);
        response.put_int(remaining);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233D12
// IDA_NAME: _ZN19Dispatcher_UseJewel4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseJewel::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseJewel::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D1C
// IDA_NAME: _ZN19Dispatcher_UseJewel7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseJewel::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseJewel::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596F18
// IDA_NAME: _ZN21Dispatcher_VerifyGoldC2Ev
// DEMANGLED: Dispatcher_VerifyGold::Dispatcher_VerifyGold(void)
Dispatcher_VerifyGold::Dispatcher_VerifyGold()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240B94
// IDA_NAME: _ZN21Dispatcher_VerifyGoldD2Ev
// DEMANGLED: Dispatcher_VerifyGold::~Dispatcher_VerifyGold()
// IDA_EA: 0x8240BC4
// IDA_NAME: _ZN21Dispatcher_VerifyGoldD0Ev
// DEMANGLED: Dispatcher_VerifyGold::~Dispatcher_VerifyGold()
Dispatcher_VerifyGold::~Dispatcher_VerifyGold() {}

// IDA_EA: 0x8223BC0
// IDA_NAME: _ZN21Dispatcher_VerifyGold12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_VerifyGold::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_VerifyGold::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    unsigned short request_id = 0;
    if (packet.get_short(request_id) != 1) {
        return LineFunc(
            56945,
            "virtual int Dispatcher_VerifyGold::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    // All three proven 140828 senders append a fixed u32 zero and finalize.
    uint32_t native_reserved_zero = 0xffffffffu;
    if (packet.get_int(native_reserved_zero) != 1 ||
        native_reserved_zero != 0u) {
        return LineFunc(
            56946,
            "virtual int Dispatcher_VerifyGold::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    PacketGuard response;
    response.put_header(1, static_cast<int>(ENUM_CMDPACKET_VERIFY_GOLD));
    response.put_byte(1);
    response.put_short(request_id);
    response.put_int(user->getCurCharacMoney());
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x823405A
// IDA_NAME: _ZN21Dispatcher_VerifyGold4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_VerifyGold::read(PacketBuf &,MSG_BASE &)
int Dispatcher_VerifyGold::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234064
// IDA_NAME: _ZN21Dispatcher_VerifyGold7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_VerifyGold::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_VerifyGold::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
