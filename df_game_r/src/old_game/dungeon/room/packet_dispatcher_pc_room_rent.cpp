#include "old_game/dungeon/room/packet_dispatcher_pc_room_rent.h"

#include "common/config.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/room/pc_room_public_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x822E334
// IDA_NAME: _ZN13MSG_RENT_ITEMC2Ev
// DEMANGLED: MSG_RENT_ITEM::MSG_RENT_ITEM(void)
MSG_RENT_ITEM::MSG_RENT_ITEM()
    : MSG_BASE(), selection_count(0), rental_type(0), selections() {}

// IDA_EA: 0x8231CDA
// IDA_NAME: _ZN15stRentItemParamC2Ev
// DEMANGLED: stRentItemParam::stRentItemParam(void)
stRentItemParam::stRentItemParam()
    : ParamBase(), error_code(0), rent_output() {}

// IDA_EA: 0x82466FC
// IDA_NAME: _ZN15stRentItemParamD2Ev
// DEMANGLED: stRentItemParam::~stRentItemParam()
// IDA_EA: 0x824675E
// IDA_NAME: _ZN15stRentItemParamD0Ev
// DEMANGLED: stRentItemParam::~stRentItemParam()
stRentItemParam::~stRentItemParam() {}

// IDA_EA: 0x8597D8E
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItemC2Ev
// DEMANGLED: Dispatcher_PcRoomRentItem::Dispatcher_PcRoomRentItem(void)
Dispatcher_PcRoomRentItem::Dispatcher_PcRoomRentItem()
    : IPacketDispatcher<MSG_RENT_ITEM,
                        stRentItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243D8C
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItemD2Ev
// DEMANGLED: Dispatcher_PcRoomRentItem::~Dispatcher_PcRoomRentItem()
// IDA_EA: 0x8243DBC
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItemD0Ev
// DEMANGLED: Dispatcher_PcRoomRentItem::~Dispatcher_PcRoomRentItem()
Dispatcher_PcRoomRentItem::~Dispatcher_PcRoomRentItem() {}

// IDA_EA: 0x81E512E
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PcRoomRentItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PcRoomRentItem::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_RENT_ITEM& message = static_cast<MSG_RENT_ITEM&>(base_message);
    unsigned char rental_type = 0;
    if (packet.get_byte(rental_type) != 1) {
        return LineFunc(24124,
                        "virtual int Dispatcher_PcRoomRentItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    message.rental_type = static_cast<int8_t>(rental_type);

    unsigned char selection_count = 0;
    if (packet.get_byte(selection_count) != 1) {
        return LineFunc(24128,
                        "virtual int Dispatcher_PcRoomRentItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    message.selection_count = static_cast<int8_t>(selection_count);
    if (message.selection_count != 1) {
        return LineFunc(24131,
                        "virtual int Dispatcher_PcRoomRentItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    for (int index = 0; message.selection_count > index; ++index) {
        if (packet.get_byte(message.selections[index].section_index) != 1) {
            return LineFunc(24136,
                            "virtual int Dispatcher_PcRoomRentItem::read(PacketBuf&, MSG_BASE&)",
                            0,
                            0);
        }
        if (packet.get_byte(message.selections[index].package_index) != 1) {
            return LineFunc(24138,
                            "virtual int Dispatcher_PcRoomRentItem::read(PacketBuf&, MSG_BASE&)",
                            0,
                            0);
        }
    }
    return 0;
}

// IDA_EA: 0x81E52C0
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PcRoomRentItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PcRoomRentItem::process(CUser* user,
                                       MSG_BASE& base_message,
                                       ParamBase& base_param) {
    MSG_RENT_ITEM& message = static_cast<MSG_RENT_ITEM&>(base_message);
    stRentItemParam& param = static_cast<stRentItemParam&>(base_param);
    param.error_code = check_error(user, message);
    if (param.error_code != 0) {
        return 0;
    }

    InstanceRentalSystem::RentInput input;
    input.rental_type = message.rental_type;
    for (int index = 0; message.selection_count > index; ++index) {
        InstanceRentalSystem::RentInput::RentInElem selection;
        selection.section_index = message.selections[index].section_index;
        selection.package_index = message.selections[index].package_index;
        input.entries.push_back(selection);
    }
    giveDimensionItems(user, input);
    param.error_code = GlobalData::s_rentalSystem->Rent(*user,
                                                        input,
                                                        param.rent_output);
    if (param.error_code == 0) {
        user->UpdateCurrentCharacRentalInfo(param.rent_output,
                                            message.rental_type,
                                            false);
    }
    return 0;
}

// IDA_EA: 0x81E542E
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PcRoomRentItem::send(CUser *,ParamBase &)
void Dispatcher_PcRoomRentItem::send(CUser* user, ParamBase& base_param) {
    const stRentItemParam& param =
        static_cast<const stRentItemParam&>(base_param);
    bool equipment_changed = false;

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_PC_ROOM_RENT_ITEM);
    if (param.error_code != 0) {
        response.put_byte(0);
        response.put_byte(static_cast<unsigned char>(param.error_code));
    } else {
        response.put_byte(1);
        response.put_byte(static_cast<int>(param.rent_output.deleted_items.size()));
        for (std::vector<InstanceRentalSystem::DeletedRentItem>::const_iterator
                 item = param.rent_output.deleted_items.begin();
             item != param.rent_output.deleted_items.end();
             ++item) {
            if (item->is_equipment) {
                response.put_byte(3);
                if (item->slot >=
                        oldsrv::config::kOrdinaryEquipmentTypeFirst &&
                    item->slot <=
                        oldsrv::config::kOrdinaryEquipmentTypeLast) {
                    equipment_changed = true;
                }
            } else {
                response.put_byte(0);
            }
            response.put_short(item->slot);
        }
    }
    response.finalize(true);
    user->Send(response);

    if (param.error_code == 0) {
        PacketGuard inventory_update;
        inventory_update.put_header(0, 14);
        inventory_update.put_byte(0);
        inventory_update.put_short(
            static_cast<int>(param.rent_output.rental_infos.size()));
        for (std::vector<InstanceRentalSystem::RentalInfo>::const_iterator
                 item = param.rent_output.rental_infos.begin();
             item != param.rent_output.rental_infos.end();
             ++item) {
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(1), item->slot, inventory_update);
        }
        inventory_update.finalize(true);
        user->Send(inventory_update);
    }
    if (equipment_changed) {
        user->send_equip(oldsrv::config::kEquipmentTypeWeapon);
    }
}

// IDA_EA: 0x81E5762
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_PcRoomRentItem::check_error(CUser *,MSG_BASE &)
int Dispatcher_PcRoomRentItem::check_error(CUser* user,
                                           MSG_BASE& base_message) {
    const MSG_RENT_ITEM& message =
        static_cast<const MSG_RENT_ITEM&>(base_message);
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return 3;
    }
    if (message.rental_type <= 0 || message.rental_type > 2) {
        return 8;
    }
    if (user->GetPCRoomNo() == 0 && message.rental_type == 1) {
        return 8;
    }
    if (message.rental_type != 2) {
        return 0;
    }
    return GlobalData::s_event_manager->GetRepeatEvent(94)->IsEventing(0) == 1
               ? 0
               : 8;
}

// IDA_EA: 0x81E583E
// IDA_NAME: _ZN25Dispatcher_PcRoomRentItem18giveDimensionItemsEP5CUserRN20InstanceRentalSystem9RentInputE
// DEMANGLED: Dispatcher_PcRoomRentItem::giveDimensionItems(CUser *,InstanceRentalSystem::RentInput &)
void Dispatcher_PcRoomRentItem::giveDimensionItems(
    CUser* user,
    InstanceRentalSystem::RentInput& input) {
    if (!user || input.rental_type != 2) {
        return;
    }

    int selection_index = 0;
    if (pc_room::GetSelectionIndex(*user, selection_index, 2) != 1) {
        return;
    }
    std::vector<EventRewardItemInfo>* const rewards =
        CDataManagerDimensionActivationData(G_CDataManager())
            ->reward_data.getRewardGiveItems(selection_index,
                                              user->get_charac_job(),
                                              user->getCurCharFirstGrowType());
    if (!rewards) {
        return;
    }
    for (std::vector<EventRewardItemInfo>::size_type index = 0;
         index != rewards->size();
         ++index) {
        InstanceRentalSystem::RentInput::RentInElem reward;
        reward.section_index = static_cast<uint8_t>(selection_index);
        reward.package_index = static_cast<uint8_t>(index);
        reward.rent_type = 1;
        input.entries.push_back(reward);
    }
}
