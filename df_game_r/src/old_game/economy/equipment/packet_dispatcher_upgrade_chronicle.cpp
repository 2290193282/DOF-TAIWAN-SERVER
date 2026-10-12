#include "old_game/economy/equipment/packet_dispatcher_upgrade_chronicle.h"
#include "infra/network/packet/PacketEnum.h"

#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_UpgradeChronicle::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8223330
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicle12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeChronicle::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeChronicle::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    short material_slot = 0;
    uint32_t material_item_id = 0;
    short target_slot = 0;
    uint32_t target_item_id = 0;
    uint8_t grade_sample = 0;
    if (packet.get_short(material_slot) != 1) {
        return LineFunc(56740, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(material_item_id) != 1) {
        return LineFunc(56741, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(target_slot) != 1) {
        return LineFunc(56742, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(target_item_id) != 1) {
        return LineFunc(56743, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(grade_sample) != 1) {
        return LineFunc(56744, kDispatchSignature, 0, 0);
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(26));
    if (protection_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE,
                                 static_cast<uint8_t>(protection_error));
        return 0;
    }
    if (user->CheckItemLock(1, target_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 213);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const Inven_Item material = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), material_slot);
    const Inven_Item target = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), target_slot);
    CStackableItem* const material_prototype =
        static_cast<CStackableItem*>(G_CDataManager()->find_item(
            material.item_id_));
    CItem* const target_item = G_CDataManager()->find_item(target.item_id_);
    if (!material_prototype || !target_item) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 1);
        return 0;
    }
    if (material.item_id_ == 0 || target.item_id_ == 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 4);
        return 0;
    }
    if (static_cast<uint32_t>(material.item_id_) != material_item_id ||
        static_cast<uint32_t>(target.item_id_) != target_item_id) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 1);
        return 0;
    }
    if (material_prototype->IsEmancipate() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 17);
        return 0;
    }
    if (target_item->is_stackable()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 1);
        return 0;
    }

    CEquipItem* const target_prototype = static_cast<CEquipItem*>(target_item);
    if (target_prototype->IsEmancipate() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 17);
        return 0;
    }
    int equipment_start = 0;
    int equipment_end = 0;
    inventory->StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(0),
                           equipment_start,
                           equipment_end);
    if (target_slot >= equipment_start && target_slot <= equipment_end) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 19);
        return 0;
    }
    if (target.avatar_select_ < target_prototype->get_endurance()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 7);
        return 0;
    }
    if (target.amplify_option_.isIdentified() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 174);
        return 0;
    }

    std::vector<int> changed_slots;
    const int upgrade_error = G_CDataManager()->get_item_upgrade()
        ->ProcUpgradeChronicle(user,
                               material_slot,
                               material_item_id,
                               target_slot,
                               target_item_id,
                               grade_sample,
                               changed_slots);
    if (upgrade_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CHRONICLE, 174);
        return 0;
    }
    if (changed_slots.size() != 3u * (changed_slots.size() / 3u)) {
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_UPGRADE_CHRONICLE);
    response.put_byte(1);
    response.put_byte(static_cast<int>(changed_slots.size() / 3u + 1u));
    // 0x8223330 reloads this slot before writing the fixed first record.  It
    // does not consume the reloaded value; only packet ordering is observable.
    inventory->GetInvenSlot(static_cast<INVEN_TYPE>(1), material_slot);
    response.put_byte(0);
    response.put_short(material_slot);
    response.put_int(1);
    for (std::vector<int>::const_iterator it = changed_slots.begin();
         it != changed_slots.end();) {
        response.put_byte(*it++);
        response.put_short(*it++);
        response.put_int(*it++);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8234032
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicle4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeChronicle::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeChronicle::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823403C
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicle7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeChronicle::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeChronicle::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596EC4
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicleC2Ev
// DEMANGLED: Dispatcher_UpgradeChronicle::Dispatcher_UpgradeChronicle(void)
Dispatcher_UpgradeChronicle::Dispatcher_UpgradeChronicle()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240C30
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicleD2Ev
// DEMANGLED: Dispatcher_UpgradeChronicle::~Dispatcher_UpgradeChronicle()
// IDA_EA: 0x8240C60
// IDA_NAME: _ZN27Dispatcher_UpgradeChronicleD0Ev
// DEMANGLED: Dispatcher_UpgradeChronicle::~Dispatcher_UpgradeChronicle()
Dispatcher_UpgradeChronicle::~Dispatcher_UpgradeChronicle() {}
