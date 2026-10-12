#ifndef DNF_DATAMANAGER_REQUIRED_ITEM_H_
#define DNF_DATAMANAGER_REQUIRED_ITEM_H_

#include <stddef.h>
#include <stdint.h>

// IDA unit: 09_dungeon_pvp_party/RequiredItem.
// Map and party exchange this record directly.  Its original C2 and clear()
// both write all three material fields, yielding a 0x0c i386 object.
// IDA_EA: 0x836A9A8
// IDA_NAME: _ZN12RequiredItemC2Ev
// DEMANGLED: RequiredItem::RequiredItem(void)
struct RequiredItem {
    RequiredItem();
    RequiredItem* clear();

    int32_t item_id;             // +0x00
    int32_t item_count;          // +0x04
    uint8_t delete_after_start;  // +0x08
};

static_assert(offsetof(RequiredItem, item_id) == 0x00,
              "RequiredItem::item_id ABI offset");
static_assert(offsetof(RequiredItem, item_count) == 0x04,
              "RequiredItem::item_count ABI offset");
static_assert(offsetof(RequiredItem, delete_after_start) == 0x08,
              "RequiredItem::delete_after_start ABI offset");
static_assert(sizeof(RequiredItem) == 0x0c,
              "RequiredItem i386 ABI size");

#endif  // DNF_DATAMANAGER_REQUIRED_ITEM_H_
