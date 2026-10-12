#include "old_data/Login/packet_dispatch/use_dye_dispatcher.h"

#include <string>

#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kUseDyeReadSignature[] =
    "virtual int Dispatcher_UseDye::read(PacketBuf&, MSG_BASE&)";
const char kUseDyeProcessSignature[] =
    "virtual int Dispatcher_UseDye::process(CUser*, MSG_BASE&, ParamBase&)";

const INVEN_TYPE kUseDyeInventory = static_cast<INVEN_TYPE>(1);
const INVEN_TYPE kUseDyeAvatarInventory = static_cast<INVEN_TYPE>(2);
const eItemDelReason kUseDyeDeleteReason = static_cast<eItemDelReason>(3);

}  // namespace

// IDA_EA: 0x82F9964
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr14SetAvatarColorEiss
// DEMANGLED: WongWork::CAvatarItemMgr::SetAvatarColor(int,short,short)
int WongWork::CAvatarItemMgr::SetAvatarColor(int key,
                                              short first_color,
                                              short second_color) {
    WongWork::Avatar_Item* const item = _FindItemAll(key);
    if (!item) return 0;
    item->setColor(first_color, second_color);
    return 1;
}

// IDA_EA: 0x82F99BC
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetAvatarInfoEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetAvatarInfo(int)const
WongWork::Avatar_Item* WongWork::CAvatarItemMgr::GetAvatarInfo(int key) const {
    return _FindItemAll(key);
}

// IDA_EA: 0x8686CBA
// IDA_NAME: _ZN15cUserHistoryLog6UseDyeEiiiRKSsi
// DEMANGLED: cUserHistoryLog::UseDye(int,int,int,std::string const&,int)
CUser* cUserHistoryLog::UseDye(int dye_item_id,
                                int avatar_item_id,
                                int avatar_key,
                                const std::string& ipg_agency_no,
                                int avatar_slot) {
    if (owner_) {
        return reinterpret_cast<CUser*>(static_cast<uintptr_t>(
            owner_->LogHistory("UseDye ,%d,%d,%d,%s,%d",
                               dye_item_id,
                               avatar_item_id,
                               avatar_key,
                               ipg_agency_no.c_str(),
                               avatar_slot)));
    }
    return owner_;
}

// IDA_EA: 0x822E3B2
// IDA_NAME: _ZN11MSG_USE_DYEC2Ev
// DEMANGLED: MSG_USE_DYE::MSG_USE_DYE(void)
MSG_USE_DYE::MSG_USE_DYE() : MSG_BASE(), dye_slot(0), avatar_slot(0) {}

// IDA_EA: 0x8231DE4
// IDA_NAME: _ZN8stUseDyeC2Ev
// DEMANGLED: stUseDye::stUseDye(void)
stUseDye::stUseDye() : ParamBase(), result(0) {}

// IDA_EA: 0x8246576
// IDA_NAME: _ZN8stUseDyeD2Ev
// DEMANGLED: stUseDye::~stUseDye()
// IDA_EA: 0x82465A6
// IDA_NAME: _ZN8stUseDyeD0Ev
// DEMANGLED: stUseDye::~stUseDye()
stUseDye::~stUseDye() {}

// The exact CMD499 specialization is defined once in this TU.  The class is
// intentionally unregistered, matching the current root-dispatcher boundary.
// IDA_EA: 0x82490D2
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824922E
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8249238
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B18E
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B1BE
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82492CA
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x859913E
// IDA_NAME: _ZN17IPacketDispatcherI11MSG_USE_DYE8stUseDyeL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_USE_DYE,stUseDye,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_USE_DYE, stUseDye, static_cast<ch_state>(0)>;

// IDA_EA: 0x8597DFE
// IDA_NAME: _ZN17Dispatcher_UseDyeC2Ev
// DEMANGLED: Dispatcher_UseDye::Dispatcher_UseDye(void)
Dispatcher_UseDye::Dispatcher_UseDye()
    : IPacketDispatcher<MSG_USE_DYE, stUseDye, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243C06
// IDA_NAME: _ZN17Dispatcher_UseDyeD2Ev
// DEMANGLED: Dispatcher_UseDye::~Dispatcher_UseDye()
// IDA_EA: 0x8243C36
// IDA_NAME: _ZN17Dispatcher_UseDyeD0Ev
// DEMANGLED: Dispatcher_UseDye::~Dispatcher_UseDye()
Dispatcher_UseDye::~Dispatcher_UseDye() {}

// IDA_EA: 0x81E6952
// IDA_NAME: _ZN17Dispatcher_UseDye4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseDye::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseDye::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USE_DYE& request = static_cast<MSG_USE_DYE&>(message);
    if (packet.get_short(request.dye_slot) != 1) {
        return LineFunc(25157, kUseDyeReadSignature, 0, 0);
    }
    if (packet.get_short(request.avatar_slot) == 1) {
        return 0;
    }
    return LineFunc(25160, kUseDyeReadSignature, 0, 0);
}

// IDA_EA: 0x81E727E
// IDA_NAME: _ZN17Dispatcher_UseDye11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_UseDye::check_error(CUser *,MSG_BASE &)
int Dispatcher_UseDye::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() == 3) {
        return user->CheckInTrade() != 0;
    }
    return 19;
}

// IDA_EA: 0x81E71C0
// IDA_NAME: _ZNK17Dispatcher_UseDye7getItemERK5CUserisR10Inven_Item
// DEMANGLED: Dispatcher_UseDye::getItem(CUser const&,int,short,Inven_Item &)const
bool Dispatcher_UseDye::getItem(const CUser& user,
                                int inventory_type,
                                short slot,
                                Inven_Item& item) const {
    item = user.getCurCharacInvenR()->GetInvenSlot(inventory_type, slot);
    return item.item_id_ == 0;
}

// IDA_EA: 0x81E70E6
// IDA_NAME: _ZNK17Dispatcher_UseDye30notifyUpdatedInventoryItemInfoER5CUsers
// DEMANGLED: Dispatcher_UseDye::notifyUpdatedInventoryItemInfo(CUser &,short)const
void Dispatcher_UseDye::notifyUpdatedInventoryItemInfo(CUser& user,
                                                       short slot) const {
    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    user.getCurCharacInvenR()->MakeItemPacket(kUseDyeInventory, slot, packet);
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x81E69EA
// IDA_NAME: _ZN17Dispatcher_UseDye7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseDye::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseDye::process(CUser* user,
                               MSG_BASE& message,
                               ParamBase& param) {
    stUseDye& result = static_cast<stUseDye&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) return 0;
    if (result.result < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_DYE,
                                 static_cast<unsigned char>(result.result));
        return LineFunc(25178, kUseDyeProcessSignature, result.result, 0);
    }

    const MSG_USE_DYE& request = static_cast<const MSG_USE_DYE&>(message);
    Inven_Item dye_item;
    int item_error = getItem(*user, kUseDyeInventory, request.dye_slot, dye_item);
    if (item_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_DYE,
                                 static_cast<unsigned char>(item_error));
        return 1;
    }

    Inven_Item avatar_item;
    item_error = getItem(*user,
                         kUseDyeAvatarInventory,
                         request.avatar_slot,
                         avatar_item);
    if (item_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_DYE,
                                 static_cast<unsigned char>(item_error));
        return 1;
    }

    CItem* const dye_prototype = G_CDataManager()->find_item(dye_item.item_id_);
    if (!dye_prototype || dye_prototype->is_stackable() != 1) return 1;

    const DyeInfo* const dye_info = dye_prototype->GetDyeInfo();
    if (!dye_info->type || dye_info->colors.empty()) return 1;
    const int color_index = dye_info->colors[0].first;
    const AvatarVariation::AvatarColorInfo* const color_info =
        G_CDataManager()->get_avatar_color_info();
    if (color_info->colors.find(color_index) == color_info->colors.end()) {
        return 1;
    }

    CItem* const avatar_prototype =
        G_CDataManager()->find_item(avatar_item.item_id_);
    if (!avatar_prototype->vtable_->is_avatar_item(avatar_prototype)) return 1;
    CEquipItem* const avatar_equipment =
        static_cast<CEquipItem*>(avatar_prototype);
    if (avatar_equipment->IsAvatarColorVariation(0) != 1) return 1;

    const int avatar_key = avatar_item.add_info_;
    const WongWork::CAvatarItemMgr* const avatar_read_manager =
        user->getCurCharacInvenR()->GetAvatarItemMgrR();
    const stAvatarExpansionInfo_t* const old_expansion =
        avatar_read_manager->GetExpansionInfo(avatar_key);
    if (!old_expansion) return 1;

    const short old_first_color = old_expansion->avatar_expansion[0];
    const short old_second_color = old_expansion->avatar_expansion[1];
    const short color = static_cast<short>(color_index);
    WongWork::CAvatarItemMgr* const avatar_write_manager =
        user->getCurCharacInvenW()->GetAvatarItemMgrW();
    if (avatar_write_manager->SetAvatarColor(avatar_key, color, 0) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_DYE, 1);
        return LineFunc(25234, kUseDyeProcessSignature, 1, user->get_acc_id());
    }

    if (user->getCurCharacInvenW()->delete_item(kUseDyeInventory,
                                                request.dye_slot,
                                                1,
                                                kUseDyeDeleteReason,
                                                true) != 1) {
        avatar_write_manager->SetAvatarColor(avatar_key,
                                             old_first_color,
                                             old_second_color);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_DYE, 17);
        return LineFunc(25243, kUseDyeProcessSignature, 1, user->get_acc_id());
    }

    notifyUpdatedInventoryItemInfo(*user, request.dye_slot);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_DYE);
    response.put_byte(1);
    response.put_short(request.avatar_slot);
    const short color_payload[2] = {color, 0};
    response.put_int(4);
    response.put_binary(reinterpret_cast<const char*>(color_payload), 4);
    response.finalize(true);
    user->Send(response);

    user->getCurCharacInvenW()->SendAvatarEvent(794, avatar_key, color_index, 0, 0);
    WongWork::Avatar_Item* const avatar_info =
        user->getCurCharacInvenW()->GetAvatarItemMgrR()->GetAvatarInfo(avatar_key);
    const std::string ipg_agency_no(avatar_info->GetIPGAgencyNo());
    CUser_history_log(user)->UseDye(dye_item.item_id_,
                                    avatar_item.item_id_,
                                    avatar_key,
                                    ipg_agency_no,
                                    request.avatar_slot);
    return 0;
}
