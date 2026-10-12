#ifndef OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_RUNTIME_H_
#define OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_RUNTIME_H_

#include "old_game/player/core/charac_expand_cdata.h"

#include <stdint.h>

#include <map>
#include <stack>
#include <vector>


class CUser;
class Inven_Item;
class PacketGuard;
struct stItemLockInfo;

namespace item_lock {

// IDA unit: 99_misc_global/item_lock/stItemLockRef.  Inter_LoadInven and
// Inter_LoadEtc append these exact four-byte references before CItemLock
// verifies the persistent lock map against the live inventory.
struct stItemLockRef {
    uint8_t lock;
    uint8_t item_space;
    uint16_t slot;

    ~stItemLockRef();
};

// IDA unit: 99_misc_global/item_lock/CItemLockTimer.  The counter tracks the
// number of lock records waiting for the old three-day unlock timeout.  Only
// the 0 -> 1 transition installs timer message 136.
class CItemLockTimer {
public:
    CItemLockTimer();

    CItemLockTimer* IncreaseItemLockTimer(CUser* user);
    CItemLockTimer* DecreaseItemLockTimer();
    int NextItemLockTimer(CUser* user);
    int GetTimerCount();
    CItemLockTimer* Reset();

private:
    int timer_count_;
};

// IDA unit: 99_misc_global/item_lock/CItemLock.  CDataMgr slot 2 owns this
// complete 0x58-byte persistent item-lock state machine.
class CItemLock : public charac_expand::CData {
public:
    CItemLock();
    virtual ~CItemLock();

    std::vector<stItemLockRef>& GetItemLockRefVec();

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;

    bool CheckItemLock(unsigned char lock) const;
    int IsAbleItemLock(unsigned char lock) const;
    int IsAbleItemUnlock(unsigned char lock, int& unlock_time) const;
    int IsAbleItemUnlockCancel(unsigned char lock) const;

    void UpdateCreatereItem(CUser* user, Inven_Item* item);
    void UpdateAvatarItem(CUser* user, Inven_Item* item);
    void UpdateItem(CUser* user, Inven_Item* item);

    void SendRequestItemLock(CUser* user, int item_space, int slot);
    void SendRequestItemUnlock(CUser* user,
                               int item_space,
                               int slot,
                               int remain_time);
    void SendRequestItemUnlockCancel(CUser* user,
                                     int item_space,
                                     int slot);
    void SendItemUnlock(CUser* user, int item_space, int slot);

    int DoItemLock(CUser* user, int item_space, int slot);
    int DoItemUnlock(CUser* user, int item_space, int slot);
    int DoItemUnlockCancel(CUser* user, int item_space, int slot);

    int GetItemLockInfo(unsigned char lock, int& lock_time) const;
    void MakeItemLockPacket(PacketGuard& packet, unsigned char lock) const;
    void SendItemLockList(CUser* user);
    int OnItemUnlockWaitTimeout(CUser* user);

    void PushItemLock(CUser* user,
                      unsigned char lock,
                      stItemLockInfo* lock_info);
    void PopItemLock(CUser* user);
    void VerifyItemLock(CUser* user);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    bool _findItem(unsigned char lock);

    std::map<unsigned char, stItemLockInfo> item_locks_;  // IDA +0x08
    std::stack<unsigned char> free_locks_;                // IDA +0x20
    std::vector<stItemLockRef> item_lock_refs_;           // IDA +0x48
    CItemLockTimer timer_;                                // IDA +0x54
};

static_assert(sizeof(stItemLockRef) == 0x04u,
              "stItemLockRef IDA ABI size must stay 0x04");
static_assert(sizeof(CItemLockTimer) == 0x04u,
              "CItemLockTimer IDA ABI size must stay 0x04");
static_assert(sizeof(CItemLock) == 0x58u,
              "CItemLock IDA ABI size must stay 0x58");

}  // namespace item_lock

#endif  // OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_RUNTIME_H_
