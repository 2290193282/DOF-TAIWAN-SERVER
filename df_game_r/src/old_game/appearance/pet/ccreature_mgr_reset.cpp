#include "old_game/appearance/pet/creature.h"
#include "old_game/appearance/pet/creature_fillpoint_runtime.h"

#include "common/manager/os_api.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <map>

namespace user_creature {

// IDA_EA: 0x8336F60
// IDA_NAME: _ZN13user_creature9CArtifact5ResetEv
// DEMANGLED: user_creature::CArtifact::Reset(void)
CArtifact* CArtifact::Reset() {
    item_pointer_ = 0;
    return this;
}

// IDA_EA: 0x833EF0C
// IDA_NAME: _ZN13user_creature9CArtifactC2Ev
// DEMANGLED: user_creature::CArtifact::CArtifact(void)
CArtifact::CArtifact() {
    Reset();
}

// IDA_EA: 0x8337050
// IDA_NAME: _ZN13user_creature13CCreatureItem9SetSlotNoEi
// DEMANGLED: user_creature::CCreatureItem::SetSlotNo(int)
CCreatureItem* CCreatureItem::SetSlotNo(int slot) {
    slot_no_ = slot;
    return this;
}

// IDA_EA: 0x833EFA4
// IDA_NAME: _ZN13user_creature13CCreatureItem9GetItemIdEv
// DEMANGLED: user_creature::CCreatureItem::GetItemId(void)
int CCreatureItem::GetItemId() {
    return item_id_;
}

// IDA_EA: 0x833EFCA
// IDA_NAME: _ZN13user_creature13CCreatureItem9GetSlotNoEv
// DEMANGLED: user_creature::CCreatureItem::GetSlotNo(void)
int CCreatureItem::GetSlotNo() {
    return slot_no_;
}

// IDA_EA: 0x8339196
// IDA_NAME: _ZN13user_creature12CCreatureMgrC2Ev
// DEMANGLED: user_creature::CCreatureMgr::CCreatureMgr(void)
CCreatureMgr::CCreatureMgr() : active_creature_(0), owner_(0) {
    Reset();
}

// IDA_EA: 0x86992CC
// IDA_NAME: _ZN13user_creature12CCreatureMgrC2ERKS0_
// DEMANGLED: user_creature::CCreatureMgr::CCreatureMgr(user_creature::CCreatureMgr const&)
CCreatureMgr::CCreatureMgr(const CCreatureMgr& other)
    : creature_items_(other.creature_items_),
      active_creature_(other.active_creature_),
      owner_(other.owner_),
      temporary_expire_time_(other.temporary_expire_time_),
      temporary_periods_(other.temporary_periods_),
      save_state_(other.save_state_) {
    // CArtifact is a one-pointer physical member; the original C2 copies the
    // exact 12-byte array after constructing the two maps.
    std::memcpy(artifacts_, other.artifacts_, sizeof(artifacts_));
}

// IDA_EA: 0x8339226
// IDA_NAME: _ZN13user_creature12CCreatureMgrD2Ev
// DEMANGLED: user_creature::CCreatureMgr::~CCreatureMgr(void)
CCreatureMgr::~CCreatureMgr() {
}

// IDA_EA: 0x869D614
// IDA_NAME: _ZN13user_creature12CCreatureMgraSERKS0_
// DEMANGLED: user_creature::CCreatureMgr::operator=(user_creature::CCreatureMgr const&)
CCreatureMgr& CCreatureMgr::operator=(const CCreatureMgr& other) {
    if (this != &other) {
        creature_items_ = other.creature_items_;
        active_creature_ = other.active_creature_;
        owner_ = other.owner_;
        artifacts_[0] = other.artifacts_[0];
        artifacts_[1] = other.artifacts_[1];
        artifacts_[2] = other.artifacts_[2];
        temporary_expire_time_ = other.temporary_expire_time_;
        temporary_periods_ = other.temporary_periods_;
        save_state_ = other.save_state_;
    }
    return *this;
}

// IDA_EA: 0x822D39A
// IDA_NAME: _ZN13user_creature12CCreatureMgr7SetUserEP5CUser
// DEMANGLED: user_creature::CCreatureMgr::SetUser(CUser *)
CCreatureMgr* CCreatureMgr::SetUser(CUser* user) {
    CreatureFillPoint::ClearCooldown(owner_);
    CreatureFillPoint::ClearCooldown(user);
    owner_ = user;
    return this;
}

// IDA_EA: 0x833A45A
// IDA_NAME: _ZN13user_creature12CCreatureMgr5ResetEv
// DEMANGLED: user_creature::CCreatureMgr::Reset(void)
CCreatureMgr* CCreatureMgr::Reset() {
    CCreatureMgr* const manager = this;
    CreatureFillPoint::ClearCooldown(owner_);
    active_creature_ = 0;
    owner_ = 0;
    temporary_expire_time_ = -1;

    if (!temporary_periods_.empty()) {
        temporary_periods_.clear();
    }
    if (!creature_items_.empty()) {
        for (CCreatureItemMap::iterator it = creature_items_.begin();
             it != creature_items_.end();
             ++it) {
            CCreatureItem* const item = it->second;
            if (item != 0 &&
                GetInstanceCreatureItemFactory()->FreeCreatureItem(item) !=
                    1) {
                LogManager::logFormat(1,
                                     "Creature.cpp",
                                     "void user_creature::CCreatureMgr::Reset()",
                                     0xB50,
                                     "FreeCreatureItem() error");
            }
        }
        creature_items_.clear();
    }

    for (int index = 0; index <= 2; ++index) {
        artifacts_[index].Reset();
    }
    save_state_ = 1;
    return manager;
}

// IDA_EA: 0x833926C
// IDA_NAME: _ZN13user_creature12CCreatureMgr24IsCreatureEquipmentScopeEi
// DEMANGLED: user_creature::CCreatureMgr::IsCreatureEquipmentScope(int)
int CCreatureMgr::IsCreatureEquipmentScope(int slot) {
    return static_cast<unsigned int>(slot) < 0x8cu;
}

// IDA_EA: 0x8531FBE
// IDA_NAME: _ZNK13user_creature12CCreatureMgr19GetCreatureItemSizeEv
// DEMANGLED: user_creature::CCreatureMgr::GetCreatureItemSize(void)const
int CCreatureMgr::GetCreatureItemSize() const {
    return static_cast<int>(creature_items_.size());
}

// IDA_EA: 0x833AD34
// IDA_NAME: _ZN13user_creature12CCreatureMgr11ChangeOwnerEP10Inven_Itemiiii
// DEMANGLED: user_creature::CCreatureMgr::ChangeOwner(Inven_Item *,int,int,int,int)
int CCreatureMgr::ChangeOwner(Inven_Item* item,
                              int slot,
                              int reason,
                              int old_charac_no,
                              int old_creature_flag) {
    if (IsCreatureEquipmentScope(slot) == 0 || item->item_category_ != 5u) {
        return 1;
    }

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    const int current_item_count = GetCreatureItemSize();
    const int charac_no = owner->getCurCharacNo();
    const int write_ok = sender.ChangeOwner(item,
                                             charac_no,
                                             slot,
                                             reason,
                                             current_item_count,
                                             old_charac_no,
                                             old_creature_flag);
    if (write_ok == 1) {
        sender.Send();
        CInventory* const inventory = static_cast<CInventory*>(
            owner->getCurCharacInvenR());
        const int item_id = item->item_id_;
        inventory->WriteCreatureLog(static_cast<INVEN_TYPE>(3),
                                    item_id,
                                    item->get_add_info(),
                                    1,
                                    item->item_category_,
                                    static_cast<eItemAddReason>(reason));
        return 1;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::ChangeOwner(Inven_Item*, int, int, int, int)",
        3110,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833AEDC
// IDA_NAME: _ZN13user_creature12CCreatureMgr12ChangeSlotNoEP10Inven_Itemii
// DEMANGLED: user_creature::CCreatureMgr::ChangeSlotNo(Inven_Item *,int,int)
int CCreatureMgr::ChangeSlotNo(Inven_Item* item, int empty_slot, int reason) {
    if (IsCreatureEquipmentScope(empty_slot) == 0 ||
        item->item_category_ != 5u) {
        return 1;
    }

    CCreatureItem* const creature_item =
        const_cast<CCreatureItem*>(FindCreatureItem(item->get_add_info()));
    if (creature_item == 0) {
        return 0;
    }
    creature_item->SetSlotNo(empty_slot);

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    const int charac_no = owner->getCurCharacNo();
    if (sender.ChangeSlotNo(item,
                            charac_no,
                            empty_slot,
                            reason) == 1) {
        sender.Send();
        const int item_id = item->item_id_;
        CInventory* const inventory = static_cast<CInventory*>(
            owner->getCurCharacInvenR());
        inventory->WriteCreatureLog(static_cast<INVEN_TYPE>(3),
                                    item_id,
                                    item->get_add_info(),
                                    1,
                                    item->item_category_,
                                    static_cast<eItemAddReason>(reason));
        return 1;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::ChangeSlotNo(Inven_Item*, int, int)",
        3140,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833A854
// IDA_NAME: _ZN13user_creature12CCreatureMgr22UnregisterCreatureItemEi
// DEMANGLED: user_creature::CCreatureMgr::UnregisterCreatureItem(int)
int CCreatureMgr::UnregisterCreatureItem(int key) {
    CCreatureItemMap::iterator it = creature_items_.find(key);
    if (it == creature_items_.end()) {
        CUser* const owner = owner_;
        cMyTrace trace(
            "bool user_creature::CCreatureMgr::UnregisterCreatureItem(int)",
            3009,
            5);
        trace("UnregisterCreatureItem failed. uid(%d), key(%d)\\n",
              owner->get_unique_id(),
              key);
        return 0;
    }

    GetInstanceCreatureItemFactory()->FreeCreatureItem(it->second);
    creature_items_.erase(it);
    return 1;
}

// IDA_EA: 0x833A77E
// IDA_NAME: _ZN13user_creature12CCreatureMgr20RegisterCreatureItemEPNS_13CCreatureItemEi
// DEMANGLED: user_creature::CCreatureMgr::RegisterCreatureItem(user_creature::CCreatureItem *,int)
int CCreatureMgr::RegisterCreatureItem(CCreatureItem* item, int key) {
    if (FindCreatureItem(key) != 0) {
        CUser* const owner = owner_;
        cMyTrace trace(
            "bool user_creature::CCreatureMgr::RegisterCreatureItem(user_creature::CCreatureItem*, int)",
            2996,
            5);
        trace("RegisterCreatureItem failed. uid(%d), key(%d)\\n",
              owner->get_unique_id(),
              key);
        return 0;
    }
    creature_items_.insert(std::make_pair(key, item));
    return 1;
}

// IDA_EA: 0x833A942
// IDA_NAME: _ZNK13user_creature12CCreatureMgr21IsExistedCreatureItemEi
// DEMANGLED: user_creature::CCreatureMgr::IsExistedCreatureItem(int)const
int CCreatureMgr::IsExistedCreatureItem(int slot) const {
    for (CCreatureItemMap::const_iterator it = creature_items_.begin();
         it != creature_items_.end();
         ++it) {
        CCreatureItem* const item = it->second;
        if (item == 0) {
            cMyTrace trace(
                "bool user_creature::CCreatureMgr::IsExistedCreatureItem(int) const",
                3034,
                5);
            trace("[%s][%d]",
                  "bool user_creature::CCreatureMgr::IsExistedCreatureItem(int) const",
                  3034);
        } else if (item->GetSlotNo() == slot) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x833B606
// IDA_NAME: _ZN13user_creature12CCreatureMgr18DeleteCreatureItemEii
// DEMANGLED: user_creature::CCreatureMgr::DeleteCreatureItem(int,int)
int CCreatureMgr::DeleteCreatureItem(int key, int reserved) {
    (void)reserved;
    return UnregisterCreatureItem(key);
}

// IDA_EA: 0x833AA32
// IDA_NAME: _ZN13user_creature12CCreatureMgr8SendMailEP10Inven_Itemii
// DEMANGLED: user_creature::CCreatureMgr::SendMail(Inven_Item *,int,int)
int CCreatureMgr::SendMail(Inven_Item* item, int slot, int creature_flag) {
    if (IsCreatureEquipmentScope(slot) == 0 || item->item_category_ != 5u) {
        return 1;
    }

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    const int write_ok = sender.SendMail(item,
                                         owner->getCurCharacNo(),
                                         slot,
                                         creature_flag,
                                         GetCreatureItemSize());
    if (write_ok == 1) {
        sender.Send();
        const int result = DeleteCreatureItem(item->get_add_info(), slot) == 1;
        return result;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::SendMail(Inven_Item*, int, int)",
        3053,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833AB9A
// IDA_NAME: _ZN13user_creature12CCreatureMgr8RecvMailEP10Inven_Itemii
// DEMANGLED: user_creature::CCreatureMgr::RecvMail(Inven_Item *,int,int)
int CCreatureMgr::RecvMail(Inven_Item* item, int slot, int creature_flag) {
    if (IsCreatureEquipmentScope(slot) == 0 || item->item_category_ != 5u) {
        return 1;
    }

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    const int write_ok = sender.RecvMail(item,
                                         owner->getCurCharacNo(),
                                         slot,
                                         creature_flag,
                                         GetCreatureItemSize());
    if (write_ok == 1) {
        sender.Send();
        const int item_id = item->item_id_;
        CInventory* const inventory = static_cast<CInventory*>(
            owner->getCurCharacInvenR());
        inventory->WriteCreatureLog(static_cast<INVEN_TYPE>(3),
                                    item_id,
                                    item->get_add_info(),
                                    1,
                                    item->item_category_,
                                    static_cast<eItemAddReason>(creature_flag));
        return 1;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::RecvMail(Inven_Item*, int, int)",
        3081,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833B09C
// IDA_NAME: _ZN13user_creature12CCreatureMgr13AuctionRegistEP10Inven_Itemii
// DEMANGLED: user_creature::CCreatureMgr::AuctionRegist(Inven_Item *,int,int)
int CCreatureMgr::AuctionRegist(Inven_Item* item,
                                int slot,
                                int creature_flag) {
    if (IsCreatureEquipmentScope(slot) == 0 || item->item_category_ != 5u) {
        return 1;
    }

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    const int write_ok = sender.AuctionRegist(item,
                                               owner->getCurCharacNo(),
                                               slot,
                                               creature_flag,
                                               GetCreatureItemSize());
    if (write_ok == 1) {
        sender.Send();
        const int result = DeleteCreatureItem(item->get_add_info(), slot) == 1;
        return result;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::AuctionRegist(Inven_Item*, int, int)",
        3164,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833B204
// IDA_NAME: _ZN13user_creature12CCreatureMgr18InsertCreatureItemEP10Inven_Itemiiii
// DEMANGLED: user_creature::CCreatureMgr::InsertCreatureItem(Inven_Item *,int,int,int,int)
int CCreatureMgr::InsertCreatureItem(Inven_Item* item,
                                     int slot,
                                     int creature_flag,
                                     int duration_days,
                                     int delete_tick) {
    if (IsCreatureEquipmentScope(slot) == 0 || item->item_category_ != 5u) {
        return 1;
    }

    CUser* const owner = owner_;
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(135), owner);
    const int write_ok = sender.InsertCreatureItem(owner,
                                                    item,
                                                    slot,
                                                    creature_flag,
                                                    GetCreatureItemSize(),
                                                    duration_days,
                                                    delete_tick);
    if (write_ok == 1) {
        sender.Send();
        const int expire_time = duration_days != 0 || delete_tick == 0
            ? 86400 * duration_days
            : delete_tick - OS_API::GetDateTimeTick();
        setTempExpireTime(expire_time);
        return 1;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::InsertCreatureItem(Inven_Item*, int, int, int, int)",
        3247,
        5);
    trace("cannot write db uid(%d)\\n", owner->get_unique_id());
    return 0;
}

// IDA_EA: 0x833B620
// IDA_NAME: _ZN13user_creature12CCreatureMgr22SetCreatureItemAddInfoEii
// DEMANGLED: user_creature::CCreatureMgr::SetCreatureItemAddInfo(int,int)
int CCreatureMgr::SetCreatureItemAddInfo(int slot, int add_info) {
    CUser* const owner = owner_;
    CInventory* const inventory = static_cast<CInventory*>(
        owner->getCurCharacInvenRefW());
    Inven_Item* const item = inventory->GetInvenRef(3, slot);
    if (!item || item->isEmpty()) {
        return 0;
    }
    item->set_add_info(add_info);
    return 1;
}

// IDA_EA: 0x833F10C
// IDA_NAME: _ZN13user_creature12CCreatureMgr17setTempExpireTimeEi
// DEMANGLED: user_creature::CCreatureMgr::setTempExpireTime(int)
CCreatureMgr* CCreatureMgr::setTempExpireTime(int expire_time) {
    temporary_expire_time_ = expire_time;
    return this;
}

}  // namespace user_creature
