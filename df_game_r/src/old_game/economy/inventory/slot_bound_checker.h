#ifndef OLDSRV_OLD_GAME_INVENTORY_SLOT_BOUND_CHECKER_H_
#define OLDSRV_OLD_GAME_INVENTORY_SLOT_BOUND_CHECKER_H_

#include <stddef.h>

#include <map>

#include "old_game/economy/inventory/inventory.h"


struct SlotBoundKey {
    SlotBoundKey(unsigned short item_id_value,
                 Inven_Item::ITEM_TYPE item_type_value);

    bool operator<(const SlotBoundKey& rhs) const;

    unsigned short item_id;
    Inven_Item::ITEM_TYPE item_type;
};

struct SlotBound {
    SlotBound(int first_value, int second_value, bool flag_value);

    int first;
    int second;
    bool flag;
};

struct CSlotBoundChecker {
    CSlotBoundChecker();
    ~CSlotBoundChecker();

    int get_slot_bound(Inven_Item::ITEM_TYPE item_type,
                       int item_id,
                       int& first,
                       int& second) const;
    bool insert(int first,
                int second,
                Inven_Item::ITEM_TYPE item_type,
                bool shifted_second,
                bool flag);
    bool init();
    int get_item_type(int slot,
                      int item_id,
                      Inven_Item::ITEM_TYPE& item_type) const;

    std::map<SlotBoundKey, SlotBound> slot_bounds;
};

extern CSlotBoundChecker g_slotBoundChecker;

static_assert(sizeof(SlotBoundKey) == 0x08u,
              "SlotBoundKey IDA ABI size must stay 0x08");
static_assert(offsetof(SlotBoundKey, item_id) == 0x00u,
              "SlotBoundKey::item_id must stay at +0x00");
static_assert(offsetof(SlotBoundKey, item_type) == 0x04u,
              "SlotBoundKey::item_type must stay at +0x04");
static_assert(sizeof(SlotBound) == 0x0Cu,
              "SlotBound IDA ABI size must stay 0x0c");
static_assert(offsetof(SlotBound, first) == 0x00u,
              "SlotBound::first must stay at +0x00");
static_assert(offsetof(SlotBound, second) == 0x04u,
              "SlotBound::second must stay at +0x04");
static_assert(offsetof(SlotBound, flag) == 0x08u,
              "SlotBound::flag must stay at +0x08");
static_assert(sizeof(CSlotBoundChecker) == 0x18u,
              "CSlotBoundChecker IDA ABI size must stay 0x18");

#endif  // OLDSRV_OLD_GAME_INVENTORY_SLOT_BOUND_CHECKER_H_
