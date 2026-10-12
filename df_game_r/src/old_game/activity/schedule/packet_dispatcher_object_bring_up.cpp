#include "old_game/activity/schedule/packet_dispatcher_object_bring_up.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kObjectBringUpReadSignature[] =
    "virtual int Dispatcher_ObjectBringUp::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x824B856
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824B9A6
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824B9B0
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825D5EE
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825B758 / 0x825B788
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EED0Ev
// IDA_EA: 0x8598FB6
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_OBJECT_BRING_UP15stObjectBringUpL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_OBJECT_BRING_UP,
                                 stObjectBringUp,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8231AE6
// IDA_NAME: _ZN15stObjectBringUpC2Ev
// DEMANGLED: stObjectBringUp::stObjectBringUp(void)
stObjectBringUp::stObjectBringUp()
    : ParamBase(),
      error_code(0),
      material_slot(0),
      material_total(0),
      reward_count(0),
      reward_values() {}

// IDA_EA: 0x8246902
// IDA_NAME: _ZN15stObjectBringUpD2Ev
// IDA_EA: 0x8246964
// IDA_NAME: _ZN15stObjectBringUpD0Ev
// DEMANGLED: stObjectBringUp::~stObjectBringUp()
stObjectBringUp::~stObjectBringUp() {}

// IDA_EA: 0x8597B5E
// IDA_NAME: _ZN24Dispatcher_ObjectBringUpC2Ev
// DEMANGLED: Dispatcher_ObjectBringUp::Dispatcher_ObjectBringUp(void)
Dispatcher_ObjectBringUp::Dispatcher_ObjectBringUp()
    : IPacketDispatcher<MSG_OBJECT_BRING_UP,
                        stObjectBringUp,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x824426C
// IDA_NAME: _ZN24Dispatcher_ObjectBringUpD2Ev
// IDA_EA: 0x824429C
// IDA_NAME: _ZN24Dispatcher_ObjectBringUpD0Ev
// DEMANGLED: Dispatcher_ObjectBringUp::~Dispatcher_ObjectBringUp()
Dispatcher_ObjectBringUp::~Dispatcher_ObjectBringUp() {}

// IDA_EA: 0x81E147E
// IDA_NAME: _ZN24Dispatcher_ObjectBringUp4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ObjectBringUp::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ObjectBringUp::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_OBJECT_BRING_UP& request =
        static_cast<MSG_OBJECT_BRING_UP&>(message);
    if (packet.get_short(request.material_slot) == 1) {
        return 0;
    }
    return LineFunc(22186, kObjectBringUpReadSignature, 0, 0);
}

// IDA_EA: 0x81E14D4
// IDA_NAME: _ZN24Dispatcher_ObjectBringUp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ObjectBringUp::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ObjectBringUp::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    stObjectBringUp& result = static_cast<stObjectBringUp&>(param);
    EventClassify::CObjectBringUp* const event =
        static_cast<EventClassify::CObjectBringUp*>(
            G_CDataManager()->get_event_script_mng()->get_event_entity(411));
    if (!event || event->is_eventing() != 1) {
        result.error_code = 178;
        return 0;
    }

    result.error_code = check_error(user, message);
    if (result.error_code != 0) {
        return 0;
    }

    const MSG_OBJECT_BRING_UP& request =
        static_cast<const MSG_OBJECT_BRING_UP&>(message);
    const Inven_Item material = user->getCurCharacInvenR()->GetInvenSlot(
        1, static_cast<int>(request.material_slot));
    if (!G_CDataManager()->find_item(static_cast<int>(material.item_id_))) {
        result.error_code = 17;
        return 0;
    }
    if (!event->verifyMaterialItemByRule(user, material.item_id_)) {
        result.error_code = 23;
        return 0;
    }

    user->incCurUseBringUpMaterialCount();
    result.material_total = static_cast<uint8_t>(
        user->getCurUseBringUpMaterialTotal());
    user->getCurCharacInvenW()->delete_item(
        static_cast<INVEN_TYPE>(1),
        static_cast<int>(request.material_slot),
        1,
        static_cast<eItemDelReason>(3),
        true);
    if (event->sendBringUpReward(user,
                                 result.reward_values,
                                 result.reward_count) == 1) {
        result.material_slot = request.material_slot;
    } else {
        result.error_code = 1;
    }
    return 0;
}

// IDA_EA: 0x81E16CE
// IDA_NAME: _ZN24Dispatcher_ObjectBringUp11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ObjectBringUp::check_error(CUser *,MSG_BASE &)
int Dispatcher_ObjectBringUp::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return -1;
    }
    return user->get_state() > 2 ? 0 : -2;
}

// IDA_EA: 0x81E1704
// IDA_NAME: _ZN24Dispatcher_ObjectBringUp4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ObjectBringUp::send(CUser *,ParamBase &)
void Dispatcher_ObjectBringUp::send(CUser* user, ParamBase& param) {
    const stObjectBringUp& result = static_cast<const stObjectBringUp&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_OBJECT_BRING_UP_USE_ITEM);
    if (result.error_code != 0) {
        packet.put_byte(0);
        packet.put_byte(result.error_code);
    } else {
        packet.put_byte(1);
        packet.put_short(result.material_slot);
        packet.put_byte(result.material_total);
        packet.put_byte(result.reward_count);
        if (result.reward_count != 0) {
            std::vector<int>::const_iterator value = result.reward_values.begin();
            while (value != result.reward_values.end()) {
                packet.put_short(*value);
                ++value;
                packet.put_int(*value);
                ++value;
                packet.put_int(*value);
                ++value;
                packet.put_short(*value);
                ++value;
            }
        }
    }
    packet.finalize(true);
    user->Send(packet);
}
