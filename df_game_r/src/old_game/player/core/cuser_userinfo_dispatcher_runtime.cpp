#include "old_game/player/core/userinfo_dispatcher_reads.h"

#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory_params.h"
#include "old_game/player/core/userinfo_messages.h"
#include "old_data/PVF/data/datamanager/image_communication_info.h"
#include "old_data/db/inventory/db_update_inventory_capacity.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/social/party/party.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/secu_protection_field_abi.h"

typedef char CMD_GET_USERINFO_Must_Be_8[
    (ENUM_CMDPACKET_GET_USERINFO == 8) ? 1 : -1];
// CLIENT_PROTOCOL_EVIDENCE: direct native enum values; no protocol mapping.
typedef char CMD_REPAIR_EQUIPMENT_Must_Be_23[
    (ENUM_CMDPACKET_REPAIR_EQUIPMENT == 23) ? 1 : -1];
typedef char CMD_DECREASE_DURABILITY_Must_Be_48[
    (ENUM_CMDPACKET_DECREASE_DURABILITY == 48) ? 1 : -1];
typedef char CMD_UPGRADE_CARGO_Must_Be_405[
    (ENUM_CMDPACKET_UPGRADE_CARGO == 405) ? 1 : -1];
typedef char CMD_UPGRADE_INVENTORY_Must_Be_428[
    (ENUM_CMDPACKET_UPGRADE_INVENTORY == 428) ? 1 : -1];
typedef char CMD_IMAGE_COMMUNICATION_EQUIPMENT_USE_Must_Be_473[
    (ENUM_CMDPACKET_IMAGE_COMMUNICATION_EQUIPMENT_USE == 473) ? 1 : -1];

// IDA_EA: 0x81C3D46
// IDA_NAME: _ZN22DisPatcher_GetUserInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_GetUserInfo::read(PacketBuf &,MSG_BASE &)
int DisPatcher_GetUserInfo::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USERINFO_REQ& request =
        reinterpret_cast<MSG_USERINFO_REQ&>(message);
    if (packet.get_short(request.target_uid) != 1) {
        return LineFunc(3931,
                        "virtual int DisPatcher_GetUserInfo::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.detail_type) == 1) {
        return 0;
    }
    return LineFunc(3933,
                    "virtual int DisPatcher_GetUserInfo::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C3DD8
// IDA_NAME: _ZN22DisPatcher_GetUserInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_GetUserInfo::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_GetUserInfo::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)param;

    const int result = check_error(user, message);
    if (result > 0) {
        return -1;
    }
    if (result >= 0) {
        const MSG_USERINFO_REQ& request =
            reinterpret_cast<const MSG_USERINFO_REQ&>(message);
        G_GameWorld()->get_user_info(user, request.target_uid,
                                     request.detail_type);
        return result;
    }

    return LineFunc(
        3949,
        "virtual int DisPatcher_GetUserInfo::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x81C3E7C
// IDA_NAME: _ZN22DisPatcher_GetUserInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_GetUserInfo::check_error(CUser *,MSG_BASE &)
int DisPatcher_GetUserInfo::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    return user->get_state() ? 0 : -1;
}

// IDA_EA: 0x823232C
// IDA_NAME: _ZN22DisPatcher_GetUserInfoD2Ev
// DEMANGLED: DisPatcher_GetUserInfo::~DisPatcher_GetUserInfo()
// IDA_EA: 0x823235C
// IDA_NAME: _ZN22DisPatcher_GetUserInfoD0Ev
// DEMANGLED: DisPatcher_GetUserInfo::~DisPatcher_GetUserInfo()
DisPatcher_GetUserInfo::~DisPatcher_GetUserInfo() {}

// IDA_EA: 0x85951A4
// IDA_NAME: _ZN22DisPatcher_GetUserInfoC2Ev
// DEMANGLED: DisPatcher_GetUserInfo::DisPatcher_GetUserInfo(void)
DisPatcher_GetUserInfo::DisPatcher_GetUserInfo()
    : IPacketDispatcher<MSG_USERINFO_REQ,
                        stOnGetUserInfoParam,
                        static_cast<ch_state>(1)>() {}

// IDA_EA: 0x81C0DFA
// IDA_NAME: _ZN19DisPatcher_UseEquip4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_UseEquip::read(PacketBuf &,MSG_BASE &)
int DisPatcher_UseEquip::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DECREASE_DURABILITY& request =
        static_cast<MSG_DECREASE_DURABILITY&>(message);
    if (packet.get_byte(request.equip_slot) == 1) {
        return 0;
    }
    return LineFunc(2512,
                    "virtual int DisPatcher_UseEquip::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C0E4A
// IDA_NAME: _ZN19DisPatcher_UseEquip7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseEquip::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_UseEquip::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    const int result = check_error(user, message);
    if (result > 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_DECREASE_DURABILITY,
            static_cast<unsigned char>(result));
        return -1;
    }

    const MSG_DECREASE_DURABILITY& request =
        static_cast<const MSG_DECREASE_DURABILITY&>(message);
    const int slot = static_cast<int>(request.equip_slot);
    CInventory* inventory = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    stOnUseEquipParam& use_param =
        reinterpret_cast<stOnUseEquipParam&>(param);
    use_param.result = inventory->use_equip(slot);
    use_param.slot = static_cast<unsigned char>(slot);
    return result;
}

// IDA_EA: 0x81C0EE2
// IDA_NAME: _ZN19DisPatcher_UseEquip11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_UseEquip::check_error(CUser *,MSG_BASE &)
int DisPatcher_UseEquip::check_error(CUser* user, MSG_BASE& message) {
    (void)message;

    const int state = user->get_state();
    if (state != 5 && state != 7 && state != 8 && state != 10 &&
        state != 12) {
        return 19;
    }

    CParty* party = user->GetParty();
    if (!party) {
        return 0;
    }
    if (party->battle_field().instance_dungeon_mode() == 1) {
        return 19;
    }
    const CDungeon* const dungeon = party->battle_field().get_dungeon();
    if (dungeon && dungeon->get_blood_dungeon_type() != 0) {
        return 19;
    }
    return 0;
}

// IDA_EA: 0x81C0FAC
// IDA_NAME: _ZN19DisPatcher_UseEquip4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_UseEquip::send(CUser *,ParamBase &)
void DisPatcher_UseEquip::send(CUser* user, ParamBase& param) {
    stOnUseEquipParam& use_param =
        reinterpret_cast<stOnUseEquipParam&>(param);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_DECREASE_DURABILITY);
    if (use_param.result) {
        packet.put_byte(0);
        packet.put_byte(use_param.result);
    } else {
        packet.put_byte(1);
        packet.put_byte(use_param.slot);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8232110
// IDA_NAME: _ZN19DisPatcher_UseEquipD2Ev
// DEMANGLED: DisPatcher_UseEquip::~DisPatcher_UseEquip()
// IDA_EA: 0x8232140
// IDA_NAME: _ZN19DisPatcher_UseEquipD0Ev
// DEMANGLED: DisPatcher_UseEquip::~DisPatcher_UseEquip()
DisPatcher_UseEquip::~DisPatcher_UseEquip() {}

// IDA_EA: 0x8595118
// IDA_NAME: _ZN19DisPatcher_UseEquipC2Ev
// DEMANGLED: DisPatcher_UseEquip::DisPatcher_UseEquip(void)
DisPatcher_UseEquip::DisPatcher_UseEquip()
    : IPacketDispatcher<MSG_DECREASE_DURABILITY,
                        stOnUseEquipParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81C5FA6
// IDA_NAME: _ZN22DisPatcher_RepairEquip4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_RepairEquip::read(PacketBuf &,MSG_BASE &)
int DisPatcher_RepairEquip::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_REPAIR_EQUIP& request = static_cast<MSG_REPAIR_EQUIP&>(message);
    unsigned char inven_type = 0;
    unsigned short slot = 0;
    unsigned short word_10 = 0;
    if (packet.get_byte(inven_type) != 1) {
        return LineFunc(5350,
                        "virtual int DisPatcher_RepairEquip::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(slot) != 1) {
        return LineFunc(5351,
                        "virtual int DisPatcher_RepairEquip::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(word_10) == 1) {
        request.inven_type = inven_type;
        request.slot = slot;
        request.word_10 = word_10;
        return 0;
    }
    return LineFunc(5353,
                    "virtual int DisPatcher_RepairEquip::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C6082
// IDA_NAME: _ZN22DisPatcher_RepairEquip7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_RepairEquip::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_RepairEquip::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    const int result = check_error(user, message);
    if (result > 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REPAIR_EQUIPMENT,
            static_cast<unsigned char>(result));
        return -1;
    }
    if (result < 0) {
        return LineFunc(
            5376,
            "virtual int DisPatcher_RepairEquip::process(CUser*, MSG_BASE&, ParamBase&)",
            0,
            user->get_acc_id());
    }

    const MSG_REPAIR_EQUIP& request =
        static_cast<const MSG_REPAIR_EQUIP&>(message);
    stRepairEquipParam& repair_param =
        reinterpret_cast<stRepairEquipParam&>(param);
    repair_param.inven_type = request.inven_type;
    repair_param.slot = request.slot;
    repair_param.repair_cost = 0;
    repair_param.result = G_Store()->repair_equip(
        user,
        static_cast<char>(request.inven_type),
        static_cast<short>(request.slot),
        request.word_10,
        repair_param.repair_cost);
    if (!repair_param.result) {
        repair_param.inven_type = request.inven_type;
        repair_param.slot = request.slot;
    }
    return 0;
}

// IDA_EA: 0x81C619C
// IDA_NAME: _ZN22DisPatcher_RepairEquip11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_RepairEquip::check_error(CUser *,MSG_BASE &)
int DisPatcher_RepairEquip::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    const unsigned int state = user->get_state();
    if (state <= 12u && ((1u << state) & 0x15E8u) != 0) {
        return 0;
    }
    return 19;
}

// IDA_EA: 0x81C61EA
// IDA_NAME: _ZN22DisPatcher_RepairEquip4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_RepairEquip::send(CUser *,ParamBase &)
void DisPatcher_RepairEquip::send(CUser* user, ParamBase& param) {
    stRepairEquipParam& repair_param =
        reinterpret_cast<stRepairEquipParam&>(param);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REPAIR_EQUIPMENT);
    if (repair_param.result) {
        packet.put_byte(0);
        packet.put_byte(repair_param.result);
    } else {
        packet.put_byte(1);
        CInventory* inventory = static_cast<CInventory*>(
            user->getCurCharacInvenR());
        packet.put_int(inventory->get_money());
        packet.put_byte(repair_param.inven_type);
        packet.put_short(repair_param.slot);
        packet.put_short(repair_param.repair_cost);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8232548
// IDA_NAME: _ZN22DisPatcher_RepairEquipD2Ev
// DEMANGLED: DisPatcher_RepairEquip::~DisPatcher_RepairEquip()
// IDA_EA: 0x8232578
// IDA_NAME: _ZN22DisPatcher_RepairEquipD0Ev
// DEMANGLED: DisPatcher_RepairEquip::~DisPatcher_RepairEquip()
DisPatcher_RepairEquip::~DisPatcher_RepairEquip() {}

// IDA_EA: 0x8595230
// IDA_NAME: _ZN22DisPatcher_RepairEquipC2Ev
// DEMANGLED: DisPatcher_RepairEquip::DisPatcher_RepairEquip(void)
DisPatcher_RepairEquip::DisPatcher_RepairEquip()
    : IPacketDispatcher<MSG_REPAIR_EQUIP,
                        stRepairEquipParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81E21CC
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUse4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ImageCommunicationEquipmentUse::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81E21D6
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUse7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ImageCommunicationEquipmentUse::process(CUser* user,
                                                       MSG_BASE& message,
                                                       ParamBase& param) {
    stImageCommunicationEquipmentUse& image_param =
        reinterpret_cast<stImageCommunicationEquipmentUse&>(param);
    image_param.result = check_error(user, message);
    if (!image_param.result) {
        ImageCommunicationData* data =
            G_CDataManager()->GetImageCommnicationData();
        CInventory* inventory = static_cast<CInventory*>(
            user->getCurCharacInvenW());
        inventory->use_money(data->charge,
                             static_cast<eMoneySubReason>(48),
                             true);
        image_param.money_cost =
            G_CDataManager()->GetImageCommnicationData()->charge;
        image_param.cur_money = user->getCurCharacMoney();
    }
    return 0;
}

// IDA_EA: 0x81E2280
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUse11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::check_error(CUser *,MSG_BASE &)
int Dispatcher_ImageCommunicationEquipmentUse::check_error(
    CUser* user,
    MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return -2;
    }
    const int charge = G_CDataManager()->GetImageCommnicationData()->charge;
    if (charge <= user->getCurCharacMoney()) {
        return 0;
    }
    return 10;
}

// IDA_EA: 0x81E22E6
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUse4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::send(CUser *,ParamBase &)
void Dispatcher_ImageCommunicationEquipmentUse::send(CUser* user,
                                                     ParamBase& param) {
    stImageCommunicationEquipmentUse& image_param =
        reinterpret_cast<stImageCommunicationEquipmentUse&>(param);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_IMAGE_COMMUNICATION_EQUIPMENT_USE);
    if (image_param.result) {
        packet.put_byte(0);
        packet.put_byte(image_param.result);
    } else {
        packet.put_byte(1);
        packet.put_int(image_param.money_cost);
        packet.put_int(image_param.cur_money);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8244182
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUseD2Ev
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::~Dispatcher_ImageCommunicationEquipmentUse()
// IDA_EA: 0x82441B2
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUseD0Ev
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::~Dispatcher_ImageCommunicationEquipmentUse()
Dispatcher_ImageCommunicationEquipmentUse::~Dispatcher_ImageCommunicationEquipmentUse() {}

// IDA_EA: 0x8597C92
// IDA_NAME: _ZN41Dispatcher_ImageCommunicationEquipmentUseC2Ev
// DEMANGLED: Dispatcher_ImageCommunicationEquipmentUse::Dispatcher_ImageCommunicationEquipmentUse(void)
Dispatcher_ImageCommunicationEquipmentUse::Dispatcher_ImageCommunicationEquipmentUse()
    : IPacketDispatcher<MSG_BASE,
                        stImageCommunicationEquipmentUse,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8693198
// IDA_NAME: _ZN5CUser23SetEventCreateDnfRewardEb
// DEMANGLED: CUser::SetEventCreateDnfReward(bool)
CUser* CUser::SetEventCreateDnfReward(bool received) {
    event_create_dnf_reward_ = static_cast<unsigned char>(received);
    return this;
}

// IDA_EA: 0x86931B4
// IDA_NAME: _ZN5CUser23GetEventCreateDnfRewardEv
// DEMANGLED: CUser::GetEventCreateDnfReward(void)
int CUser::GetEventCreateDnfReward() {
    return event_create_dnf_reward_;
}

// IDA_EA: 0x822A0A4
// IDA_NAME: _ZN23Dispatcher_UpgradeCargo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeCargo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeCargo::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(
            59352,
            "virtual int Dispatcher_UpgradeCargo::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int secu_result =
        (::GlobalData::s_pSecuProtectionField)->Check(
            user,
            static_cast<SECURITY_PROTCTION>(29));
    if (secu_result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_UPGRADE_CARGO,
            static_cast<unsigned char>(secu_result));
        return 0;
    }

    unsigned short slot = 0;
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            59362,
            "virtual int Dispatcher_UpgradeCargo::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CInventory* inventory_r = static_cast<CInventory*>(
        user->getCurCharacInvenR());
    Inven_Item item = inventory_r->GetInvenSlot(1, slot);
    if (item.get_add_info() <= 0 ||
        user->verifyStackableItemActionPlace(
            static_cast<StackableAction::Type>(43),
            static_cast<unsigned int>(item.getKey())) != true) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CARGO, 17);
        return 0;
    }

    const int expand_count = 16;
    CCargo* cargo_r = user->getCurCharacCargoR();
    const int next_capacity = expand_count + cargo_r->GetCapacity();
    if (cargo_r->isUpgradable(next_capacity) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CARGO, 19);
        return 0;
    }

    CInventory* inventory_w = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    if (inventory_w->delete_item(static_cast<INVEN_TYPE>(1),
                                  slot,
                                  1,
                                  static_cast<eItemDelReason>(3),
                                  true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CARGO, 17);
        return 0;
    }

    PacketGuard out_packet;
    out_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 14);
    out_packet.put_byte(0);
    out_packet.put_short(1);
    inventory_r->MakeItemPacket(static_cast<INVEN_TYPE>(1), slot, out_packet);
    out_packet.finalize(true);
    user->Send(out_packet);

    cargo_r = user->getCurCharacCargoR();
    const int upgraded_capacity = expand_count + cargo_r->GetCapacity();
    CCargo* cargo_w = static_cast<CCargo*>(
        user->getCurCharacCargoW());
    if (cargo_w->UpgradeCargo(upgraded_capacity) == 1) {
        out_packet.clear();
        out_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 66);
        out_packet.put_short(1);
        cargo_r = user->getCurCharacCargoR();
        out_packet.put_short(cargo_r->GetCapacity());
        out_packet.finalize(true);
        user->Send(out_packet);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_CARGO, 19);
    }

    return 0;
}

// IDA_EA: 0x8234366
// IDA_NAME: _ZN23Dispatcher_UpgradeCargo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeCargo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeCargo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234370
// IDA_NAME: _ZN23Dispatcher_UpgradeCargo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeCargo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeCargo::process(CUser* user, MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x822A4BC
// IDA_NAME: _ZN27Dispatcher_UpgradeInventory12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeInventory::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeInventory::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(
            59432,
            "virtual int Dispatcher_UpgradeInventory::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int secu_result =
        (::GlobalData::s_pSecuProtectionField)->Check(
            user,
            static_cast<SECURITY_PROTCTION>(29));
    if (secu_result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_UPGRADE_INVENTORY,
            static_cast<unsigned char>(secu_result));
        return 0;
    }

    unsigned short slot = 0;
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            59442,
            "virtual int Dispatcher_UpgradeInventory::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CInventory* inventory_r = static_cast<CInventory*>(
        user->getCurCharacInvenR());
    Inven_Item item = inventory_r->GetInvenSlot(1, slot);
    const int item_id = item.getKey();
    if (item.get_add_info() <= 0 ||
        (item_id != 2660298 && item_id != 8251)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_INVENTORY, 17);
        return 0;
    }

    if (inventory_r->get_inventory_capacity() > 15) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_INVENTORY, 19);
        return 0;
    }

    CInventory* inventory_w = static_cast<CInventory*>(
        user->getCurCharacInvenW());
    if (inventory_w->delete_item(static_cast<INVEN_TYPE>(1),
                                  slot,
                                  1,
                                  static_cast<eItemDelReason>(3),
                                  true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_INVENTORY, 17);
        return 0;
    }

    PacketGuard out_packet;
    out_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 14);
    out_packet.put_byte(0);
    out_packet.put_short(1);
    inventory_r->MakeItemPacket(static_cast<INVEN_TYPE>(1), slot, out_packet);
    out_packet.finalize(true);
    user->Send(out_packet);

    const int old_capacity = inventory_r->get_inventory_capacity();
    const int new_capacity = old_capacity + 8;
    CUser_history_log(user)->ExpandInventory(old_capacity, new_capacity, 1);
    inventory_r->set_inventory_capacity(new_capacity);

    const unsigned char cur_capacity =
        static_cast<unsigned char>(
            inventory_r->get_inventory_capacity());
    const unsigned int charac_no =
        static_cast<unsigned int>(user->getCurCharacNo());
    DB_UpdateInventoryCapacity::makeRequest(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        user->GetUID(),
        charac_no,
        cur_capacity);

    out_packet.clear();
    out_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 66);
    out_packet.put_short(11);
    out_packet.put_short(inventory_r->get_inventory_capacity());
    out_packet.finalize(true);
    user->Send(out_packet);
    return 0;
}

// IDA_EA: 0x823437A
// IDA_NAME: _ZN27Dispatcher_UpgradeInventory4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeInventory::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeInventory::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234384
// IDA_NAME: _ZN27Dispatcher_UpgradeInventory7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeInventory::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeInventory::process(CUser* user, MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85977FA
// IDA_NAME: _ZN23Dispatcher_UpgradeCargoC2Ev
// DEMANGLED: Dispatcher_UpgradeCargo::Dispatcher_UpgradeCargo(void)
Dispatcher_UpgradeCargo::Dispatcher_UpgradeCargo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823FFB2
// IDA_NAME: _ZN23Dispatcher_UpgradeCargoD2Ev
// DEMANGLED: Dispatcher_UpgradeCargo::~Dispatcher_UpgradeCargo()
// IDA_EA: 0x823FFE2
// IDA_NAME: _ZN23Dispatcher_UpgradeCargoD0Ev
// DEMANGLED: Dispatcher_UpgradeCargo::~Dispatcher_UpgradeCargo()
Dispatcher_UpgradeCargo::~Dispatcher_UpgradeCargo() {}

// IDA_EA: 0x85978DA
// IDA_NAME: _ZN27Dispatcher_UpgradeInventoryC2Ev
// DEMANGLED: Dispatcher_UpgradeInventory::Dispatcher_UpgradeInventory(void)
Dispatcher_UpgradeInventory::Dispatcher_UpgradeInventory()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823FF64
// IDA_NAME: _ZN27Dispatcher_UpgradeInventoryD2Ev
// DEMANGLED: Dispatcher_UpgradeInventory::~Dispatcher_UpgradeInventory()
// IDA_EA: 0x823FF94
// IDA_NAME: _ZN27Dispatcher_UpgradeInventoryD0Ev
// DEMANGLED: Dispatcher_UpgradeInventory::~Dispatcher_UpgradeInventory()
Dispatcher_UpgradeInventory::~Dispatcher_UpgradeInventory() {}
