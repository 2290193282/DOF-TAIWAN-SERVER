#include "old_data/Login/packet_dispatch/cera_package_open_dispatcher.h"

#include <cstring>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kCeraPackageOpenReadSignature[] =
    "virtual int Dispatcher_CeraPackageOpen::read(PacketBuf&, MSG_BASE&)";
const char kCeraPackageOpenProcessSignature[] =
    "virtual int Dispatcher_CeraPackageOpen::process(CUser*, MSG_BASE&, ParamBase&)";

const INVEN_TYPE kCeraPackageInventory = static_cast<INVEN_TYPE>(1);
const eItemDelReason kCeraPackageDeleteReason =
    static_cast<eItemDelReason>(3);

}  // namespace

// The state-0 specialization is this derived object's sole template/vtable
// provider.  It is deliberately not registered into the 606-slot root.
// IDA_EA: 0x8248CC8
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8248E32
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8248E3C
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x8248ECE
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825B0F2 / 0x825B122
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EED2Ev /
//           _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EED0Ev
// IDA_EA: 0x85991CA
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_OPEN_CERAPACKAGE17stOpenCeraPackageL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_OPEN_CERAPACKAGE,
                                 stOpenCeraPackage,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597EA6
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpenC2Ev
// DEMANGLED: Dispatcher_CeraPackageOpen::Dispatcher_CeraPackageOpen(void)
Dispatcher_CeraPackageOpen::Dispatcher_CeraPackageOpen()
    : IPacketDispatcher<MSG_OPEN_CERAPACKAGE,
                        stOpenCeraPackage,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243B6A
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpenD2Ev
// DEMANGLED: Dispatcher_CeraPackageOpen::~Dispatcher_CeraPackageOpen()
// IDA_EA: 0x8243B9A
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpenD0Ev
// DEMANGLED: Dispatcher_CeraPackageOpen::~Dispatcher_CeraPackageOpen()
Dispatcher_CeraPackageOpen::~Dispatcher_CeraPackageOpen() {}

// IDA_EA: 0x81E76FA
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpen4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CeraPackageOpen::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CeraPackageOpen::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_OPEN_CERAPACKAGE& request =
        static_cast<MSG_OPEN_CERAPACKAGE&>(message);
    if (packet.get_short(request.target_slot) != 1) {
        return LineFunc(25451, kCeraPackageOpenReadSignature, 0, 0);
    }
    if (packet.get_byte(request.item_count) != 1) {
        return LineFunc(25454, kCeraPackageOpenReadSignature, 0, 0);
    }
    if (request.item_count > 35u) {
        return LineFunc(25457, kCeraPackageOpenReadSignature, 0, 0);
    }
    for (unsigned int i = 0; i < request.item_count; ++i) {
        if (packet.get_int(request.choices[i].item_index) != 1) {
            return LineFunc(25462, kCeraPackageOpenReadSignature, 0, 0);
        }
        if (packet.get_byte(request.choices[i].select) != 1) {
            return LineFunc(25465, kCeraPackageOpenReadSignature, 0, 0);
        }
    }
    return 0;
}

// IDA_EA: 0x81E7CC2
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpen11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_CeraPackageOpen::check_error(CUser *,MSG_BASE &)
int Dispatcher_CeraPackageOpen::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() != 3 || user->CheckInTrade() ||
        !user->getCurCharacR()) {
        return 19;
    }
    return 0;
}

// IDA_EA: 0x81E788A
// IDA_NAME: _ZN26Dispatcher_CeraPackageOpen7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CeraPackageOpen::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CeraPackageOpen::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    stOpenCeraPackage& result = static_cast<stOpenCeraPackage&>(param);
    result.result = check_error(user, message);
    if (result.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPEN_CERAPACKAGE,
                                 static_cast<unsigned char>(result.result));
        return 0;
    }

    const MSG_OPEN_CERAPACKAGE& request =
        static_cast<const MSG_OPEN_CERAPACKAGE&>(message);
    if (user->CheckItemLock(1, request.target_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPEN_CERAPACKAGE, 4);
        return 0;
    }

    const Inven_Item package_item =
        user->getCurCharacInvenR()->GetInvenSlot(1, request.target_slot);
    if (!package_item.item_id_) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPEN_CERAPACKAGE, 1);
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(package_item.item_id_);
    if (!item || item->is_stackable() != 1 ||
        item->vtable_->get_item_type(item) != 38) {
        // The original returns one to the template rather than emitting an
        // error packet for a raced/mismatched package prototype.
        return 1;
    }

    WongWork::CCeraShop::stCeraShopItemParam_t selected_items;
    for (unsigned int i = 0; i < request.item_count; ++i) {
        selected_items.selected_items.push_back(std::make_pair(
            static_cast<int>(request.choices[i].item_index),
            static_cast<int>(request.choices[i].select)));
    }

    std::vector<Inven_Item> output_items;
    STSpecailItem_Param special_param(output_items, selected_items);
    special_param.user = user;
    special_param.item_id = package_item.item_id_;
    special_param.special_value = 0;
    special_param.restrict_code = 0;
    special_param.current_seconds = 0;
    special_param.is_vending_machine = false;
    special_param.first_flag = 0;
    special_param.second_flag = 0;
    special_param.third_flag = false;

    STSpecailItem_Result special_result;
    if (GlobalData::s_pCeraShop->CallSpecialItemHandler(
            5, special_param, special_result) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPEN_CERAPACKAGE,
                                 static_cast<unsigned char>(special_result.result));
        return 0;
    }
    if (special_result.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPEN_CERAPACKAGE,
                                 static_cast<unsigned char>(special_result.result));
        return 0;
    }

    user->getCurCharacInvenW()->delete_item(kCeraPackageInventory,
                                            request.target_slot,
                                            1,
                                            kCeraPackageDeleteReason,
                                            true);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_OPEN_CERAPACKAGE);
    response.put_byte(1);
    response.put_short(request.target_slot);
    response.finalize(true);
    user->Send(response);
    return 0;
}
