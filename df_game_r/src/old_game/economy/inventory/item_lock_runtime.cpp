#include "old_game/economy/inventory/item_lock_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/config.h"

#include "old_game/economy/inventory/item_lock_payload.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "common/manager/global_data.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_item_lock.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kItemUnlockWaitSeconds = 259200;

}  // namespace

namespace item_lock {

// IDA_EA: 0x84F4252
// IDA_NAME: _ZN9item_lock13stItemLockRefD2Ev
// DEMANGLED: item_lock::stItemLockRef::~stItemLockRef()
stItemLockRef::~stItemLockRef() {}

// IDA_EA: 0x854147C
// IDA_NAME: _ZN9item_lock14CItemLockTimer21IncreaseItemLockTimerEP5CUser
// DEMANGLED: item_lock::CItemLockTimer::IncreaseItemLockTimer(CUser *)
CItemLockTimer* CItemLockTimer::IncreaseItemLockTimer(CUser* user) {
    if (!timer_count_) {
        G_TimerQueue()->InsertTimer(0,
                                    user->GetUID(),
                                    136,
                                    60,
                                    0,
                                    user->getCurCharacNo());
    }
    ++timer_count_;
    return this;
}

// IDA_EA: 0x85414F0
// IDA_NAME: _ZN9item_lock14CItemLockTimer21DecreaseItemLockTimerEv
// DEMANGLED: item_lock::CItemLockTimer::DecreaseItemLockTimer(void)
CItemLockTimer* CItemLockTimer::DecreaseItemLockTimer() {
    --timer_count_;
    return this;
}

// IDA_EA: 0x8541502
// IDA_NAME: _ZN9item_lock14CItemLockTimer17NextItemLockTimerEP5CUser
// DEMANGLED: item_lock::CItemLockTimer::NextItemLockTimer(CUser *)
int CItemLockTimer::NextItemLockTimer(CUser* user) {
    if (timer_count_) {
        return G_TimerQueue()->InsertTimer(0,
                                           user->GetUID(),
                                           136,
                                           60,
                                           0,
                                           user->getCurCharacNo());
    }
    return timer_count_;
}

// IDA_EA: 0x854303A
// IDA_NAME: _ZN9item_lock14CItemLockTimer13GetTimerCountEv
// DEMANGLED: item_lock::CItemLockTimer::GetTimerCount(void)
int CItemLockTimer::GetTimerCount() {
    return timer_count_;
}

// IDA_EA: 0x8543044
// IDA_NAME: _ZN9item_lock14CItemLockTimerC2Ev
// DEMANGLED: item_lock::CItemLockTimer::CItemLockTimer(void)
CItemLockTimer::CItemLockTimer()
    : timer_count_(0) {}

// IDA_EA: 0x8543052
// IDA_NAME: _ZN9item_lock14CItemLockTimer5ResetEv
// DEMANGLED: item_lock::CItemLockTimer::Reset(void)
CItemLockTimer* CItemLockTimer::Reset() {
    timer_count_ = 0;
    return this;
}

// IDA_EA: 0x849AAA4
// IDA_NAME: _ZN9item_lock9CItemLock17GetItemLockRefVecEv
// DEMANGLED: item_lock::CItemLock::GetItemLockRefVec(void)
std::vector<stItemLockRef>& CItemLock::GetItemLockRefVec() {
    return item_lock_refs_;
}

// IDA_EA: 0x854156A
// IDA_NAME: _ZN9item_lock9CItemLockC2Ev
// DEMANGLED: item_lock::CItemLock::CItemLock(void)
CItemLock::CItemLock() {}

// IDA_EA: 0x8541644
// IDA_NAME: _ZN9item_lock9CItemLockD2Ev
// DEMANGLED: item_lock::CItemLock::~CItemLock()
// IDA_EA: 0x85416F6
// IDA_NAME: _ZN9item_lock9CItemLockD0Ev
// DEMANGLED: item_lock::CItemLock::~CItemLock()
CItemLock::~CItemLock() {}

// IDA_EA: 0x8541714
// IDA_NAME: _ZN9item_lock9CItemLock8loadDataEP5CUserPc
// DEMANGLED: item_lock::CItemLock::loadData(CUser *,char *)
bool CItemLock::loadData(CUser* user, char* data) {
    stItemLockInfo* lock_info = reinterpret_cast<stItemLockInfo*>(data);
    for (int index = 99; index >= 0; --index) {
        PushItemLock(user,
                     static_cast<unsigned char>(index + 1),
                     &lock_info[index]);
    }
    return 1;
}

// IDA_EA: 0x854176E
// IDA_NAME: _ZN9item_lock9CItemLock9_saveDataEP5CUser
// DEMANGLED: item_lock::CItemLock::_saveData(CUser *)
int CItemLock::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("ItemLock.cpp", 75);
    CStreamGuard guard(stream, true);
    *guard << 0x1B8;
    *guard << user->GetUID();

    SIG_ITEM_LOCK_UPDATE* payload =
        guard->GetInBuffer<SIG_ITEM_LOCK_UPDATE>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));

    for (std::map<unsigned char, stItemLockInfo>::const_iterator it =
             item_locks_.begin();
         it != item_locks_.end();
         ++it) {
        const int index = static_cast<int>(it->first) - 1;
        if (index >= 0 && index < 100) {
            payload->item_locks[index] = it->second;
        } else {
            LogManager::logFormat(
                1,
                "ItemLock.cpp",
                "virtual bool item_lock::CItemLock::_saveData(CUser*)",
                89,
                "itemlock error index %d",
                index);
        }
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x8541958
// IDA_NAME: _ZNK9item_lock9CItemLock7getDataEPc
// DEMANGLED: item_lock::CItemLock::getData(char *)const
void CItemLock::getData(char* data) const {
    stItemLockInfo* lock_info = reinterpret_cast<stItemLockInfo*>(data);
    for (std::map<unsigned char, stItemLockInfo>::const_iterator it =
             item_locks_.begin();
         it != item_locks_.end();
         ++it) {
        const int index = static_cast<int>(it->first) - 1;
        if (index >= 0 && index < 100) {
            lock_info[index] = it->second;
        } else {
            LogManager::logFormat(
                1,
                "ItemLock.cpp",
                "virtual void item_lock::CItemLock::getData(char*) const",
                108,
                "itemlock error index %d",
                index);
        }
    }
}

// IDA_EA: 0x8541A3E
// IDA_NAME: _ZN9item_lock9CItemLock6_resetEv
// DEMANGLED: item_lock::CItemLock::_reset(void)
int CItemLock::_reset() {
    item_locks_.clear();
    item_lock_refs_.clear();
    while (!free_locks_.empty()) {
        free_locks_.pop();
    }
    return static_cast<int>(reinterpret_cast<uintptr_t>(timer_.Reset()));
}

// IDA_EA: 0x8541A96
// IDA_NAME: _ZNK9item_lock9CItemLock13CheckItemLockEh
// DEMANGLED: item_lock::CItemLock::CheckItemLock(unsigned char)const
bool CItemLock::CheckItemLock(unsigned char lock) const {
    return item_locks_.find(lock) != item_locks_.end();
}

// IDA_EA: 0x8541AFE
// IDA_NAME: _ZNK9item_lock9CItemLock14IsAbleItemLockEh
// DEMANGLED: item_lock::CItemLock::IsAbleItemLock(unsigned char)const
int CItemLock::IsAbleItemLock(unsigned char lock) const {
    (void)lock;
    return free_locks_.empty() ? 22 : 0;
}

// IDA_EA: 0x8541B2A
// IDA_NAME: _ZNK9item_lock9CItemLock16IsAbleItemUnlockEhRi
// DEMANGLED: item_lock::CItemLock::IsAbleItemUnlock(unsigned char,int &)const
int CItemLock::IsAbleItemUnlock(unsigned char lock, int& unlock_time) const {
    (void)unlock_time;
    std::map<unsigned char, stItemLockInfo>::const_iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        return 19;
    }
    return it->second.lock_type == 1 ? 0 : 19;
}

// IDA_EA: 0x8541BB0
// IDA_NAME: _ZNK9item_lock9CItemLock22IsAbleItemUnlockCancelEh
// DEMANGLED: item_lock::CItemLock::IsAbleItemUnlockCancel(unsigned char)const
int CItemLock::IsAbleItemUnlockCancel(unsigned char lock) const {
    std::map<unsigned char, stItemLockInfo>::const_iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        return 19;
    }
    return it->second.lock_type == 2 ? 0 : 19;
}

// IDA_EA: 0x8541C36
// IDA_NAME: _ZN9item_lock9CItemLock18UpdateCreatereItemEP5CUserP10Inven_Item
// DEMANGLED: item_lock::CItemLock::UpdateCreatereItem(CUser *,Inven_Item *)
void CItemLock::UpdateCreatereItem(CUser* user, Inven_Item* item) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("ItemLock.cpp", 181);
    CStreamGuard guard(stream, true);
    const int item_key = item->add_info_;
    const stAmplifyOption_t amplify = item->amplify_option_;
    *guard << 0x1C1;
    *guard << user->GetUID();
    *guard << item_key;
    *guard << amplify.GetLock();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8541D42
// IDA_NAME: _ZN9item_lock9CItemLock16UpdateAvatarItemEP5CUserP10Inven_Item
// DEMANGLED: item_lock::CItemLock::UpdateAvatarItem(CUser *,Inven_Item *)
void CItemLock::UpdateAvatarItem(CUser* user, Inven_Item* item) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("ItemLock.cpp", 191);
    CStreamGuard guard(stream, true);
    const int item_key = item->add_info_;
    const stAmplifyOption_t amplify = item->amplify_option_;
    *guard << 0x1C2;
    *guard << user->GetUID();
    *guard << item_key;
    *guard << amplify.GetLock();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8541E4E
// IDA_NAME: _ZN9item_lock9CItemLock10UpdateItemEP5CUserP10Inven_Item
// DEMANGLED: item_lock::CItemLock::UpdateItem(CUser *,Inven_Item *)
void CItemLock::UpdateItem(CUser* user, Inven_Item* item) {
    if (item->item_category_ == 8) {
        UpdateAvatarItem(user, item);
    } else if (item->item_category_ == 5) {
        UpdateCreatereItem(user, item);
    }
}

// IDA_EA: 0x8541EA0
// IDA_NAME: _ZN9item_lock9CItemLock19SendRequestItemLockEP5CUserii
// DEMANGLED: item_lock::CItemLock::SendRequestItemLock(CUser *,int,int)
void CItemLock::SendRequestItemLock(CUser* user,
                                    int item_space,
                                    int slot) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_ITEM_LOCK);
    packet.put_byte(1);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8541F5A
// IDA_NAME: _ZN9item_lock9CItemLock21SendRequestItemUnlockEP5CUseriii
// DEMANGLED: item_lock::CItemLock::SendRequestItemUnlock(CUser *,int,int,int)
void CItemLock::SendRequestItemUnlock(CUser* user,
                                      int item_space,
                                      int slot,
                                      int remain_time) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK);
    packet.put_byte(1);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.put_int(remain_time);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8542026
// IDA_NAME: _ZN9item_lock9CItemLock27SendRequestItemUnlockCancelEP5CUserii
// DEMANGLED: item_lock::CItemLock::SendRequestItemUnlockCancel(CUser *,int,int)
void CItemLock::SendRequestItemUnlockCancel(CUser* user,
                                            int item_space,
                                            int slot) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK_CANCEL);
    packet.put_byte(1);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85420E0
// IDA_NAME: _ZN9item_lock9CItemLock14SendItemUnlockEP5CUserii
// DEMANGLED: item_lock::CItemLock::SendItemUnlock(CUser *,int,int)
void CItemLock::SendItemUnlock(CUser* user, int item_space, int slot) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 252);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8542192
// IDA_NAME: _ZN9item_lock9CItemLock10DoItemLockEP5CUserii
// DEMANGLED: item_lock::CItemLock::DoItemLock(CUser *,int,int)
int CItemLock::DoItemLock(CUser* user, int item_space, int slot) {
    Inven_Item* item = user->GetCurCharacInventoryRef(
        static_cast<unsigned short>(item_space),
        static_cast<unsigned short>(slot));
    if (!item) {
        return 0;
    }

    stAmplifyOption_t amplify = item->amplify_option_;
    unsigned char lock = static_cast<unsigned char>(amplify.GetLock());
    if (lock) {
        return 0;
    }

    lock = free_locks_.top();
    free_locks_.pop();

    stItemLockInfo lock_info;
    lock_info.lock_type = 1;
    lock_info.lock_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    item_locks_.insert(std::make_pair(lock, lock_info));
    alter();

    amplify.SetLock(lock);
    item->amplify_option_ = amplify;
    UpdateItem(user, item);
    SendRequestItemLock(user, item_space, slot);
    const int item_id = item->item_id_;
    const int item_key = item->add_info_;
    CUser_history_log(user)->ItemLockUsingLog(
        0, item_space, slot, item_id, item_key);
    return 1;
}

// IDA_EA: 0x854231A
// IDA_NAME: _ZN9item_lock9CItemLock12DoItemUnlockEP5CUserii
// DEMANGLED: item_lock::CItemLock::DoItemUnlock(CUser *,int,int)
int CItemLock::DoItemUnlock(CUser* user, int item_space, int slot) {
    Inven_Item* item = user->GetCurCharacInventoryRef(
        static_cast<unsigned short>(item_space),
        static_cast<unsigned short>(slot));
    if (!item) {
        return 0;
    }

    const stAmplifyOption_t amplify = item->amplify_option_;
    const unsigned char lock =
        static_cast<unsigned char>(amplify.GetLock());
    if (!lock) {
        return 0;
    }

    std::map<unsigned char, stItemLockInfo>::iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        return 0;
    }

    it->second.lock_type = 2;
    it->second.lock_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    alter();
    timer_.IncreaseItemLockTimer(user);

    SendRequestItemUnlock(user,
                          item_space,
                          slot,
                          std::max(
                              0,
                              static_cast<int>(it->second.lock_time) +
                                  kItemUnlockWaitSeconds -
                                  GlobalData::s_systemTime_.getCurSec()));
    const int item_id = item->item_id_;
    const int item_key = item->add_info_;
    CUser_history_log(user)->ItemLockUsingLog(
        1, item_space, slot, item_id, item_key);
    return 1;
}

// IDA_EA: 0x85424C2
// IDA_NAME: _ZN9item_lock9CItemLock18DoItemUnlockCancelEP5CUserii
// DEMANGLED: item_lock::CItemLock::DoItemUnlockCancel(CUser *,int,int)
int CItemLock::DoItemUnlockCancel(CUser* user, int item_space, int slot) {
    Inven_Item* item = user->GetCurCharacInventoryRef(
        static_cast<unsigned short>(item_space),
        static_cast<unsigned short>(slot));
    if (!item) {
        return 0;
    }

    const stAmplifyOption_t amplify = item->amplify_option_;
    const unsigned char lock =
        static_cast<unsigned char>(amplify.GetLock());
    if (!lock) {
        return 0;
    }

    std::map<unsigned char, stItemLockInfo>::iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        return 0;
    }

    it->second.lock_type = 1;
    it->second.lock_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    alter();
    timer_.DecreaseItemLockTimer();

    SendRequestItemUnlockCancel(user, item_space, slot);
    const int item_id = item->item_id_;
    const int item_key = item->add_info_;
    CUser_history_log(user)->ItemLockUsingLog(
        2, item_space, slot, item_id, item_key);
    return 1;
}

// IDA_EA: 0x854261E
// IDA_NAME: _ZNK9item_lock9CItemLock15GetItemLockInfoEhRi
// DEMANGLED: item_lock::CItemLock::GetItemLockInfo(unsigned char,int &)const
int CItemLock::GetItemLockInfo(unsigned char lock, int& lock_time) const {
    std::map<unsigned char, stItemLockInfo>::const_iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        return 0;
    }
    lock_time = static_cast<int>(it->second.lock_time);
    return it->second.lock_type;
}

// IDA_EA: 0x85426A6
// IDA_NAME: _ZNK9item_lock9CItemLock18MakeItemLockPacketER11PacketGuardh
// DEMANGLED: item_lock::CItemLock::MakeItemLockPacket(PacketGuard &,unsigned char)const
void CItemLock::MakeItemLockPacket(PacketGuard& packet,
                                   unsigned char lock) const {
    std::map<unsigned char, stItemLockInfo>::const_iterator it =
        item_locks_.find(lock);
    if (it == item_locks_.end()) {
        packet.put_byte(0);
        return;
    }

    packet.put_byte(it->second.lock_type);
    if (it->second.lock_type == 2) {
        packet.put_int(std::max(
            0,
            static_cast<int>(it->second.lock_time) +
                kItemUnlockWaitSeconds -
                GlobalData::s_systemTime_.getCurSec()));
    }
}

// IDA_EA: 0x85427A0
// IDA_NAME: _ZN9item_lock9CItemLock16SendItemLockListEP5CUser
// DEMANGLED: item_lock::CItemLock::SendItemLockList(CUser *)
void CItemLock::SendItemLockList(CUser* user) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 251);
    int count_index = packet.get_index();
    int count = 0;
    packet.put_short(0);

    for (std::vector<stItemLockRef>::const_iterator it =
             item_lock_refs_.begin();
         it != item_lock_refs_.end();
         ++it) {
        int lock_time = 0;
        const int lock_type = GetItemLockInfo(it->lock, lock_time);
        if (lock_type) {
            unsigned char item_space = it->item_space;
            // Raw 0x85428EC..0x8542906 is an OR, not Hex-Rays' likely-range
            // simplification; preserve the old always-true unsigned test.
            if (user->getCurChannelEquipslotSwitch() == 1 &&
                (it->slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst ||
                 it->slot <= oldsrv::config::kOrdinaryEquipmentTypeLast)) {
                if (it->item_space == 3) {
                    item_space = 18;
                } else if (it->item_space == 18) {
                    item_space = 3;
                }
            }
            packet.put_byte(item_space);
            packet.put_short(it->slot);
            packet.put_byte(lock_type);
            if (lock_type == 2) {
                packet.put_int(std::max(
                    0,
                    lock_time + kItemUnlockWaitSeconds -
                        GlobalData::s_systemTime_.getCurSec()));
            }
            ++count;
        } else {
            Inven_Item* item = user->GetCurCharacInventoryRef(
                static_cast<unsigned short>(it->item_space), it->slot);
            if (item) {
                stAmplifyOption_t amplify = item->amplify_option_;
                amplify.SetLock(0);
                item->amplify_option_ = amplify;
                UpdateItem(user, item);
            }
        }
    }

    packet.put_byte(count_index, count);
    packet.finalize(true);
    user->Send(packet);
    item_lock_refs_.clear();
}

// IDA_EA: 0x8542A6A
// IDA_NAME: _ZN9item_lock9CItemLock23OnItemUnlockWaitTimeoutEP5CUser
// DEMANGLED: item_lock::CItemLock::OnItemUnlockWaitTimeout(CUser *)
int CItemLock::OnItemUnlockWaitTimeout(CUser* user) {
    PopItemLock(user);
    return 1;
}

// IDA_EA: 0x8542A8A
// IDA_NAME: _ZN9item_lock9CItemLock12PushItemLockEP5CUserhP14stItemLockInfo
// DEMANGLED: item_lock::CItemLock::PushItemLock(CUser *,unsigned char,stItemLockInfo *)
void CItemLock::PushItemLock(CUser* user,
                             unsigned char lock,
                             stItemLockInfo* lock_info) {
    if (!lock_info->lock_type) {
        free_locks_.push(lock);
        return;
    }

    if (lock_info->lock_type == 2) {
        if (static_cast<int>(lock_info->lock_time) <=
            GlobalData::s_systemTime_.getCurSec() -
                kItemUnlockWaitSeconds) {
            lock_info->lock_type = 0;
            lock_info->lock_time = 0;
            free_locks_.push(lock);
            alter();
            return;
        }
        timer_.IncreaseItemLockTimer(user);
    }

    item_locks_.insert(std::make_pair(lock, *lock_info));
}

// IDA_EA: 0x8542B80
// IDA_NAME: _ZN9item_lock9CItemLock11PopItemLockEP5CUser
// DEMANGLED: item_lock::CItemLock::PopItemLock(CUser *)
void CItemLock::PopItemLock(CUser* user) {
    if (!timer_.GetTimerCount()) {
        return;
    }

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    std::map<unsigned char, stItemLockInfo>::iterator it =
        item_locks_.begin();
    while (it != item_locks_.end()) {
        if (static_cast<int>(it->second.lock_time) +
                    kItemUnlockWaitSeconds <= current_second &&
            it->second.lock_type == 2) {
            ENUM_ITEMSPACE item_space;
            int slot;
            if (!user->FindItemLock(it->first,
                                    item_space,
                                    slot,
                                    user)) {
                return;
            }

            Inven_Item* item = user->GetCurCharacInventoryRef(
                static_cast<unsigned short>(item_space),
                static_cast<unsigned short>(slot));
            if (item) {
                stAmplifyOption_t amplify = item->amplify_option_;
                amplify.SetLock(0);
                item->amplify_option_ = amplify;
                UpdateItem(user, item);
            }

            int packet_item_space = static_cast<int>(item_space);
            if (user->getCurChannelEquipslotSwitch() == 1 &&
                slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
                slot <= oldsrv::config::kOrdinaryEquipmentTypeLast) {
                if (packet_item_space == 3) {
                    packet_item_space = 18;
                } else if (packet_item_space == 18) {
                    packet_item_space = 3;
                }
            }
            SendItemUnlock(user, packet_item_space, slot);

            free_locks_.push(it->first);
            std::map<unsigned char, stItemLockInfo>::iterator erase_it = it++;
            item_locks_.erase(erase_it);
            timer_.DecreaseItemLockTimer();
            alter();
            const int item_id = item->item_id_;
            const int item_key = item->add_info_;
            CUser_history_log(user)->ItemLockUsingLog(
                3, packet_item_space, slot, item_id, item_key);
        } else {
            ++it;
        }
    }

    timer_.NextItemLockTimer(user);
}

// IDA_EA: 0x8542E12
// IDA_NAME: _ZN9item_lock9CItemLock9_findItemEh
// DEMANGLED: item_lock::CItemLock::_findItem(unsigned char)
bool CItemLock::_findItem(unsigned char lock) {
    for (std::vector<stItemLockRef>::iterator it =
             item_lock_refs_.begin();
         it != item_lock_refs_.end();
         ++it) {
        if (it->lock == lock) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x8542E98
// IDA_NAME: _ZN9item_lock9CItemLock14VerifyItemLockEP5CUser
// DEMANGLED: item_lock::CItemLock::VerifyItemLock(CUser *)
void CItemLock::VerifyItemLock(CUser* user) {
    if (!user) {
        return;
    }

    bool altered = false;
    std::map<unsigned char, stItemLockInfo>::iterator it =
        item_locks_.begin();
    while (it != item_locks_.end()) {
        const int lock_type = it->second.lock_type;
        if ((lock_type == 1 || lock_type == 2) &&
            !_findItem(it->first)) {
            free_locks_.push(it->first);
            std::map<unsigned char, stItemLockInfo>::iterator erase_it = it++;
            item_locks_.erase(erase_it);
            if (lock_type == 2) {
                timer_.DecreaseItemLockTimer();
            }
            altered = true;
        } else {
            ++it;
        }
    }

    if (altered) {
        alter();
    }
}

}  // namespace item_lock
