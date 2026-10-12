#include "old_game/progression/career/packet_dispatcher_growth_weapon_event_use_material.h"

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_event_update_growth_weapon_timepiece.h"
#include "old_game/activity/schedule/growth_equip_event.h"
#include "old_game/activity/schedule/event_table.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const int kGrowthWeaponEventId = 100;
// The old dispatcher passes literal inventory type 1 to GetInvenRef.  The
// shared enum has no recovered enumerator table yet, so retain the verified
// ABI value without inventing a public enumerator name.
const INVEN_TYPE kGrowthWeaponMaterialInventoryType =
    static_cast<INVEN_TYPE>(1);
const eItemDelReason kGrowthWeaponMaterialDeleteReason =
    static_cast<eItemDelReason>(54);

const char kReadSignature[] =
    "virtual int Dispatcher_GrowthWeaponEventUseMaterial::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_GrowthWeaponEventUseMaterial::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8248ACE
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8248C2A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8248C34
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B0A4 / 0x825B0D4
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EED2Ev / D0Ev
// IDA_EA: 0x825D58E
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x859915A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GROWTH_EQUIPMENT17stGrowthEquipmentL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_GROWTH_EQUIPMENT,
                                 stGrowthEquipment,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597E36
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterialC2Ev
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::Dispatcher_GrowthWeaponEventUseMaterial(void)
Dispatcher_GrowthWeaponEventUseMaterial::Dispatcher_GrowthWeaponEventUseMaterial() {}

// IDA_EA: 0x8243ACE / 0x8243AFE
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterialD2Ev / D0Ev
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::~Dispatcher_GrowthWeaponEventUseMaterial()
Dispatcher_GrowthWeaponEventUseMaterial::~Dispatcher_GrowthWeaponEventUseMaterial() {}

// IDA_EA: 0x81E810C
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GrowthWeaponEventUseMaterial::read(PacketBuf& packet,
                                                   MSG_BASE& message) {
    MSG_GROWTH_EQUIPMENT& request =
        static_cast<MSG_GROWTH_EQUIPMENT&>(message);
    if (packet.get_int(request.item_space) != 1) {
        return LineFunc(25706, kReadSignature, 0, 0);
    }
    if (packet.get_int(request.slot) == 1) {
        return 0;
    }
    return LineFunc(25709, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E82FA
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::check_error(CUser *,MSG_BASE &)
int Dispatcher_GrowthWeaponEventUseMaterial::check_error(CUser* user,
                                                          MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 19;
    }

    const MSG_GROWTH_EQUIPMENT& request =
        static_cast<const MSG_GROWTH_EQUIPMENT&>(message);
    const Inven_Item* const material =
        user->getCurCharacInvenR()->GetInvenRef(kGrowthWeaponMaterialInventoryType,
                                                static_cast<int>(request.slot));
    if (material->item_id_ == 0) {
        return 21;
    }
    if (material->item_id_ != G_CDataManager()->get_GrowthWeaponMaterialItem()) {
        return 17;
    }

    const GrowthWeaponEvent& event = user->growth_weapon_event_;
    if (event.charac_no_ != user->get_charac_no(-1)) {
        return 3;
    }
    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user->GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    if (static_cast<int16_t>(event.server_group_) != server_group) {
        return 3;
    }
    if (static_cast<int16_t>(event.timepiece_) >=
        G_CDataManager()->get_GrowthWeaponMaterialMax()) {
        return 17;
    }
    return GlobalData::s_event_manager->GetRepeatEvent(kGrowthWeaponEventId)
                   ->IsEventing(0) != 1;
}

// IDA_EA: 0x81E81A4
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GrowthWeaponEventUseMaterial::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    stGrowthEquipment& response = static_cast<stGrowthEquipment&>(param);
    response.result = check_error(user, message);
    if (response.result > 0) {
        return 0;
    }
    if (response.result < 0) {
        return LineFunc(25724, kProcessSignature, response.result, 0);
    }

    const MSG_GROWTH_EQUIPMENT& request =
        static_cast<const MSG_GROWTH_EQUIPMENT&>(message);
    const INVEN_TYPE inventory_type =
        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(request.item_space)));
    if (user->getCurCharacInvenW()->delete_item(
            inventory_type,
            static_cast<int>(request.slot),
            1,
            kGrowthWeaponMaterialDeleteReason,
            true) == 1) {
        GrowthWeaponEvent& event = user->growth_weapon_event_;
        const unsigned short timepiece = static_cast<unsigned short>(
            static_cast<int16_t>(++event.timepiece_));
        DB_EventUpdateGrowthWeaponTimepiece::makeRequest(
            user->get_acc_id(), static_cast<short>(timepiece), 0);
        response.timepiece = timepiece;
        response.item_space = static_cast<int>(request.item_space);
        response.slot = static_cast<int>(request.slot);
    } else {
        response.result = 17;
    }
    return 0;
}

// IDA_EA: 0x81E8496
// IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::send(CUser *,ParamBase &)
void Dispatcher_GrowthWeaponEventUseMaterial::send(CUser* user,
                                                   ParamBase& param) {
    const stGrowthEquipment& response =
        static_cast<const stGrowthEquipment&>(param);
    if (response.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_GROWTH_WEAPON_USE_MATERIAL,
            static_cast<unsigned char>(response.result));
        return;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GROWTH_WEAPON_USE_MATERIAL);
    packet.put_byte(1);
    packet.put_int(response.item_space);
    packet.put_int(response.slot);
    packet.put_short(response.timepiece);
    packet.finalize(1);
    user->Send(packet);
}

// IDA_EA: 0x8597E1A
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinityC2Ev
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::Dispatcher_GrowthWeaponEventChangeInfinity(void)
Dispatcher_GrowthWeaponEventChangeInfinity::
    Dispatcher_GrowthWeaponEventChangeInfinity() {}

// IDA_EA: 0x8243B1C
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinityD2Ev
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::~Dispatcher_GrowthWeaponEventChangeInfinity()
// IDA_EA: 0x8243B4C
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinityD0Ev
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::~Dispatcher_GrowthWeaponEventChangeInfinity()
Dispatcher_GrowthWeaponEventChangeInfinity::
    ~Dispatcher_GrowthWeaponEventChangeInfinity() {}

// IDA_EA: 0x81E7D1C
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GrowthWeaponEventChangeInfinity::read(PacketBuf& packet,
                                                      MSG_BASE& message) {
    MSG_GROWTH_EQUIPMENT& request =
        static_cast<MSG_GROWTH_EQUIPMENT&>(message);
    if (packet.get_int(request.item_space) != 1) {
        return LineFunc(
            25596,
            "virtual int Dispatcher_GrowthWeaponEventChangeInfinity::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(request.slot) == 1) {
        return 0;
    }
    return LineFunc(
        25599,
        "virtual int Dispatcher_GrowthWeaponEventChangeInfinity::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81E7F2E
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::check_error(CUser *,MSG_BASE &)
int Dispatcher_GrowthWeaponEventChangeInfinity::check_error(CUser* user,
                                                             MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 19;
    }

    const MSG_GROWTH_EQUIPMENT& request =
        static_cast<const MSG_GROWTH_EQUIPMENT&>(message);
    const INVEN_TYPE inventory_type =
        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(request.item_space)));
    if (user->CheckItemLock(inventory_type, static_cast<int>(request.slot))) {
        return 213;
    }

    const GrowthWeaponEvent& event = user->growth_weapon_event_;
    if (event.charac_no_ != user->get_charac_no(-1) ||
        event.infinity_weapon_id_ != 0u) {
        return 3;
    }

    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user->GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    if (static_cast<int16_t>(event.server_group_) != server_group) {
        return 3;
    }
    if (static_cast<int16_t>(event.timepiece_) <
        G_CDataManager()->get_GrowthWeaponMaterialMax()) {
        return 17;
    }

    return GlobalData::s_event_manager->GetRepeatEvent(kGrowthWeaponEventId)
                   ->IsEventing(0) != 1;
}

// IDA_EA: 0x81E7DB4
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GrowthWeaponEventChangeInfinity::process(CUser* user,
                                                         MSG_BASE& message,
                                                         ParamBase& param) {
    stGrowthEquipment& response = static_cast<stGrowthEquipment&>(param);
    response.result = check_error(user, message);
    if (response.result > 0) {
        return 0;
    }
    if (response.result < 0) {
        return LineFunc(
            25615,
            "virtual int Dispatcher_GrowthWeaponEventChangeInfinity::process(CUser*, MSG_BASE&, ParamBase&)",
            response.result,
            0);
    }

    const MSG_GROWTH_EQUIPMENT& request =
        static_cast<const MSG_GROWTH_EQUIPMENT&>(message);
    const INVEN_TYPE inventory_type =
        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(request.item_space)));
    const Inven_Item* const source = user->getCurCharacInvenR()->GetInvenRef(
        inventory_type, static_cast<int>(request.slot));
    if (!source || source->item_id_ == 0) {
        response.result = 21;
        return 0;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(kGrowthWeaponEventId);
    if (repeat_event && repeat_event->IsEventing(0) == 1) {
        response.result = static_cast<GrowthEquipEvent*>(repeat_event)
                              ->SendInfinityGrowthWeapon(
                                  user,
                                  source,
                                  static_cast<int>(request.item_space),
                                  static_cast<int>(request.slot));
    }
    return 0;
}

// IDA_EA: 0x81E80BE
// IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::send(CUser *,ParamBase &)
void Dispatcher_GrowthWeaponEventChangeInfinity::send(CUser* user,
                                                      ParamBase& param) {
    const stGrowthEquipment& response =
        static_cast<const stGrowthEquipment&>(param);
    if (response.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_GROWTH_WEAPON_CHANGE_INFINITY,
            static_cast<unsigned char>(response.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_GROWTH_WEAPON_CHANGE_INFINITY);
}
