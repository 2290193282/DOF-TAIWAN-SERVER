#include "old_data/Login/packet_dispatch/collect_items_dispatcher.h"

#include <cstring>
#include <limits.h>

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kCollectItemsReadSignature[] =
    "virtual int Dispatcher_CollectItems::read(PacketBuf&, MSG_BASE&)";
const char kCollectItemsProcessSignature[] =
    "virtual int Dispatcher_CollectItems::process(CUser*, MSG_BASE&, ParamBase&)";

const INVEN_TYPE kCollectItemsInventory = static_cast<INVEN_TYPE>(1);
const eItemDelReason kCollectItemsDeleteReason =
    static_cast<eItemDelReason>(3);
const uint32_t kCollectItemsTicketId = 4810u;

}  // namespace

// IDA_EA: 0x822C75A
// IDA_NAME: _ZN19Packet_CollectItemsC2Ev
// DEMANGLED: Packet_CollectItems::Packet_CollectItems(void)
Packet_CollectItems::Packet_CollectItems()
    : PacketHeader(0x27e6u, 0x0039u), account_id(0), item_count(0),
      charac_no(0), charac_name_length(0) {
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x8231C16
// IDA_NAME: _ZN14stCollectItemsC2Ev
// DEMANGLED: stCollectItems::stCollectItems(void)
stCollectItems::stCollectItems() : ParamBase(), result(0), item_slot(0), item_count(0) {}

// IDA_EA: 0x824677C
// IDA_NAME: _ZN14stCollectItemsD2Ev
// DEMANGLED: stCollectItems::~stCollectItems()
// IDA_EA: 0x82467AC
// IDA_NAME: _ZN14stCollectItemsD0Ev
// DEMANGLED: stCollectItems::~stCollectItems()
stCollectItems::~stCollectItems() {}

// This instantiation is the only provider for the old CMD483 template/vtable
// family.  It is intentionally not wired into the 606-slot root table.
// IDA_EA: 0x824A8A0
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824A9F0
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824A9FA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B4E8
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B518
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D5D0
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8599096
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_COLLECT_ITEMS14stCollectItemsL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_COLLECT_ITEMS,stCollectItems,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_COLLECT_ITEMS,
                                 stCollectItems,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597D3A
// IDA_NAME: _ZN23Dispatcher_CollectItemsC2Ev
// DEMANGLED: Dispatcher_CollectItems::Dispatcher_CollectItems(void)
Dispatcher_CollectItems::Dispatcher_CollectItems()
    : IPacketDispatcher<MSG_COLLECT_ITEMS,
                        stCollectItems,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243FAE
// IDA_NAME: _ZN23Dispatcher_CollectItemsD2Ev
// DEMANGLED: Dispatcher_CollectItems::~Dispatcher_CollectItems()
// IDA_EA: 0x8243FDE
// IDA_NAME: _ZN23Dispatcher_CollectItemsD0Ev
// DEMANGLED: Dispatcher_CollectItems::~Dispatcher_CollectItems()
Dispatcher_CollectItems::~Dispatcher_CollectItems() {}

// IDA_EA: 0x81E33A4
// IDA_NAME: _ZN23Dispatcher_CollectItems4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CollectItems::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CollectItems::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_COLLECT_ITEMS& request = static_cast<MSG_COLLECT_ITEMS&>(message);
    if (packet.get_short(request.item_slot) != 1) {
        return LineFunc(23183, kCollectItemsReadSignature, 0, 0);
    }
    if (packet.get_short(request.item_count) == 1) {
        return 0;
    }
    return LineFunc(23184, kCollectItemsReadSignature, 0, 0);
}

// IDA_EA: 0x81E329E
// IDA_NAME: _ZN23Dispatcher_CollectItems11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CollectItems::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CollectItems::check_error(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    const MSG_COLLECT_ITEMS& request =
        static_cast<const MSG_COLLECT_ITEMS&>(message);
    if (user->get_state() != 3) {
        return 7;
    }
    if (request.item_count != 1u) {
        return 22;
    }

    const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
        kCollectItemsInventory, request.item_slot);
    if (!G_CDataManager()->find_item(item.item_id_)) {
        return 17;
    }
    // The legacy inventory's stack count is Inven_Item::add_info_.  This is
    // the same field CInventory::delete_item decrements for non-equipment
    // items, so the preflight and the mutation use one canonical owner.
    if (item.add_info_ < request.item_count) {
        return 22;
    }
    if (item.item_id_ != kCollectItemsTicketId) {
        return 23;
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        return 60;
    }
    if (GlobalData::s_timeGate_->isOpen()) {
        return 22;
    }
    return 0;
}

// IDA_EA: 0x81E343C
// IDA_NAME: _ZN23Dispatcher_CollectItems7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CollectItems::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CollectItems::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    stCollectItems& result = static_cast<stCollectItems&>(param);
    result.result = check_error(user, message, param);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(23202,
                        kCollectItemsProcessSignature,
                        result.result,
                        0);
    }

    const MSG_COLLECT_ITEMS& request =
        static_cast<const MSG_COLLECT_ITEMS&>(message);
    if (user->getCurCharacInvenW()->delete_item(kCollectItemsInventory,
                                                request.item_slot,
                                                request.item_count,
                                                kCollectItemsDeleteReason,
                                                true) != 1) {
        return LineFunc(23207, kCollectItemsProcessSignature, 0, 0);
    }

    result.item_slot = request.item_slot;
    result.item_count = request.item_count;

    Packet_CollectItems monitor_packet;
    monitor_packet.item_count = request.item_count;
    monitor_packet.charac_no = user->get_charac_no(-1);
    monitor_packet.account_id = user->get_acc_id();
    const char* const charac_name = user->getCurCharacName();
    monitor_packet.charac_name_length =
        static_cast<uint8_t>(std::strlen(charac_name));
    std::strncpy(monitor_packet.charac_name, charac_name, 0x1du);

    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet), 0x39);
    return 0;
}

// IDA_EA: 0x81E360C
// IDA_NAME: _ZN23Dispatcher_CollectItems4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_CollectItems::send(CUser *,ParamBase &)
void Dispatcher_CollectItems::send(CUser* user, ParamBase& param) {
    const stCollectItems& result = static_cast<const stCollectItems&>(param);
    if (result.result == INT_MAX) {
        return;
    }
    if (result.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COLLECT_ITEMS,
                                 static_cast<unsigned char>(result.result));
        return;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_COLLECT_ITEMS);
    response.put_byte(1);
    response.put_short(result.item_slot);
    response.put_short(result.item_count);
    response.finalize(true);
    user->Send(response);
}
