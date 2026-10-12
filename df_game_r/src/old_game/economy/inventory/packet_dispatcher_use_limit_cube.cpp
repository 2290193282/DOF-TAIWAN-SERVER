#include "old_game/economy/inventory/packet_dispatcher_use_limit_cube.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstring>
#include <utility>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_UseLimitCube::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_UseLimitCube::process(CUser*, MSG_BASE&, ParamBase&)";

enum {
    kInventoryItemSpace = 0,
    kInventoryType = 1,
    kLimitCubeError = 17,
    kItemLockedError = 1,
    kLimitCubeDeleteReason = 8,
    kLimitCubeAddReason = 9,
};

void SendChangedSlots(CUser* user, const std::vector<int>& changed_slots) {
    int sent = 0;
    for (std::vector<int>::const_iterator slot = changed_slots.begin();
         slot != changed_slots.end() && sent <= 9;
         ++slot) {
        // The old local vector has triples (space, slot, count).  It only
        // publishes space and slot; the retained count is a transaction
        // record, not an outgoing field.
        const int item_space = *slot++;
        if (slot == changed_slots.end()) {
            break;
        }
        const int item_slot = *slot++;
        if (slot == changed_slots.end()) {
            break;
        }
        (void)*slot;
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(item_space),
                             item_slot);
        ++sent;
    }
}

}  // namespace

// IDA_EA: 0x822DD4E
// IDA_NAME: _ZN18MSG_USE_LIMIT_CUBEC2Ev
// DEMANGLED: MSG_USE_LIMIT_CUBE::MSG_USE_LIMIT_CUBE(void)
MSG_USE_LIMIT_CUBE::MSG_USE_LIMIT_CUBE() : MSG_BASE() {
    // The old C2 clears exactly the first dword after the MSG_BASE C2 call.
    // It intentionally does not initialize the three fields that read() must
    // fill from the C2S frame.
    std::memset(this, 0, sizeof(uint32_t));
}

// IDA_EA: 0x8231598
// IDA_NAME: _ZN14stUseLimitCubeC2Ev
// DEMANGLED: stUseLimitCube::stUseLimitCube(void)
stUseLimitCube::stUseLimitCube()
    : ParamBase(), error_code(0), reserved_05_07(), result_item_id(0),
      result_count(0), reserved_10(0) {}

// IDA_EA: 0x824703C
// IDA_NAME: _ZN14stUseLimitCubeD2Ev
// DEMANGLED: stUseLimitCube::~stUseLimitCube()
// IDA_EA: 0x824706C
// IDA_NAME: _ZN14stUseLimitCubeD0Ev
// DEMANGLED: stUseLimitCube::~stUseLimitCube()
stUseLimitCube::~stUseLimitCube() {}

// IDA_EA: 0x8597426
// IDA_NAME: _ZN23Dispatcher_UseLimitCubeC2Ev
// DEMANGLED: Dispatcher_UseLimitCube::Dispatcher_UseLimitCube(void)
Dispatcher_UseLimitCube::Dispatcher_UseLimitCube()
    : IPacketDispatcher<MSG_USE_LIMIT_CUBE,
                        stUseLimitCube,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x824532E
// IDA_NAME: _ZN23Dispatcher_UseLimitCubeD2Ev
// DEMANGLED: Dispatcher_UseLimitCube::~Dispatcher_UseLimitCube()
// IDA_EA: 0x824535E
// IDA_NAME: _ZN23Dispatcher_UseLimitCubeD0Ev
// DEMANGLED: Dispatcher_UseLimitCube::~Dispatcher_UseLimitCube()
Dispatcher_UseLimitCube::~Dispatcher_UseLimitCube() {}

// IDA_EA: 0x81D3BBC
// IDA_NAME: _ZN23Dispatcher_UseLimitCube11check_errorEijiR10Inven_ItemS1_PK5CItem
// DEMANGLED: Dispatcher_UseLimitCube::check_error(int,unsigned int,int,Inven_Item &,Inven_Item &,CItem const*)
int Dispatcher_UseLimitCube::check_error(
    int material_slot,
    unsigned int expected_material_item_id,
    int limit_cube_slot,
    Inven_Item& material,
    Inven_Item& limit_cube,
    const CItem* limit_cube_prototype) {
    (void)material_slot;
    (void)limit_cube_slot;
    if (!limit_cube_prototype || material.getKey() == 0 ||
        static_cast<unsigned int>(material.getKey()) !=
            expected_material_item_id ||
        limit_cube.getKey() == 0 ||
        limit_cube_prototype->is_stackable() != 1 ||
        limit_cube_prototype->vtable_->get_item_type(limit_cube_prototype) !=
            29 ||
        const_cast<CStackableItem*>(
            static_cast<const CStackableItem*>(limit_cube_prototype))
                ->is_included_material_for_limit_cube(
                    expected_material_item_id) != 1) {
        return kLimitCubeError;
    }
    return 0;
}

// IDA_EA: 0x81D3C62
// IDA_NAME: _ZN23Dispatcher_UseLimitCube4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseLimitCube::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseLimitCube::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_USE_LIMIT_CUBE& message =
        static_cast<MSG_USE_LIMIT_CUBE&>(base_message);
    if (packet.get_short(message.material_slot) != 1) {
        return LineFunc(13181, kReadSignature, 0, 0);
    }
    if (packet.get_int(message.expected_material_item_id) != 1) {
        return LineFunc(13185, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.limit_cube_slot) != 1) {
        return LineFunc(13189, kReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81D3D38
// IDA_NAME: _ZN23Dispatcher_UseLimitCube7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseLimitCube::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseLimitCube::process(CUser* user,
                                     MSG_BASE& base_message,
                                     ParamBase& base_param) {
    if (user->get_state() != static_cast<ch_state>(3)) {
        return 0;
    }

    const MSG_USE_LIMIT_CUBE& message =
        static_cast<const MSG_USE_LIMIT_CUBE&>(base_message);
    stUseLimitCube& result = static_cast<stUseLimitCube&>(base_param);
    CInventory* const read_inventory = user->getCurCharacInvenR();
    Inven_Item material = read_inventory->GetInvenSlot(
        kInventoryType, message.material_slot);
    Inven_Item limit_cube = read_inventory->GetInvenSlot(
        kInventoryType, message.limit_cube_slot);
    CStackableItem* const limit_cube_prototype =
        static_cast<CStackableItem*>(
            G_CDataManager()->find_item(limit_cube.getKey()));
    CItem* const material_prototype =
        G_CDataManager()->find_item(material.getKey());

    if (user->CheckItemLock(GetInvenTypeFromItemSpace(
                                static_cast<ENUM_ITEMSPACE>(kInventoryItemSpace)),
                            message.material_slot)) {
        result.error_code = kItemLockedError;
        return 0;
    }

    const int error = check_error(message.material_slot,
                                  message.expected_material_item_id,
                                  message.limit_cube_slot,
                                  material,
                                  limit_cube,
                                  limit_cube_prototype);
    if (error != 0) {
        result.error_code = static_cast<uint8_t>(error);
        return 0;
    }

    std::pair<int, int> selected_result;
    limit_cube_prototype->get_limit_cube_item(selected_result);
    if (selected_result.first == 0 && selected_result.second == 0) {
        result.error_code = kLimitCubeError;
        return 0;
    }
    result.result_item_id = static_cast<uint32_t>(selected_result.first);
    result.result_count = static_cast<uint32_t>(selected_result.second);

    const int first_material_count =
        limit_cube_prototype->get_1st_material_count(
            message.expected_material_item_id);
    const int second_material_item_id =
        limit_cube_prototype->get_2nd_material_index();
    const int second_material_count =
        limit_cube_prototype->get_2nd_material_count();
    std::vector<std::pair<int, int> > required_items;
    required_items.push_back(std::make_pair(
        static_cast<int>(message.expected_material_item_id),
        first_material_count));
    required_items.push_back(std::make_pair(limit_cube.getKey(), 1));
    required_items.push_back(std::make_pair(second_material_item_id,
                                            second_material_count));
    if (read_inventory->CheckNeedItemList(required_items) != 1) {
        result.error_code = kLimitCubeError;
        return 0;
    }

    CInventory trial_inventory;
    InventoryMemory trial_memory;
    trial_inventory.SetInventoryMemory(&trial_memory);
    trial_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (trial_inventory.tryDeleteEventItems(required_items) != 1) {
        result.error_code = kLimitCubeError;
        return 0;
    }

    uint8_t preserved_seal_or_state = 0;
    uint8_t preserved_reseal_count = 0;
    int preserved_extend_info = 0;
    int preserved_add_info = 0;
    if (!material_prototype || material_prototype->is_stackable() != 1) {
        preserved_seal_or_state = material.seal_or_state_;
        preserved_reseal_count = static_cast<uint8_t>(material.GetReSealCount());
        preserved_extend_info = material.extend_info_;
        preserved_add_info = material.get_add_info();
    }

    std::vector<int> changed_slots;
    CInventory* const write_inventory = user->getCurCharacInvenW();
    if (write_inventory->delete_item(
            static_cast<INVEN_TYPE>(kInventoryType),
            message.material_slot,
            first_material_count,
            static_cast<eItemDelReason>(kLimitCubeDeleteReason),
            true) == 1) {
        changed_slots.push_back(kInventoryItemSpace);
        changed_slots.push_back(message.material_slot);
        changed_slots.push_back(first_material_count);
    }
    if (write_inventory->delete_item(
            static_cast<INVEN_TYPE>(kInventoryType),
            message.limit_cube_slot,
            1,
            static_cast<eItemDelReason>(kLimitCubeDeleteReason),
            true) == 1) {
        changed_slots.push_back(kInventoryItemSpace);
        changed_slots.push_back(message.limit_cube_slot);
        changed_slots.push_back(1);
    }

    const int second_material_slot =
        read_inventory->check_item_exist(second_material_item_id);
    if (second_material_slot < 0) {
        result.error_code = kLimitCubeError;
        return 0;
    }
    if (write_inventory->delete_item(
            static_cast<INVEN_TYPE>(kInventoryType),
            second_material_slot,
            second_material_count,
            static_cast<eItemDelReason>(kLimitCubeDeleteReason),
            true) == 1) {
        changed_slots.push_back(kInventoryItemSpace);
        changed_slots.push_back(second_material_slot);
        changed_slots.push_back(second_material_count);
    }
    SendChangedSlots(user, changed_slots);

    Inven_Item generated;
    generated.item_id_ = selected_result.first;
    CItem* const generated_prototype =
        G_CDataManager()->find_item(selected_result.first);
    if (!generated_prototype) {
        result.error_code = kLimitCubeError;
        return 0;
    }
    generated_prototype->vtable_->make_item(generated_prototype, generated);
    if (generated_prototype->is_stackable() == 1) {
        generated.set_add_info(selected_result.second);
    } else {
        generated.seal_or_state_ = preserved_seal_or_state;
        generated.SetReSealCount(preserved_reseal_count);
        generated.extend_info_ = preserved_extend_info;
        generated.set_add_info(preserved_add_info);
    }

    const int inserted_slot = write_inventory->insertItemIntoInventory(
        generated,
        static_cast<eItemAddReason>(kLimitCubeAddReason),
        true,
        true);
    if (inserted_slot < 0) {
        result.error_code = kLimitCubeError;
        return 0;
    }
    result.error_code = 0;

    if (limit_cube.getKey() == 7942 &&
        (generated.getKey() <= 0x671d || generated.getKey() > 0x6749)) {
        LogManager::logFormat(1, "PacketDispatcher_Impl_1.cpp",
                              kProcessSignature, 0x344b,
                              "LIMIT CUBE : WRONG RESULT - cube_item(%u), result_item(%u)",
                              limit_cube.getKey(), generated.getKey());
    } else if (limit_cube.getKey() == 7953 &&
               (generated.getKey() <= 0x6749 ||
                generated.getKey() > 0x6775)) {
        LogManager::logFormat(1, "PacketDispatcher_Impl_1.cpp",
                              kProcessSignature, 0x3452,
                              "LIMIT CUBE : WRONG RESULT - cube_item(%u), result_item(%u)",
                              limit_cube.getKey(), generated.getKey());
    }
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(kInventoryItemSpace),
                         inserted_slot);
    return 0;
}

// IDA_EA: 0x81D4784
// IDA_NAME: _ZN23Dispatcher_UseLimitCube4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_UseLimitCube::send(CUser *,ParamBase &)
void Dispatcher_UseLimitCube::send(CUser* user, ParamBase& base_param) {
    const stUseLimitCube& result =
        static_cast<const stUseLimitCube&>(base_param);
    if (result.error_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_LIMIT_CUBE,
                                 result.error_code);
        return;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_LIMIT_CUBE);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(result.result_item_id));
    packet.put_short(static_cast<short>(result.result_count));
    packet.finalize(true);
    user->Send(packet);
}

// The old template owns the seven non-handler virtual members for the exact
// state-3 MSG_USE_LIMIT_CUBE/stUseLimitCube specialization.  Keep this as
// the sole owner, separate from the CMD339 class's concrete vtable.
// IDA_EA: 0x8251FAC
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8252108
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8252112
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825C646
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EED2Ev
// IDA_EA: 0x825C676
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EED0Ev
// IDA_EA: 0x825D74C
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598A22
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_USE_LIMIT_CUBE14stUseLimitCubeL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_USE_LIMIT_CUBE,
                                 stUseLimitCube,
                                 static_cast<ch_state>(3)>;
