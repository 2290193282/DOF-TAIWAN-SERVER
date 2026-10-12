#ifndef OLDSRV_OLD_DATA_DB_ACCOUNT_CHARAC_VIEW_COMPAT_H_
#define OLDSRV_OLD_DATA_DB_ACCOUNT_CHARAC_VIEW_COMPAT_H_

#include "common/config.h"
#include "old_data/db/account/login_payloads.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace oldsrv {
namespace db {
namespace charac_view_compat {

enum {
    kCharacterViewRecordCount = 36,
    kLegacyVisibleItemCount = 12,
    kInsertedWeaponAvatarSlot = 10,
};

#pragma pack(push, 1)

// Pre-0828 character-view persistence record.  The 0828 format inserts the
// weapon-avatar appearance at equipment index 10, shifting the physical
// weapon and title entries (and therefore the three trailing timestamps) by
// eight bytes.
struct LegacyCharacViewEntry {
    struct VisibleItem {
        uint32_t item_id;
        uint16_t option;
        uint16_t color;
    };

    uint32_t charac_no;                         // +0x00
    SIG_LOGIN_DATA::CharacViewEntry::IdentityFields identity; // +0x04
    uint8_t request_rename_state;               // +0x22
    uint8_t visible_header[5];                  // +0x23
    VisibleItem visible_items[kLegacyVisibleItemCount]; // +0x28
    uint32_t create_time;                       // +0x88
    uint32_t rename_expire_time;                // +0x8C
    uint32_t visible_state;                     // +0x90
};

#pragma pack(pop)

static_assert(oldsrv::config::kEquipmentTypeWeaponAvatar == 10,
              "0828 weapon-avatar slot must stay at index 10");
static_assert(oldsrv::config::kEquipmentTypeWeapon == 11,
              "0828 physical-weapon slot must stay at index 11");
static_assert(oldsrv::config::kEquipmentTypeTitle == 12,
              "0828 title slot must stay at index 12");
static_assert(sizeof(LegacyCharacViewEntry::VisibleItem) == 0x08u,
              "legacy visible-item record must stay eight bytes");
static_assert(sizeof(LegacyCharacViewEntry) == 0x94u,
              "legacy character-view record must stay 0x94 bytes");
static_assert(offsetof(LegacyCharacViewEntry, visible_items) == 0x28u &&
                  offsetof(LegacyCharacViewEntry, create_time) == 0x88u &&
                  offsetof(LegacyCharacViewEntry, rename_expire_time) ==
                      0x8Cu &&
                  offsetof(LegacyCharacViewEntry, visible_state) == 0x90u,
              "legacy character-view field offsets drift");
static_assert(sizeof(SIG_LOGIN_DATA::CharacViewEntry) == 0x9Cu,
              "0828 character-view record must stay 0x9C bytes");
static_assert(
    offsetof(SIG_LOGIN_DATA::CharacViewEntry, visible_items) == 0x28u &&
        offsetof(SIG_LOGIN_DATA::CharacViewEntry, create_time) == 0x90u &&
        offsetof(SIG_LOGIN_DATA::CharacViewEntry, rename_expire_time) ==
            0x94u &&
        offsetof(SIG_LOGIN_DATA::CharacViewEntry, visible_state) == 0x98u,
    "0828 character-view field offsets drift");

inline bool UpgradeLegacyCharacViewBlob(
    const char* legacy_blob,
    unsigned int legacy_blob_size,
    SIG_LOGIN_DATA::CharacViewEntry* current_entries,
    unsigned int current_entry_count) {
    if (!legacy_blob || !current_entries ||
        current_entry_count != kCharacterViewRecordCount ||
        legacy_blob_size !=
            sizeof(LegacyCharacViewEntry) * kCharacterViewRecordCount) {
        return false;
    }

    const LegacyCharacViewEntry* const legacy_entries =
        reinterpret_cast<const LegacyCharacViewEntry*>(legacy_blob);
    memset(current_entries,
           0,
           sizeof(SIG_LOGIN_DATA::CharacViewEntry) * current_entry_count);

    for (unsigned int record = 0; record < current_entry_count; ++record) {
        const LegacyCharacViewEntry& legacy = legacy_entries[record];
        SIG_LOGIN_DATA::CharacViewEntry& current = current_entries[record];

        // The prefix through +0x27 is byte-identical in both formats.
        memcpy(&current,
               &legacy,
               offsetof(LegacyCharacViewEntry, visible_items));

        // Slots 0..9 retain their indices.  Slot 10 is the newly inserted
        // weapon-avatar appearance and remains zero from the memset above.
        for (unsigned int slot = 0; slot < kInsertedWeaponAvatarSlot; ++slot) {
            current.visible_items[slot].item_id =
                legacy.visible_items[slot].item_id;
            current.visible_items[slot].option =
                legacy.visible_items[slot].option;
            current.visible_items[slot].color =
                legacy.visible_items[slot].color;
        }

        // Old physical weapon/title slots 10/11 become native 0828 slots
        // 11/12.  This must not be replaced by a flat 12-slot memcpy.
        for (unsigned int old_slot = kInsertedWeaponAvatarSlot;
             old_slot < kLegacyVisibleItemCount;
             ++old_slot) {
            const unsigned int new_slot = old_slot + 1;
            current.visible_items[new_slot].item_id =
                legacy.visible_items[old_slot].item_id;
            current.visible_items[new_slot].option =
                legacy.visible_items[old_slot].option;
            current.visible_items[new_slot].color =
                legacy.visible_items[old_slot].color;
        }

        current.create_time = legacy.create_time;
        current.rename_expire_time = legacy.rename_expire_time;
        current.visible_state = legacy.visible_state;
    }
    return true;
}

}  // namespace charac_view_compat
}  // namespace db
}  // namespace oldsrv

#endif  // OLDSRV_OLD_DATA_DB_ACCOUNT_CHARAC_VIEW_COMPAT_H_
