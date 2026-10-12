#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/packet_dispatcher_invest_item_amplify_option.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_InvestItemAmplifyOption::dispatch_sig(CUser*, PacketBuf&)";

int InvestMaterialCount(const stInvestAmplifyOption_t& option) {
    int count = 0;
    // The PVF importer preserves the old four-byte material count at +0x04
    // (value plus reserved) rather than replacing it with a new count field.
    std::memcpy(&count, &option.value, sizeof(count));
    return count;
}

}  // namespace

// IDA_EA: 0x85968F8
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOptionC2Ev
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::Dispatcher_InvestItemAmplifyOption(void)
Dispatcher_InvestItemAmplifyOption::Dispatcher_InvestItemAmplifyOption()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82417C4
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOptionD2Ev
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::~Dispatcher_InvestItemAmplifyOption()
// IDA_EA: 0x82417F4
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOptionD0Ev
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::~Dispatcher_InvestItemAmplifyOption()
Dispatcher_InvestItemAmplifyOption::~Dispatcher_InvestItemAmplifyOption() {}

// IDA_EA: 0x82194F4
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOption12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_InvestItemAmplifyOption::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    if (!user) {
        return LineFunc(52353, kDispatchSignature, 0, 0);
    }
    if (user->get_state() != static_cast<ch_state>(3)) {
        return LineFunc(52354, kDispatchSignature, 0, 0);
    }

    unsigned char invest_type = 0;
    unsigned short target_slot = 0;
    unsigned long target_item_id = 0;
    unsigned short material_slot = 0;
    unsigned long material_item_id = 0;
    if (packet.get_byte(invest_type) != 1) {
        return LineFunc(52361, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(target_slot) != 1) {
        return LineFunc(52362, kDispatchSignature, 0, 0);
    }
    if (packet.get_item_idx(target_item_id) != 1) {
        return LineFunc(52363, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(material_slot) != 1) {
        return LineFunc(52364, kDispatchSignature, 0, 0);
    }
    if (packet.get_item_idx(material_item_id) != 1) {
        return LineFunc(52365, kDispatchSignature, 0, 0);
    }

    char guild_item_name[129] = {};
    unsigned char selected_ability_type = 0;
    if (packet.get_byte(selected_ability_type) != 1) {
        return LineFunc(52375, kDispatchSignature, 0, 0);
    }
    if (selected_ability_type > 4) {
        return LineFunc(52377, kDispatchSignature, 0, 0);
    }
    if (invest_type == 2) {
        unsigned int guild_item_name_length = 0;
        if (packet.get_int(guild_item_name_length) != 1) {
            return LineFunc(52384, kDispatchSignature, 0, 0);
        }
        if (packet.get_str(guild_item_name,
                           sizeof(guild_item_name),
                           guild_item_name_length) != 1) {
            return LineFunc(52385, kDispatchSignature, 0, 0);
        }
    }
    if (invest_type > 2) {
        return LineFunc(52390, kDispatchSignature, 0, 0);
    }

    // The legacy dispatcher framework does not install a C++ exception
    // boundary around reconstructed dispatch_sig implementations.  The
    // original semantic checks below report ENUM_ERROR by throwing; without
    // a local boundary, an ordinary locked item (error 213) aborts the whole
    // channel process and every client sees a network disconnect.
    try {
    CDataManager* const data_manager = G_CDataManager();
    CItemAmplifier* const amplifier = GlobalData::s_itemAmplifier_;
    CInventory* const readable_inventory = user->getCurCharacInvenR();
    CInventory* const writable_inventory = user->getCurCharacInvenW();
    GameWorld* const game_world = G_GameWorld();
    if (!data_manager || !amplifier || !amplifier->isScriptReady() ||
        !readable_inventory || !writable_inventory || !game_world ||
        target_slot == material_slot) {
        throw static_cast<ENUM_ERROR>(17);
    }
    // GetInvenSlot uses this same CheckValidSlot predicate.  Prove the
    // subsequent same-slot update before any destructive mutation instead
    // of treating update_item as a second transaction that might fail.
    if (writable_inventory->CheckValidSlot(1, target_slot) != 1 ||
        writable_inventory->GetInvenRef(1, target_slot) == 0) {
        throw static_cast<ENUM_ERROR>(17);
    }

    WongWork::CItemUpgrade* const item_upgrade =
        data_manager->get_item_upgrade();
    if (invest_type == 2 && !item_upgrade) {
        throw static_cast<ENUM_ERROR>(17);
    }

    CItem* const target_prototype =
        data_manager->find_item(static_cast<int>(target_item_id));
    if (user->CheckItemLock(1, target_slot)) {
        throw static_cast<ENUM_ERROR>(213);
    }
    if (!target_prototype) {
        throw static_cast<ENUM_ERROR>(17);
    }

    Inven_Item target =
        readable_inventory->GetInvenSlot(1, target_slot);
    if (static_cast<unsigned long>(target.item_id_) != target_item_id) {
        throw static_cast<ENUM_ERROR>(17);
    }
    Inven_Item material =
        readable_inventory->GetInvenSlot(1, material_slot);
    if (static_cast<unsigned long>(material.item_id_) != material_item_id) {
        throw static_cast<ENUM_ERROR>(17);
    }

    if (!amplifier->checkInvestableItem(
            target_prototype, target, user->get_charac_level())) {
        throw static_cast<ENUM_ERROR>(8);
    }
    CItem* const material_prototype =
        data_manager->find_item(static_cast<int>(material_item_id));
    if (!material_prototype) {
        throw static_cast<ENUM_ERROR>(17);
    }
    if (!target_prototype->IsEnableWorld(material_prototype) ||
        !material_prototype->IsEnableWorld(target_prototype)) {
        throw static_cast<ENUM_ERROR>(8);
    }

    stInvestAmplifyOption_t material_info = {};
    const ENUM_ITEM_AMPLIFY_INVEST_TYPE typed_invest_type =
        static_cast<ENUM_ITEM_AMPLIFY_INVEST_TYPE>(invest_type);
    if (amplifier->getInvestMaterialInfo(
            typed_invest_type, material_item_id, material_info) != 1) {
        throw static_cast<ENUM_ERROR>(17);
    }
    if (material_info.ability_type == 5 && selected_ability_type == 0) {
        throw static_cast<ENUM_ERROR>(17);
    }
    if (selected_ability_type != 0) {
        if (material_info.ability_type != 5) {
            throw static_cast<ENUM_ERROR>(17);
        }
        material_info.ability_type = selected_ability_type;
    }
    const int material_count = InvestMaterialCount(material_info);
    if (material_info.ability_type == 0 || material_count <= 0 ||
        material.add_info_ < material_count) {
        throw static_cast<ENUM_ERROR>(17);
    }
    if (material_info.ability_type == target.amplify_option_.getAbilityType()) {
        throw static_cast<ENUM_ERROR>(23);
    }
    if (game_world->IsPVPChannel() ||
        game_world->IsPvPSkilTreeChannel()) {
        throw static_cast<ENUM_ERROR>(19);
    }

    int random_upgrade = 0;
    int error = amplifier->enableInvestAmplifyOption(
        target, typed_invest_type);
    if (error != 0) {
        throw static_cast<ENUM_ERROR>(error);
    }
    if (invest_type == 2) {
        error = item_upgrade->enableAmplifyRandomUpgrade(
            user, target, material, random_upgrade);
        if (error == 21) {
            throw static_cast<ENUM_ERROR>(203);
        }
        if (error != 0) {
            throw static_cast<ENUM_ERROR>(error);
        }
    }

    // Complete the fallible target mutations on the local target copy.  The
    // random result above is sampled exactly once; prepare consumes that
    // value and never draws again.  Nothing is published before the material
    // delete.  update_item is an in-memory same-slot assignment whose only
    // failure condition (slot validity) was proved above.
    const int old_upgrade = target.GetUpgrade();
    // The legacy helper mutates the local inventory copy and has pointer/void
    // style return semantics; the official CMD205 caller does not interpret
    // its return value as a success flag.
    (void)amplifier->investAmplifyOption(
        target_prototype, target, material_info);
    if (invest_type == 2) {
        error = item_upgrade->prepareAmplifyRandomUpgrade(
            target, random_upgrade);
        if (error != 0) {
            throw static_cast<ENUM_ERROR>(error);
        }
    }

    if (writable_inventory->delete_item(
            static_cast<INVEN_TYPE>(1),
            material_slot,
            material_count,
            static_cast<eItemDelReason>(3),
            true) != 1) {
        throw static_cast<ENUM_ERROR>(22);
    }
    user->history_log().apply_Item(
        material.item_id_, target.item_id_, static_cast<eApplyItemReason>(2));

    writable_inventory->update_item(
        static_cast<INVEN_TYPE>(1), target_slot, target);
    if (invest_type == 2) {
        item_upgrade->commitAmplifyRandomUpgrade(
            user, target, old_upgrade, guild_item_name);
    }
    const Inven_Item material_after =
        readable_inventory->GetInvenSlot(1, material_slot);

    PacketGuard result;
    result.put_header(1, ENUM_CMDPACKET_INVEST_ITEM_AMPLIFY_OPTION);
    result.put_byte(1);
    result.put_byte(invest_type);
    result.put_short(material_slot);
    result.put_int(material_after.add_info_);
    result.put_short(target_slot);
    result.put_byte(target.amplify_option_.getAbilityType());
    result.put_short(target.amplify_option_.getAbilityValue());
    if (invest_type == 2) {
        result.put_byte(target.GetUpgrade());
    }
    result.finalize(true);
    user->Send(result);
    return 0;
    } catch (ENUM_ERROR error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INVEST_ITEM_AMPLIFY_OPTION,
            static_cast<unsigned char>(error));
        return 0;
    }
}

// IDA_EA: 0x8233D3A
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::read(PacketBuf &,MSG_BASE &)
int Dispatcher_InvestItemAmplifyOption::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D44
// IDA_NAME: _ZN34Dispatcher_InvestItemAmplifyOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_InvestItemAmplifyOption::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_InvestItemAmplifyOption::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
