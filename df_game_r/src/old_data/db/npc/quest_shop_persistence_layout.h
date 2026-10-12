#ifndef OLDSRV_OLD_DATA_DB_NPC_QUEST_SHOP_PERSISTENCE_LAYOUT_H_
#define OLDSRV_OLD_DATA_DB_NPC_QUEST_SHOP_PERSISTENCE_LAYOUT_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "old_game/player/core/character_tag_data.h"

namespace quest_shop_persistence {

enum {
    kBuyingStatusColumnCount = 26,
};

// charac_quest_shop columns 2..27 are not stored in appendage-status order.
// The official DB_LoadEtc::_LoadQuestShop body writes them to these absolute
// CHARAC_QUEST_SHOP_DATA byte offsets.  Keep the one table shared by both the
// DB reader and writer so a database column can never drift to another
// appendage status again.
static const uint16_t kBuyingStatusPayloadOffsets[
    kBuyingStatusColumnCount] = {
        38, 108, 12, 18, 14, 16, 34, 32, 40, 70, 60, 52, 54,
        58, 56, 104, 96, 98, 102, 100, 42, 44, 78, 66, 72, 118};

inline size_t BuyingStatusIndexForColumn(size_t column) {
    return (kBuyingStatusPayloadOffsets[column] -
            offsetof(CHARAC_QUEST_SHOP_DATA, buying_count)) /
           sizeof(uint16_t);
}

inline uint16_t ReadBuyingStatusColumn(const unsigned char* payload,
                                       size_t column) {
    uint16_t value = 0;
    memcpy(&value,
           payload + kBuyingStatusPayloadOffsets[column],
           sizeof(value));
    return value;
}

inline void WriteBuyingStatusColumn(CHARAC_QUEST_SHOP_DATA* quest_shop,
                                    size_t column,
                                    uint16_t value) {
    quest_shop->buying_count[BuyingStatusIndexForColumn(column)] = value;
}

static_assert(sizeof(kBuyingStatusPayloadOffsets) /
                      sizeof(kBuyingStatusPayloadOffsets[0]) ==
                  kBuyingStatusColumnCount,
              "quest-shop DB column layout must retain all 26 statuses");
static_assert(offsetof(CHARAC_QUEST_SHOP_DATA, buying_count) == 12u,
              "quest-shop status mapping requires buying_count at +0x0c");

}  // namespace quest_shop_persistence

#endif  // OLDSRV_OLD_DATA_DB_NPC_QUEST_SHOP_PERSISTENCE_LAYOUT_H_
