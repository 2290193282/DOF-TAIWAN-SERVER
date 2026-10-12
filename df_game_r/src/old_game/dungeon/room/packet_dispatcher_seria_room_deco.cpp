#include "old_game/dungeon/room/packet_dispatcher_seria_room_deco.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x8597DAA
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_EventC2Ev
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::Dispatcher_SeriaRoom_Deco_Event(void)
Dispatcher_SeriaRoom_Deco_Event::Dispatcher_SeriaRoom_Deco_Event()
    : IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                        stSeriaRoomDecoEvent,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243CF0
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_EventD2Ev
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::~Dispatcher_SeriaRoom_Deco_Event()
// IDA_EA: 0x8243D20
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_EventD0Ev
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::~Dispatcher_SeriaRoom_Deco_Event()
Dispatcher_SeriaRoom_Deco_Event::~Dispatcher_SeriaRoom_Deco_Event() {}

// IDA_EA: 0x81E5C88
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_Event4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SeriaRoom_Deco_Event::read(PacketBuf& packet,
                                          MSG_BASE& base_message) {
    MSG_SERIAROOM_DECO_EVENT& message =
        static_cast<MSG_SERIAROOM_DECO_EVENT&>(base_message);
    if (packet.get_short(message.inventory_slot) == 1) {
        return 0;
    }
    return LineFunc(
        24575,
        "virtual int Dispatcher_SeriaRoom_Deco_Event::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81E5CDE
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_Event7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SeriaRoom_Deco_Event::process(CUser* user,
                                             MSG_BASE& base_message,
                                             ParamBase& base_param) {
    const MSG_SERIAROOM_DECO_EVENT& message =
        static_cast<const MSG_SERIAROOM_DECO_EVENT&>(base_message);
    stSeriaRoomDecoEvent& param =
        static_cast<stSeriaRoomDecoEvent&>(base_param);
    param.error_code = check_error(user, base_message);
    if (param.error_code > 0) {
        return 0;
    }
    if (param.error_code < 0) {
        return LineFunc(
            24592,
            "virtual int Dispatcher_SeriaRoom_Deco_Event::process(CUser*, MSG_BASE&, ParamBase&)",
            param.error_code,
            0);
    }

    const Inven_Item item =
        user->getCurCharacInvenR()->GetInvenSlot(1, message.inventory_slot);
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager->find_item(item.item_id_)) {
        param.error_code = 17;
        return 0;
    }
    if (data_manager->getVaildEventItem(item.item_id_) == 1) {
        param.selected_inventory_slot = message.inventory_slot;
        return 0;
    }
    param.error_code = 23;
    return 0;
}

// IDA_EA: 0x81E5E50
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_Event11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::check_error(CUser *,MSG_BASE &)
int Dispatcher_SeriaRoom_Deco_Event::check_error(CUser* user,
                                                 MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    return user->get_state() == 3 ? 0 : 7;
}

// IDA_EA: 0x81E5E86
// IDA_NAME: _ZN31Dispatcher_SeriaRoom_Deco_Event4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SeriaRoom_Deco_Event::send(CUser *,ParamBase &)
void Dispatcher_SeriaRoom_Deco_Event::send(CUser* user,
                                           ParamBase& base_param) {
    const stSeriaRoomDecoEvent& param =
        static_cast<const stSeriaRoomDecoEvent&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SERIAROOM_DECO_EVENT);
    if (param.error_code != 0) {
        response.put_byte(0);
        response.put_byte(param.error_code);
    } else {
        response.put_byte(1);
        response.put_short(param.selected_inventory_slot);
        response.put_short(param.deco_flow);
    }
    response.finalize(true);
    user->Send(response);
}
