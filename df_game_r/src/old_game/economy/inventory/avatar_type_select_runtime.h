#ifndef OLDSRV_OLD_GAME_INVENTORY_AVATAR_TYPE_SELECT_RUNTIME_H_
#define OLDSRV_OLD_GAME_INVENTORY_AVATAR_TYPE_SELECT_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

// IDA unit: 06_userinfo_stat_inventory/stAvatarTypeSelect_t.
//
// Keep this layout in a dependency-free leaf header.  Equipment PVF data
// embeds vectors of this record, while the wider avatar runtime also depends
// on inventory/event declarations; importing that wider header from
// equipment_script.h creates a global/item.h -> equipment -> inventory cycle.
struct __attribute__((packed)) stAvatarTypeSelect_t {
    ~stAvatarTypeSelect_t();

    bool init();

    // IDA 0x898D1FC clears four dwords, the socket count and five
    // unsigned-short socket types.
    int32_t usable_period;        // +0x00
    int32_t gold_price;           // +0x04
    int32_t win_point_price;      // +0x08
    int32_t cera_price;           // +0x0c
    int32_t socket_count;         // +0x10
    uint16_t socket_types[5];     // +0x14
};

static_assert(offsetof(stAvatarTypeSelect_t, usable_period) == 0x00u,
              "avatar selection usable period must stay at +0x00");
static_assert(offsetof(stAvatarTypeSelect_t, gold_price) == 0x04u,
              "avatar selection gold price must stay at +0x04");
static_assert(offsetof(stAvatarTypeSelect_t, win_point_price) == 0x08u,
              "avatar selection win-point price must stay at +0x08");
static_assert(offsetof(stAvatarTypeSelect_t, cera_price) == 0x0cu,
              "avatar selection Cera price must stay at +0x0c");
static_assert(offsetof(stAvatarTypeSelect_t, socket_count) == 0x10u,
              "avatar selection socket count must stay at +0x10");
static_assert(offsetof(stAvatarTypeSelect_t, socket_types) == 0x14u,
              "avatar selection socket types must stay at +0x14");
static_assert(sizeof(stAvatarTypeSelect_t) == 0x1eu,
              "stAvatarTypeSelect_t IDA ABI size must stay 0x1E");

#endif  // OLDSRV_OLD_GAME_INVENTORY_AVATAR_TYPE_SELECT_RUNTIME_H_
