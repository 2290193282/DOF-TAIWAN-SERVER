#include "old_data/Login/packet_dispatch/avatar_option_change_dispatcher.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kAvatarOptionChangeReadSignature[] =
    "virtual int Dispatcher_AvatarOptionChange::read(PacketBuf&, MSG_BASE&)";
const char kAvatarOptionChangeProcessSignature[] =
    "virtual int Dispatcher_AvatarOptionChange::process(CUser*, MSG_BASE&, ParamBase&)";

const INVEN_TYPE kAvatarOptionChangeMaterialInventory =
    static_cast<INVEN_TYPE>(1);
const INVEN_TYPE kAvatarOptionChangeAvatarInventory =
    static_cast<INVEN_TYPE>(2);
const eItemDelReason kAvatarOptionChangeDeleteReason =
    static_cast<eItemDelReason>(47);
// 0828 keeps the client request on CMD457, while the inventory-to-DB queue
// owner for SIG_AVATAR_OPTION_CHANGE is signal 707.  Signal 195 belongs to
// DB_ReqChangeCharacName in the 0828 DBTask table and must never be reused.
const int kAvatarOptionChangeDbSignal = 707;

}  // namespace

// IDA_EA: 0x8686D0E
// IDA_NAME: _ZN15cUserHistoryLog21UseAvatarOptionChangeEiiii
// DEMANGLED: cUserHistoryLog::UseAvatarOptionChange(int,int,int,int)
CUser* cUserHistoryLog::UseAvatarOptionChange(int avatar_ui_id,
                                               int avatar_item_id,
                                               int old_option,
                                               int new_option) {
    if (owner_) {
        return reinterpret_cast<CUser*>(static_cast<uintptr_t>(
            owner_->LogHistory("UseAvatarOptionChange,%d,%d,%d,%d",
                               avatar_ui_id, avatar_item_id, old_option,
                               new_option)));
    }
    return owner_;
}

// IDA_EA: 0x824BC32
// IDA_NAME: _ZN20stChangeAvatarOptionC2Ev
// DEMANGLED: stChangeAvatarOption::stChangeAvatarOption(void)
stChangeAvatarOption::stChangeAvatarOption() : ParamBase() {}

// IDA_EA: 0x824BC4E
// IDA_NAME: _ZN20stChangeAvatarOptionD2Ev
// DEMANGLED: stChangeAvatarOption::~stChangeAvatarOption()
// IDA_EA: 0x824BC7E
// IDA_NAME: _ZN20stChangeAvatarOptionD0Ev
// DEMANGLED: stChangeAvatarOption::~stChangeAvatarOption()
stChangeAvatarOption::~stChangeAvatarOption() {}

// The original state-3 message/template specialization is emitted once here.
// No root PacketDispatcher registration is added.
// IDA_EA: 0x824BC9C
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_CHANGE_AVATAR_OPTION20stChangeAvatarOptionL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,stChangeAvatarOption,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824BDF8
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_CHANGE_AVATAR_OPTION20stChangeAvatarOptionL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,stChangeAvatarOption,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824BE02
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_CHANGE_AVATAR_OPTION20stChangeAvatarOptionL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,stChangeAvatarOption,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B7F4 / 0x825B824 / 0x825D5FA / 0x85983CA
// IDA_NAME: ...D2Ev / ...D0Ev / ...4sendEP5CUserR9ParamBase / ...C2Ev
template class IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,
                                 stChangeAvatarOption,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x859591C
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChangeC2Ev
// DEMANGLED: Dispatcher_AvatarOptionChange::Dispatcher_AvatarOptionChange(void)
Dispatcher_AvatarOptionChange::Dispatcher_AvatarOptionChange()
    : IPacketDispatcher<MSG_CHANGE_AVATAR_OPTION,
                        stChangeAvatarOption,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8244308
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChangeD2Ev
// DEMANGLED: Dispatcher_AvatarOptionChange::~Dispatcher_AvatarOptionChange()
// IDA_EA: 0x8244338
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChangeD0Ev
// DEMANGLED: Dispatcher_AvatarOptionChange::~Dispatcher_AvatarOptionChange()
Dispatcher_AvatarOptionChange::~Dispatcher_AvatarOptionChange() {}

// IDA_EA: 0x81E04F2
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChange4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AvatarOptionChange::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AvatarOptionChange::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHANGE_AVATAR_OPTION& request =
        static_cast<MSG_CHANGE_AVATAR_OPTION&>(message);
    if (packet.get_short(request.material_slot) != 1)
        return LineFunc(21376, kAvatarOptionChangeReadSignature, 0, 0);
    if (request.material_slot >=
        oldsrv::config::kMainInventoryItemCount)
        return LineFunc(21379, kAvatarOptionChangeReadSignature, 0, 0);
    if (packet.get_int(request.material_item_id) != 1)
        return LineFunc(21382, kAvatarOptionChangeReadSignature, 0, 0);
    if (static_cast<int32_t>(request.material_item_id) < 0)
        return LineFunc(21385, kAvatarOptionChangeReadSignature, 0, 0);
    if (packet.get_short(request.avatar_slot) != 1)
        return LineFunc(21388, kAvatarOptionChangeReadSignature, 0, 0);
    if (request.avatar_slot > 0x68u)
        return LineFunc(21392, kAvatarOptionChangeReadSignature, 0, 0);
    if (packet.get_int(request.avatar_item_id) != 1)
        return LineFunc(21395, kAvatarOptionChangeReadSignature, 0, 0);
    if (static_cast<int32_t>(request.avatar_item_id) < 0)
        return LineFunc(21398, kAvatarOptionChangeReadSignature, 0, 0);
    if (packet.get_byte(request.option) != 1)
        return LineFunc(21401, kAvatarOptionChangeReadSignature, 0, 0);
    if (static_cast<int8_t>(request.option) < 0)
        return LineFunc(21404, kAvatarOptionChangeReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81E0A40
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChange11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_AvatarOptionChange::check_error(CUser *,MSG_BASE &)
int Dispatcher_AvatarOptionChange::check_error(CUser* user,
                                               MSG_BASE& message) {
    if (!user || user->get_state() <= 2) return -1;

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_error) return protection_error;

    const MSG_CHANGE_AVATAR_OPTION& request =
        static_cast<const MSG_CHANGE_AVATAR_OPTION&>(message);
    const CInventory* const inventory = user->getCurCharacInvenR();
    const Inven_Item material = inventory->GetInvenSlot(
        kAvatarOptionChangeMaterialInventory, request.material_slot);
    if (material.item_id_ != static_cast<int32_t>(request.material_item_id))
        return 17;
    if (material.add_info_ <= 0) return 22;

    CItem* const material_prototype =
        G_CDataManager()->find_item(material.item_id_);
    if (!material_prototype || !material_prototype->is_stackable()) return 17;
    const CStackableItem* const material_item =
        static_cast<const CStackableItem*>(material_prototype);
    if (material_item->IsAvatarOptionChangeItem() != 1) return 17;

    const Inven_Item avatar = inventory->GetInvenSlot(
        kAvatarOptionChangeAvatarInventory, request.avatar_slot);
    if (avatar.item_id_ != static_cast<int32_t>(request.avatar_item_id))
        return 17;
    CItem* const avatar_prototype = G_CDataManager()->find_item(avatar.item_id_);
    if (!avatar_prototype || avatar_prototype->is_stackable() ||
        !avatar_prototype->vtable_->is_avatar_item(avatar_prototype))
        return 17;
    if (user->CheckItemLock(kAvatarOptionChangeAvatarInventory,
                            request.avatar_slot))
        return 213;

    const AvatarOptionChangeType& rule =
        material_item->avatar_option_change_type();
    if (material_item->usable_avatar_item_ids().empty()) {
        // The 0627 dispatcher used the then-current terminal enum values
        // directly (job 11 and equipment type 26) as the PVF `[all]`
        // sentinels.  0828 added female swordman and weapon-avatar entries,
        // moving those sentinels to the configured enum extents.  Keep the
        // runtime check owned by the same enum definitions as the importer.
        if (avatar_prototype->check_job_type(rule.option_0) != 1 &&
            rule.option_0 != oldsrv::config::kCharacterJobCommon)
            return 254;
        if (avatar_prototype->vtable_->get_item_type(avatar_prototype) !=
                rule.option_1 &&
            rule.option_1 != oldsrv::config::kEquipmentTypeInvalid)
            return 253;
        if (avatar_prototype->get_grade() != rule.option_2 && rule.option_2)
            return 252;
    } else if (material_item->usable_avatar_item_ids().find(avatar.item_id_) ==
               material_item->usable_avatar_item_ids().end()) {
        return 255;
    }

    if (!material_item->prohibited_avatar_options().empty() &&
        material_item->prohibited_avatar_options().find(request.option) !=
            material_item->prohibited_avatar_options().end())
        return 251;
    if (static_cast<const CEquipItem*>(avatar_prototype)->IsAvatarOptionValid(
            static_cast<char>(request.option)))
        return 0;
    return 250;
}

// IDA_EA: 0x81E0774
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChange7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AvatarOptionChange::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AvatarOptionChange::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    stChangeAvatarOption& result = static_cast<stChangeAvatarOption&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) return 0;
    if (result.result < 0)
        return LineFunc(21419, kAvatarOptionChangeProcessSignature,
                        result.result, 0);

    const MSG_CHANGE_AVATAR_OPTION& request =
        static_cast<const MSG_CHANGE_AVATAR_OPTION&>(message);
    CInventory* const inventory = user->getCurCharacInvenW();
    CInventory shadow_inventory;
    InventoryMemory shadow_memory;
    shadow_inventory.SetInventoryMemory(&shadow_memory);
    shadow_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (shadow_inventory.delete_item(kAvatarOptionChangeMaterialInventory,
                                     request.material_slot,
                                     1,
                                     kAvatarOptionChangeDeleteReason,
                                     false) != 1) {
        result.result = 22;
        return 0;
    }

    if (!shadow_inventory.GetInvenRef(kAvatarOptionChangeAvatarInventory,
                                      request.avatar_slot)) {
        result.result = 17;
        return 0;
    }

    // The retail implementation intentionally does not branch on this live
    // deletion result after its shadow-transaction preflight.
    inventory->delete_item(kAvatarOptionChangeMaterialInventory,
                           request.material_slot,
                           1,
                           kAvatarOptionChangeDeleteReason,
                           true);
    Inven_Item* const avatar = inventory->GetInvenRef(
        kAvatarOptionChangeAvatarInventory, request.avatar_slot);
    CUser_history_log(user)->UseAvatarOptionChange(
        avatar->add_info_, avatar->item_id_, avatar->avatar_select_,
        request.option);
    avatar->avatar_select_ = request.option;
    inventory->SendAvatarEvent(kAvatarOptionChangeDbSignal,
                               avatar->add_info_,
                               request.option,
                               0,
                               0);
    result.material_slot = request.material_slot;
    result.avatar_slot = request.avatar_slot;
    result.option = request.option;
    return 0;
}

// IDA_EA: 0x81E0E32
// IDA_NAME: _ZN29Dispatcher_AvatarOptionChange4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_AvatarOptionChange::send(CUser *,ParamBase &)
void Dispatcher_AvatarOptionChange::send(CUser* user, ParamBase& param) {
    const stChangeAvatarOption& result =
        static_cast<const stChangeAvatarOption&>(param);
    if (result.result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_AVATAR_OPTION_CHANGE,
            static_cast<unsigned char>(result.result));
        return;
    }

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_AVATAR_OPTION_CHANGE);
    response.put_byte(1);
    response.put_short(result.avatar_slot);
    response.put_byte(result.option);
    response.finalize(true);
    user->Send(response);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             result.material_slot);
}
