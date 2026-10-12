#include "old_data/Login/packet_dispatch/purify_item_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// The original dispatcher throws this named integral enum for checked
// inventory races.  Its owning network-frame catch remains outside this
// derived handler, exactly as in the old binary.
enum ENUM_ERROR : int {};

namespace {

const char kPurifyReadSignature[] =
    "virtual int Dispatcher_PurifyItem::read(PacketBuf&, MSG_BASE&)";
const char kPurifyProcessSignature[] =
    "virtual int Dispatcher_PurifyItem::process(CUser*, MSG_BASE&, ParamBase&)";

const INVEN_TYPE kPurifyInventory = static_cast<INVEN_TYPE>(1);
const eItemDelReason kPurifyMaterialDeleteReason =
    static_cast<eItemDelReason>(3);
const eApplyItemReason kPurifyApplyItemReason =
    static_cast<eApplyItemReason>(2);

}  // namespace

// The original state-0 specialization is the sole CMD204 template/vtable
// provider.  It is deliberately not registered into PacketDispatcher's
// 606-slot root during derived-provider closure.
// IDA_EA: 0x82484F4
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8248650
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824865A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825AFBA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825AFEA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82486EC
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x859879E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PURIFY_ITEM9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PURIFY_ITEM,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_PURIFY_ITEM,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x822E42E
// IDA_NAME: _ZN15MSG_PURIFY_ITEMC2Ev
// DEMANGLED: MSG_PURIFY_ITEM::MSG_PURIFY_ITEM(void)
MSG_PURIFY_ITEM::MSG_PURIFY_ITEM()
    : MSG_BASE(), target_slot(0), target_item_id(0), material_slot(0),
      material_item_id(0) {}

// IDA_EA: 0x85968DC
// IDA_NAME: _ZN21Dispatcher_PurifyItemC2Ev
// DEMANGLED: Dispatcher_PurifyItem::Dispatcher_PurifyItem(void)
Dispatcher_PurifyItem::Dispatcher_PurifyItem()
    : IPacketDispatcher<MSG_PURIFY_ITEM,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241812
// IDA_NAME: _ZN21Dispatcher_PurifyItemD2Ev
// DEMANGLED: Dispatcher_PurifyItem::~Dispatcher_PurifyItem()
// IDA_EA: 0x8241842
// IDA_NAME: _ZN21Dispatcher_PurifyItemD0Ev
// DEMANGLED: Dispatcher_PurifyItem::~Dispatcher_PurifyItem()
Dispatcher_PurifyItem::~Dispatcher_PurifyItem() {}

// IDA_EA: 0x8218B94
// IDA_NAME: _ZN21Dispatcher_PurifyItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PurifyItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PurifyItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_PURIFY_ITEM& request = static_cast<MSG_PURIFY_ITEM&>(message);
    if (packet.get_short(request.target_slot) != 1) {
        return LineFunc(52159, kPurifyReadSignature, 0, 0);
    }
    if (packet.get_item_idx(request.target_item_id) != 1) {
        return LineFunc(52161, kPurifyReadSignature, 0, 0);
    }
    if (packet.get_short(request.material_slot) != 1) {
        return LineFunc(52163, kPurifyReadSignature, 0, 0);
    }
    if (packet.get_item_idx(request.material_item_id) == 1) {
        return 0;
    }
    return LineFunc(52165, kPurifyReadSignature, 0, 0);
}

// IDA_EA: 0x8218CB6
// IDA_NAME: _ZN21Dispatcher_PurifyItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PurifyItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PurifyItem::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)param;
    if (user->get_state() != 3) {
        return LineFunc(52171, kPurifyProcessSignature, 0, 0);
    }
    if (G_GameWorld()->GetChannelType() == 7) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_PURIFY_ITEM, 17);
        return 0;
    }

    MSG_PURIFY_ITEM& request = static_cast<MSG_PURIFY_ITEM&>(message);
    if (user->CheckItemLock(static_cast<ENUM_ITEMSPACE>(1), request.target_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_PURIFY_ITEM, 213);
        return 0;
    }

    const int material_type =
        GlobalData::s_itemAmplifier_->getPurifyMaterialType(
            static_cast<int>(request.material_item_id));
    if (material_type == -1 ||
        GlobalData::s_itemAmplifier_->getPurifyMaterialItemIdx(material_type) !=
            static_cast<int>(request.material_item_id)) {
        throw static_cast<ENUM_ERROR>(17);
    }

    CInventory* const read_inventory = user->getCurCharacInvenR();
    Inven_Item target_item = read_inventory->GetInvenSlot(1, request.target_slot);
    if (static_cast<unsigned long>(target_item.item_id_) != request.target_item_id ||
        target_item.amplify_option_.isIdentified()) {
        throw static_cast<ENUM_ERROR>(17);
    }

    Inven_Item material_item =
        read_inventory->GetInvenSlot(1, request.material_slot);
    const int material_count =
        GlobalData::s_itemAmplifier_->getPurifyMaterialItemCount(material_type);
    if (static_cast<unsigned long>(material_item.item_id_) !=
            request.material_item_id ||
        material_item.add_info_ < material_count) {
        throw static_cast<ENUM_ERROR>(17);
    }

    CInventory* const write_inventory = user->getCurCharacInvenW();
    if (write_inventory->delete_item(kPurifyInventory,
                                     request.material_slot,
                                     material_count,
                                     kPurifyMaterialDeleteReason,
                                     true) != 1) {
        throw static_cast<ENUM_ERROR>(22);
    }
    if (processPurifyType(material_type, target_item) != 1) {
        throw static_cast<ENUM_ERROR>(22);
    }

    user->history_log().apply_Item(material_item.item_id_,
                                   target_item.item_id_,
                                   kPurifyApplyItemReason);
    write_inventory->update_item(kPurifyInventory, request.target_slot,
                                 target_item);

    // The old packet fetches the material a second time after the delete so
    // its remaining add-info is sent rather than the pre-transaction count.
    material_item = read_inventory->GetInvenSlot(1, request.material_slot);
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_PURIFY_ITEM);
    response.put_byte(1);
    response.put_short(request.material_slot);
    response.put_int(material_item.add_info_);
    response.put_short(request.target_slot);
    response.put_byte(target_item.amplify_option_.getAbilityType());
    response.put_short(target_item.amplify_option_.getAbilityValue());
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82194B6
// IDA_NAME: _ZN21Dispatcher_PurifyItem17processPurifyTypeEiR10Inven_Item
// DEMANGLED: Dispatcher_PurifyItem::processPurifyType(int,Inven_Item &)
int Dispatcher_PurifyItem::processPurifyType(int material_type,
                                             Inven_Item& target_item) {
    // 0828 CMD204 exposes two identify materials.  The ordinary material is
    // type 0 and [purify material for magi] is type 2; both reveal the same
    // hidden amplify option.  Legacy [purify only material] is type 1 and
    // [purify only cera material] is type 3; both keep the reset behavior.
    if (material_type == 0 || material_type == 2) {
        return target_item.amplify_option_.identify();
    }
    if (material_type != 1 && material_type != 3) {
        return 0;
    }
    target_item.amplify_option_.reset();
    return 1;
}
