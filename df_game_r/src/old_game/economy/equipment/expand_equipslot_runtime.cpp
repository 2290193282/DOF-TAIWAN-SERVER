#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "common/config.h"

#include "old_game/player/core/character_view_data.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/inventory/expand_equipslot_payload.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_expand_equipslot.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstring>
#include <new>
#include <stdint.h>

namespace {

const int kExpandEquipSlotCount = 12;
const int kExpandEquipSlotTableCount = 3;
const int kExpandEquipSlotBytes = 0x2DC;

}  // namespace

// IDA_EA: 0x84992C0
// IDA_NAME: _ZN16CExpandEquipslotC2Ev
// DEMANGLED: CExpandEquipslot::CExpandEquipslot(void)
CExpandEquipslot::CExpandEquipslot() {
    for (int slot = 0; slot <= 11; ++slot) {
        new (&expand_equipslots_[slot]) Inven_Item;
    }
    for (int slot = 12; slot <= 23; ++slot) {
        new (&expand_equipslots_[slot]) Inven_Item;
    }
    for (int slot = 24; slot <= 35; ++slot) {
        new (&expand_equipslots_[slot]) Inven_Item;
    }
}

// IDA_EA: 0x849935C
// IDA_NAME: _ZN16CExpandEquipslotD2Ev
// DEMANGLED: CExpandEquipslot::~CExpandEquipslot()
// IDA_EA: 0x849938C
// IDA_NAME: _ZN16CExpandEquipslotD0Ev
// DEMANGLED: CExpandEquipslot::~CExpandEquipslot()
CExpandEquipslot::~CExpandEquipslot() {}

// IDA_EA: 0x84993AA
// IDA_NAME: _ZNK16CExpandEquipslot19getExpandEquipslotRE14ENUM_EQUIPSLOT
// DEMANGLED: CExpandEquipslot::getExpandEquipslotR(ENUM_EQUIPSLOT)const
const Inven_Item* CExpandEquipslot::getExpandEquipslotR(
    ENUM_EQUIPSLOT equip_slot) const {
    switch (static_cast<int>(equip_slot)) {
        case 1:
            return &expand_equipslots_[0];
        case 2:
            return &expand_equipslots_[12];
        case 3:
            return &expand_equipslots_[24];
        default:
            return 0;
    }
}

// IDA_EA: 0x84993E2
// IDA_NAME: _ZN16CExpandEquipslot19getExpandEquipslotWE14ENUM_EQUIPSLOT
// DEMANGLED: CExpandEquipslot::getExpandEquipslotW(ENUM_EQUIPSLOT)
Inven_Item* CExpandEquipslot::getExpandEquipslotW(
    ENUM_EQUIPSLOT equip_slot) {
    switch (static_cast<int>(equip_slot)) {
        case 1:
            alter();
            return &expand_equipslots_[0];
        case 2:
            alter();
            return &expand_equipslots_[12];
        case 3:
            alter();
            return &expand_equipslots_[24];
        default:
            return 0;
    }
}

// IDA_EA: 0x849943E
// IDA_NAME: _ZN16CExpandEquipslot15EquipslotSwitchEP5CUserccc
// DEMANGLED: CExpandEquipslot::EquipslotSwitch(CUser *,char,char,char)
int CExpandEquipslot::EquipslotSwitch(CUser* user,
                                      char requested_switch,
                                      char channel_switch,
                                      char expand_switch) {
    if (!user) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::EquipslotSwitch(CUser*, char, char, char)",
            68,
            "Fail Equipslot Switch : pUser is NULL");
        return 1;
    }
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::EquipslotSwitch(CUser*, char, char, char)",
            74,
            "Fail Equipslot Switch : pUser is not valid state");
        return 19;
    }

    const char current_channel_switch =
        static_cast<char>(user->getCurChannelEquipslotSwitch());
    const char current_expand_switch =
        static_cast<char>(user->getCurExpandEquipslotSwitch());
    if (current_channel_switch != channel_switch ||
        current_expand_switch != expand_switch) {
        return 1;
    }

    item_lock::CItemLock* const item_lock_data =
        static_cast<item_lock::CItemLock*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    std::vector<item_lock::stItemLockRef>& item_lock_refs =
        item_lock_data->GetItemLockRefVec();
    for (std::vector<item_lock::stItemLockRef>::iterator it =
             item_lock_refs.begin();
         it != item_lock_refs.end();
         ++it) {
        if (it->slot >=
                static_cast<unsigned int>(
                    oldsrv::config::kOrdinaryEquipmentTypeFirst) &&
            it->slot <=
                static_cast<unsigned int>(
                    oldsrv::config::kOrdinaryEquipmentTypeLast)) {
            if (it->item_space == 3u) {
                it->item_space = 18u;
            } else if (it->item_space == 18u) {
                it->item_space = 3u;
            }
        }
    }

    if (requested_switch) {
        return 1;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item switched_items[kExpandEquipSlotCount];
    Inven_Item current_equipment_slot;
    Inven_Item current_expand_slot;
    current_equipment_slot.setCopy(inventory->GetInvenSlot(
        0, oldsrv::config::kEquipmentTypeTitle));
    current_expand_slot.setCopy(expand_equipslots_[1]);

    inventory->GetInvenData(5,
                            switched_items,
                            kExpandEquipSlotBytes);
    inventory->SetInvenData(5,
                            &expand_equipslots_[0],
                            kExpandEquipSlotBytes);
    SetExpandEquipslot(1, switched_items, kExpandEquipSlotBytes);

    inventory->GetInvenRef(0, oldsrv::config::kEquipmentTypeTitle)
        ->setCopy(current_equipment_slot);
    expand_equipslots_[1].setCopy(current_expand_slot);

    CParty* const party = user->GetParty();
    if (party) {
        party->Reset_party_overlapped_drop_ratio();
    }

    const Inven_Item weapon = inventory->GetInvenSlot(
        0, oldsrv::config::kEquipmentTypeWeapon);
    if (weapon.item_id_ != 0 || expand_equipslots_[0].item_id_ != 0) {
        user->character_view().enableSaveCharacView();
        user->send_equip(oldsrv::config::kEquipmentTypeWeapon);
    }

    if (current_channel_switch == 0) {
        user->setCurChannelEquipslotSwitch(1);
    } else if (current_channel_switch == 1) {
        user->setCurChannelEquipslotSwitch(0);
    }
    return 0;
}

// IDA_EA: 0x849983E
// IDA_NAME: _ZN16CExpandEquipslot15Send_Equip_InfoEP5CUser
// DEMANGLED: CExpandEquipslot::Send_Equip_Info(CUser *)
int CExpandEquipslot::Send_Equip_Info(CUser* user) {
    if (!user) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Send_Equip_Info(CUser*)",
            154,
            "Fail Send_Equip_Info : pUser is NULL");
        return 1;
    }
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Send_Equip_Info(CUser*)",
            160,
            "Fail Send_Equip_Info : pUser is not valid state");
        return 19;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x8499986
// IDA_NAME: _ZN16CExpandEquipslot18SetExpandEquipslotEiPvi
// DEMANGLED: CExpandEquipslot::SetExpandEquipslot(int,void *,int)
charac_expand::CData* CExpandEquipslot::SetExpandEquipslot(
    int equip_slot,
    void* source,
    int bytes) {
    charac_expand::CData* result =
        reinterpret_cast<charac_expand::CData*>(
            static_cast<intptr_t>(equip_slot));
    if (equip_slot == 2) {
        if (bytes == kExpandEquipSlotBytes) {
            std::memcpy(&expand_equipslots_[12],
                        source,
                        kExpandEquipSlotBytes);
            return alter();
        }
    } else if (equip_slot == 3) {
        if (bytes == kExpandEquipSlotBytes) {
            std::memcpy(&expand_equipslots_[24],
                        source,
                        kExpandEquipSlotBytes);
            return alter();
        }
    } else if (equip_slot == 1 && bytes == kExpandEquipSlotBytes) {
        std::memcpy(&expand_equipslots_[0],
                    source,
                    kExpandEquipSlotBytes);
        return alter();
    }
    return result;
}

// IDA_EA: 0x8499A48
// IDA_NAME: _ZN16CExpandEquipslot12Change_EquipEP5CUseriiii
// DEMANGLED: CExpandEquipslot::Change_Equip(CUser *,int,int,int,int)
int CExpandEquipslot::Change_Equip(CUser* user,
                                   int source_space,
                                   int source_slot,
                                   int target_space,
                                   int target_slot) {
    if (!user) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Change_Equip(CUser*, int, int, int, int)",
            216,
            "Fail Change_Equip : pUser is NULL");
        return 1;
    }
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Change_Equip(CUser*, int, int, int, int)",
            222,
            "Fail Change_Equip : pUser is not valid state");
        return 19;
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    if (source_space != 0) {
        return 1;
    }

    if (target_space != 18) {
        return 1;
    }
    if (static_cast<unsigned char>(
            writable_inventory->CheckValidSlot(1, source_slot)) != 1u) {
        return 1;
    }
    if (static_cast<unsigned char>(
            writable_inventory->CheckValidSlot(5, target_slot)) != 1u) {
        return 1;
    }
    const Inven_Item* const source_item =
        user->getCurCharacInvenR()->GetInvenRef(1, source_slot);
    if (!source_item) {
        return 1;
    }
    const unsigned int item_id =
        static_cast<uint32_t>(source_item->item_id_);
    const CItem* const item = G_CDataManager()->find_item(item_id);
    if (static_cast<unsigned char>(
            user->getCurChannelEquipslotSwitch()) == 0u &&
        item &&
        !item->IsEnableWorld(static_cast<ENUM_WORLD_TYPE>(1))) {
        return 1;
    }

    const Inven_Item security_item =
        writable_inventory->GetInvenSlot(1, source_slot);
    if (security_item.seal_or_state_) {
        const int security_result =
            GlobalData::s_pSecuProtectionField->Check(
                user,
                static_cast<SECURITY_PROTCTION>(22));
        if (security_result) {
            return security_result;
        }
    }

    // Item lock is retained by destructive item consumers; equipping into an
    // expanded slot is a reversible CMD19 move and must remain allowed.

    const int move_result =
        Move_Item(user,
                  0,
                  source_slot,
                  18,
                  target_slot - oldsrv::config::kOrdinaryEquipmentTypeFirst);
    if (move_result) {
        return move_result;
    }
    alter();
    return 0;
}

// IDA_EA: 0x8499D1A
// IDA_NAME: _ZNK16CExpandEquipslot12GetInvenSlotE10INVEN_TYPEi
// DEMANGLED: CExpandEquipslot::GetInvenSlot(INVEN_TYPE,int)const
Inven_Item CExpandEquipslot::GetInvenSlot(INVEN_TYPE inventory_type,
                                           int slot) const {
    if (slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
        slot <= oldsrv::config::kOrdinaryEquipmentTypeLast &&
        static_cast<int>(inventory_type) == 5) {
        return expand_equipslots_[
            slot - oldsrv::config::kOrdinaryEquipmentTypeFirst];
    }
    return Inven_Item();
}

// IDA_EA: 0x8499E90
// IDA_NAME: _ZNK16CExpandEquipslot12GetInvenRefRE10INVEN_TYPEi
// DEMANGLED: CExpandEquipslot::GetInvenRefR(INVEN_TYPE,int)const
const Inven_Item* CExpandEquipslot::GetInvenRefR(
    INVEN_TYPE inventory_type,
    int slot) const {
    if (slot < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        slot > oldsrv::config::kOrdinaryEquipmentTypeLast) {
        return 0;
    }
    if (static_cast<int>(inventory_type) == 5) {
        return &expand_equipslots_[
            slot - oldsrv::config::kOrdinaryEquipmentTypeFirst];
    }
    return 0;
}

// IDA_EA: 0x8499EC6
// IDA_NAME: _ZN16CExpandEquipslot12GetInvenRefWE10INVEN_TYPEi
// DEMANGLED: CExpandEquipslot::GetInvenRefW(INVEN_TYPE,int)
Inven_Item* CExpandEquipslot::GetInvenRefW(INVEN_TYPE inventory_type,
                                            int slot) {
    if (slot < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        slot > oldsrv::config::kOrdinaryEquipmentTypeLast) {
        return 0;
    }
    if (static_cast<int>(inventory_type) != 5) {
        return 0;
    }
    alter();
    return &expand_equipslots_[
        slot - oldsrv::config::kOrdinaryEquipmentTypeFirst];
}

// IDA_EA: 0x8499F0A
// IDA_NAME: _ZNK16CExpandEquipslot12FindItemLockEhR14ENUM_ITEMSPACERi
// DEMANGLED: CExpandEquipslot::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
int CExpandEquipslot::FindItemLock(unsigned char lock,
                                   ENUM_ITEMSPACE& item_space,
                                   int& slot) const {
    for (slot = 0; slot <= 11; ++slot) {
        if (static_cast<unsigned char>(
                expand_equipslots_[slot].amplify_option_.GetLock()) == lock) {
            slot += oldsrv::config::kOrdinaryEquipmentTypeFirst;
            item_space = static_cast<ENUM_ITEMSPACE>(18);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8499F84
// IDA_NAME: _ZN16CExpandEquipslot9Move_ItemEP5CUseriiii
// DEMANGLED: CExpandEquipslot::Move_Item(CUser *,int,int,int,int)
int CExpandEquipslot::Move_Item(CUser* user,
                                int source_space,
                                int source_slot,
                                int target_space,
                                int target_slot) {
    if (!user) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Move_Item(CUser*, int, int, int, int)",
            380,
            "Fail Move_Item : pUser is NULL");
        return 17;
    }
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Move_Item(CUser*, int, int, int, int)",
            386,
            "Fail Move_Item : pUser is not valid state");
        return 17;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (source_space != 0) {
        return 17;
    }
    if (target_space != 18) {
        return 17;
    }

    Inven_Item* const source = inventory->GetInvenRef(1, source_slot);
    Inven_Item* const target = &expand_equipslots_[target_slot];
    if (!CheckEquipable(*source, *target)) {
        return 17;
    }

    if (source->amplify_option_.isIdentified() != 1 ||
        target->amplify_option_.isIdentified() != 1) {
        return 17;
    }

    Inven_Item moving_item;
    moving_item.setCopy(*source);
    const unsigned int item_id =
        static_cast<uint32_t>(moving_item.item_id_);
    if (item_id) {
        const CItem* const item = G_CDataManager()->find_item(item_id);
        if (!item) {
            cMyTrace trace(
                "int CExpandEquipslot::Move_Item(CUser*, int, int, int, int)",
                416,
                5);
            trace("CExpandEquipslot::Move_Item Move_Item return false (%d)",
                  static_cast<signed char>(moving_item.item_id_));
            return 17;
        }

        const CItemVTable* const item_vtable =
            *reinterpret_cast<const CItemVTable* const*>(item);
        if (item_vtable->is_avatar_item(item)) {
            return 17;
        }
        if (item_vtable->is_creature_item(item)) {
            return 17;
        }

        const int equip_result = IsEquipable(
            user,
            item,
            target_slot + oldsrv::config::kOrdinaryEquipmentTypeFirst);
        if (equip_result) {
            return equip_result;
        }

        if (item_vtable->period_after_unsealing(item)) {
            const int expiration =
                GlobalData::s_systemTime_.getCurSec() +
                86400 * item_vtable->period_after_unsealing(item);
            if (expiration) {
                moving_item.avatar_select_ = static_cast<uint16_t>(
                    static_cast<int16_t>(
                        (expiration - 1151683200) / 86400));
            }
        }
    }

    source->setCopy(*target);
    moving_item.seal_or_state_ = 0;
    target->setCopy(moving_item);
    return 0;
}

// IDA_EA: 0x849A2CA
// IDA_NAME: _ZN16CExpandEquipslot8loadDataEP5CUserPc
// DEMANGLED: CExpandEquipslot::loadData(CUser *,char *)
bool CExpandEquipslot::loadData(CUser* user, char* data) {
    static_cast<void>(user);
    _reset();
    int equip_slot = 1;
    for (unsigned int index = 0; index <= 35; ++index) {
        if (index && !(index % 12)) {
            if (index == 12) {
                equip_slot = 2;
            } else if (index > 12) {
                equip_slot = 3;
            }
        }
        if (equip_slot == 1) {
            expand_equipslots_[index].setCopy(
                *reinterpret_cast<const Inven_Item*>(
                    data + sizeof(Inven_Item) * index + 4));
        } else if (equip_slot == 2) {
            expand_equipslots_[12 + index % 12].setCopy(
                *reinterpret_cast<const Inven_Item*>(
                    data + sizeof(Inven_Item) * index + 4));
        } else {
            expand_equipslots_[24 + index % 12].setCopy(
                *reinterpret_cast<const Inven_Item*>(
                    data + sizeof(Inven_Item) * index + 4));
        }
    }
    alter();
    return true;
}

// IDA_EA: 0x849A436
// IDA_NAME: _ZNK16CExpandEquipslot7getDataEPc
// DEMANGLED: CExpandEquipslot::getData(char *)const
void CExpandEquipslot::getData(char* data) const {
    int equip_slot = 1;
    for (unsigned int index = 0; index <= 35; ++index) {
        if (index && !(index % 12)) {
            if (index == 12) {
                equip_slot = 2;
            } else if (index > 12) {
                equip_slot = 3;
            }
        }
        Inven_Item* const destination =
            reinterpret_cast<Inven_Item*>(
                data + sizeof(Inven_Item) * index + 4);
        if (equip_slot == 1) {
            destination->setCopy(expand_equipslots_[index]);
        } else if (equip_slot == 2) {
            destination->setCopy(expand_equipslots_[12 + index % 12]);
        } else {
            destination->setCopy(expand_equipslots_[24 + index % 12]);
        }
    }
}

// IDA_EA: 0x849A57C
// IDA_NAME: _ZN16CExpandEquipslot22Send_Expand_Equip_InfoEP5CUser
// DEMANGLED: CExpandEquipslot::Send_Expand_Equip_Info(CUser *)
int CExpandEquipslot::Send_Expand_Equip_Info(CUser* user) {
    if (!user) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Send_Expand_Equip_Info(CUser*)",
            539,
            "Fail Send_Expand_Equip_Info : pUser is NULL");
        return 1;
    }
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "ExpandEquipslot.cpp",
            "int CExpandEquipslot::Send_Expand_Equip_Info(CUser*)",
            545,
            "Fail Send_Expand_Equip_Info : pUser is NULL");
        return 19;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 339);
    packet.put_short(user->get_unique_id());
    for (unsigned char equip_slot = 1; equip_slot <= 3; ++equip_slot) {
        if (equip_slot == 1 &&
            static_cast<unsigned char>(
                user->getCurChannelEquipslotSwitch()) == 1u) {
            packet.put_byte(0);
        } else {
            packet.put_byte(equip_slot);
        }
        user->getCurCharacInvenR()->MakeEquipList(
            &packet,
            true,
            static_cast<ENUM_USERINFO>(1),
            static_cast<ENUM_EQUIPSLOT>(equip_slot));
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x849A75A
// IDA_NAME: _ZN16CExpandEquipslot9_saveDataEP5CUser
// DEMANGLED: CExpandEquipslot::_saveData(CUser *)
int CExpandEquipslot::_saveData(CUser* user) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "ExpandEquipslot.cpp", 583);
    CStreamGuard guard(stream, true);
    *guard << 0x26D;
    *guard << user->GetUID();
    const int item_count_per_slot = 12;
    const int item_count = 36;
    static_cast<void>(item_count_per_slot);
    static_cast<void>(item_count);

    SIG_SAVE_EXPAND_CHARAC* const payload =
        guard->GetInBuffer<SIG_SAVE_EXPAND_CHARAC>();
    if (!payload) {
        return 0;
    }
    std::memset(payload, 0, sizeof(*payload));
    payload->equipslot.charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));

    int equip_slot = 1;
    for (unsigned int index = 0; index <= 35; ++index) {
        if (index && !(index % 12)) {
            if (index == 12) {
                equip_slot = 2;
            } else if (index > 12) {
                equip_slot = 3;
            }
        }
        if (equip_slot == 1) {
            payload->equipslot.expand_equipslot_items[index].setCopy(
                expand_equipslots_[index]);
        } else if (equip_slot == 2) {
            payload->equipslot.expand_equipslot_items[index].setCopy(
                expand_equipslots_[12 + index % 12]);
        } else if (equip_slot == 3) {
            payload->equipslot.expand_equipslot_items[index].setCopy(
                expand_equipslots_[24 + index % 12]);
        }
    }
    payload->stat.channel_equipslot_switch =
        static_cast<int8_t>(user->getCurChannelEquipslotSwitch());
    payload->stat.expand_equipslot_switch =
        static_cast<int8_t>(user->getCurExpandEquipslotSwitch());
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x849A9E2
// IDA_NAME: _ZN16CExpandEquipslot6_resetEv
// DEMANGLED: CExpandEquipslot::_reset(void)
int CExpandEquipslot::_reset() {
    for (int slot = 0; slot <= 11; ++slot) {
        expand_equipslots_[slot].reset();
        expand_equipslots_[12 + slot].reset();
        expand_equipslots_[24 + slot].reset();
    }
    return 0;
}
