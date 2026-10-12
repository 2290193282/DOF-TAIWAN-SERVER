#include "old_game/dungeon/room/packet_dispatcher_seria_room_deco.h"

#include <type_traits>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_update_seria_room_animation_deco.h"
#include "old_game/activity/schedule/seria_room_ani_deco_event.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_SeriaRoom_Ani_Deco_Event::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_SeriaRoom_Ani_Deco_Event::process(CUser*, MSG_BASE&, ParamBase&)";
const char kTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SERIAROOM_DECO_EVENT, PARAM_TYPE = stSeriaRoomDecoEvent, ch_state STATE = (ch_state)3u]";

// Raw dispatch_template @0x82486F2 reserves an unconstructed 16-byte request
// extent and passes it directly to read().  No MSG_SERIAROOM_DECO_EVENT C2
// occurs in the original call graph.
typedef std::aligned_storage<sizeof(MSG_SERIAROOM_DECO_EVENT),
                             alignof(MSG_SERIAROOM_DECO_EVENT)>::type
    RawSeriaRoomAniDecoMessageStorage;

static_assert(sizeof(RawSeriaRoomAniDecoMessageStorage) == 0x10u,
              "CMD513 raw request extent must stay 16 bytes");

}  // namespace

// IDA_EA: 0x8599202
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B008 / 0x825B038
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EED2Ev / ...D0Ev
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x82486F2
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    RawSeriaRoomAniDecoMessageStorage message_storage;
    MSG_SERIAROOM_DECO_EVENT& message =
        *reinterpret_cast<MSG_SERIAROOM_DECO_EVENT*>(&message_storage);
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    stSeriaRoomDecoEvent param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }
    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8248842
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                 PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824884C
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                                PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code = 0;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825D582
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                       stSeriaRoomDecoEvent,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8597EDE
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_EventC2Ev
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::Dispatcher_SeriaRoom_Ani_Deco_Event(void)
Dispatcher_SeriaRoom_Ani_Deco_Event::Dispatcher_SeriaRoom_Ani_Deco_Event()
    : IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                        stSeriaRoomDecoEvent,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8243A32 / 0x8243A62
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_EventD2Ev / ...D0Ev
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::~Dispatcher_SeriaRoom_Ani_Deco_Event()
Dispatcher_SeriaRoom_Ani_Deco_Event::~Dispatcher_SeriaRoom_Ani_Deco_Event() {}

// IDA_EA: 0x81E8814
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_Event4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SeriaRoom_Ani_Deco_Event::read(PacketBuf& packet,
                                              MSG_BASE& base_message) {
    MSG_SERIAROOM_DECO_EVENT& message =
        static_cast<MSG_SERIAROOM_DECO_EVENT&>(base_message);
    if (packet.get_short(message.inventory_slot) == 1) {
        return 0;
    }
    return LineFunc(25920, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E886A
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_Event7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SeriaRoom_Ani_Deco_Event::process(CUser* user,
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
        return LineFunc(25937, kProcessSignature, param.error_code, 0);
    }

    seriaRoom_AniDeco decoration = user->getSeriaRoomAniDecoInfo();
    const int16_t decoration_flow =
        static_cast<int16_t>(decoration.deco_flow);
    CDataManager* const data_manager = G_CDataManager();
    if (decoration_flow >= static_cast<int16_t>(data_manager->maxAniDecoItem())) {
        param.error_code = 20;
        return 0;
    }

    const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
        1, message.inventory_slot);
    if (!data_manager->find_item(item.item_id_)) {
        param.error_code = 1;
        return 0;
    }
    if (decoration_flow == -1) {
        param.error_code = 1;
        return 0;
    }

    // IDA passes HIWORD(seriaRoom_AniDeco) first and the consumed item id
    // second.  The historic CDataManager parameter labels are reversed.
    if (data_manager->invalidAniDecoItem(decoration_flow, item.item_id_) != 1) {
        param.error_code = 23;
        return 0;
    }

    param.selected_inventory_slot = message.inventory_slot;
    user->getCurCharacInvenW()->delete_item(
        static_cast<INVEN_TYPE>(1),
        message.inventory_slot,
        1,
        static_cast<eItemDelReason>(3),
        true);

    ++decoration.deco_flow;
    user->setSeriaRoomAniDecoInfo(decoration);
    DB_UpdateSeriaRoomAnimationDeco::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<short>(user->GetServerGroupToPvP()),
        user->getCurCharacNo(),
        static_cast<short>(decoration.receive_flow),
        static_cast<short>(decoration.deco_flow));

    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(105);
    if (base_event->IsEventing(0) == 1) {
        SeriaRoomAniDecoEvent* const event =
            static_cast<SeriaRoomAniDecoEvent*>(
                GlobalData::s_event_manager->GetRepeatEvent(105));
        if (event) {
            param.error_code = event->processEvent(*user);
        }
    }
    return 0;
}

// IDA_EA: 0x81E8B20
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_Event11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::check_error(CUser *,MSG_BASE &)
int Dispatcher_SeriaRoom_Ani_Deco_Event::check_error(CUser* user,
                                                     MSG_BASE& message) {
    (void)user;
    (void)message;
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(105);
    return event->IsEventing(0) != 1;
}

// IDA_EA: 0x81E8B64
// IDA_NAME: _ZN35Dispatcher_SeriaRoom_Ani_Deco_Event4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SeriaRoom_Ani_Deco_Event::send(CUser *,ParamBase &)
void Dispatcher_SeriaRoom_Ani_Deco_Event::send(CUser* user,
                                              ParamBase& base_param) {
    const stSeriaRoomDecoEvent& param =
        static_cast<const stSeriaRoomDecoEvent&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SERIAROOM_ANIMATION_DECO_EVENT);
    if (param.error_code != 0) {
        response.put_byte(0);
        response.put_byte(param.error_code);
    } else {
        response.put_byte(1);
        response.put_short(param.selected_inventory_slot);
        response.put_short(user->getSeriaRoomAniDecoInfo().deco_flow);
    }
    response.finalize(true);
    user->Send(response);
}
