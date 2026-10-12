#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/appearance/pet/creature.h"

namespace WongWork {

// IDA_EA: 0x822D22C
// IDA_NAME: _ZN8WongWork14CAvatarItemMgrC2Ev
// DEMANGLED: WongWork::CAvatarItemMgr::CAvatarItemMgr(void)
CAvatarItemMgr::CAvatarItemMgr() {
    Reset();
}

// IDA_EA: 0x869937C
// IDA_NAME: _ZN8WongWork14CAvatarItemMgrC2ERKS0_
// DEMANGLED: WongWork::CAvatarItemMgr::CAvatarItemMgr(WongWork::CAvatarItemMgr const&)
CAvatarItemMgr::CAvatarItemMgr(const CAvatarItemMgr& other)
    : assigned_items_(other.assigned_items_),
      wait_assign_items_(other.wait_assign_items_),
      next_temp_key_(other.next_temp_key_),
      default_emblem_(other.default_emblem_),
      default_emblem_alignment_(other.default_emblem_alignment_),
      emblem_reduce_rates_(other.emblem_reduce_rates_),
      pending_endurance_(other.pending_endurance_) {}

// IDA_EA: 0x822D2E0
// IDA_NAME: _ZN8WongWork14CAvatarItemMgrD2Ev
// DEMANGLED: WongWork::CAvatarItemMgr::~CAvatarItemMgr(void)
CAvatarItemMgr::~CAvatarItemMgr() {
    Reset();
}

// IDA_EA: 0x869D692
// IDA_NAME: _ZN8WongWork14CAvatarItemMgraSERKS0_
// DEMANGLED: WongWork::CAvatarItemMgr::operator=(WongWork::CAvatarItemMgr const&)
CAvatarItemMgr& CAvatarItemMgr::operator=(const CAvatarItemMgr& other) {
    if (this != &other) {
        assigned_items_ = other.assigned_items_;
        wait_assign_items_ = other.wait_assign_items_;
        next_temp_key_ = other.next_temp_key_;
        default_emblem_ = other.default_emblem_;
        default_emblem_alignment_ = other.default_emblem_alignment_;
        emblem_reduce_rates_ = other.emblem_reduce_rates_;
        pending_endurance_ = other.pending_endurance_;
    }
    return *this;
}

// IDA_EA: 0x82F9AE4
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr5ResetEv
// DEMANGLED: WongWork::CAvatarItemMgr::Reset(void)
int CAvatarItemMgr::Reset() {
    // IDA +48 is the next temporary avatar key; the original resets it before
    // checking the two item maps, even on the empty fast path.
    next_temp_key_ = 1;
    const int result = assigned_items_.empty() && wait_assign_items_.empty();
    if (!result) {
        // Registered items receive their own reset before returning to the
        // Avatar_Item pool. Waiting-assignment entries only return to that
        // pool, exactly as in the original two-loop body.
        for (std::map<int, Avatar_Item*>::iterator it = assigned_items_.begin();
             it != assigned_items_.end(); ++it) {
            it->second->init();
            m_AvatarItemPool.Free(it->second);
        }
        for (std::map<int, Avatar_Item*>::iterator it =
                 wait_assign_items_.begin();
             it != wait_assign_items_.end(); ++it) {
            m_AvatarItemPool.Free(it->second);
        }

        assigned_items_.clear();
        wait_assign_items_.clear();
        default_emblem_.init();
        emblem_reduce_rates_.clear();
        pending_endurance_.clear();
    }
    return result;
}

}  // namespace WongWork
