#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_UPGRADE_ROOM_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_UPGRADE_ROOM_H_

#include <stdint.h>

#include <vector>

class CUser;
struct Inven_Item;

namespace upgrade_room {

struct Donation {
    Donation() : item_id(0u), amount(0u) {}
    Donation(uint32_t item, uint32_t value)
        : item_id(item), amount(value) {}

    uint32_t item_id;
    uint32_t amount;
};

// Authoritative game-thread operations for CMD570-574 and CMD582-585.
// Return zero on a committed operation, a positive one-byte client error when
// the target handler's error envelope is closed, and -1 for an internal
// fail-closed path that must never be converted into a success response.
int CreateRoom(CUser* user, uint16_t client_x, uint16_t client_y);
int DestroyRoom(CUser* user);
int JoinRoom(CUser* user, uint16_t room_id, uint16_t owner_uid);
int LeaveRoom(CUser* user);
int Donate(CUser* user, const std::vector<Donation>& donations);
int PutUpItem(CUser* user, uint32_t item_id, uint16_t inventory_slot);
int StartUpgrade(CUser* user);
int CancelUpgrade(CUser* user);
int SetMasterMessage(CUser* user,
                     const std::vector<uint16_t>& utf16_message);

// Shared lifecycle integration.  All three functions are idempotent.  The
// area-change hook destroys an owner's room or removes a spectator once the
// authoritative server village/area no longer matches the room scope.
void OnUserDisconnect(CUser* user);
void OnUserAreaChanged(CUser* user);
void OnEnterWorld(CUser* user);

// Narrow adapter used by the ordinary CItemUpgrade owner while CMD583 is in
// flight.  Shared item-upgrade code must call these only at the four documented
// material/money resolution and consumption sites; ordinary reinforcement is
// unchanged when no scoped context is active.
class ScopedOwnerUpgradeContext {
public:
    explicit ScopedOwnerUpgradeContext(CUser* user);
    ~ScopedOwnerUpgradeContext();

    bool ready() const { return ready_; }

private:
    CUser* user_;
    void* room_;
    bool ready_;

    ScopedOwnerUpgradeContext(const ScopedOwnerUpgradeContext&);
    ScopedOwnerUpgradeContext& operator=(
        const ScopedOwnerUpgradeContext&);
};

bool IsOwnerUpgradeContextActive(const CUser* user);
bool ResolveUpgradeMaterial(CUser* user,
                            uint16_t requested_slot,
                            Inven_Item& material);
int EffectiveMoney(const CUser* user, int owner_money);
int ConsumeUpgradeMoney(CUser* user, int total, int money_reason);
int ConsumeUpgradeMaterial(CUser* user,
                           uint16_t requested_slot,
                           int item_id,
                           int total,
                           int item_reason);

}  // namespace upgrade_room_0828

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_UPGRADE_ROOM_0828_H_
