#ifndef OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_SCRIPT_H_

#include <stdint.h>

namespace collect_box_script {

enum { kMaximumBeadCount = 15 };

// Loads both localization/collectbox/collectboxinfo.etc and the referenced
// collectboxbuffinfo.etc types.  Publication is atomic and fail-closed.
bool LoadConfig();
bool IsReady();

uint8_t BoxId();
uint32_t BaseExpiry();
uint32_t MaximumExpiry();
bool IsRawExpiry(uint32_t expiry_at);

bool IsBead(uint32_t item_id);
bool IsExtension(uint32_t item_id,
                 uint32_t* extension_days,
                 bool* unlimited);

uint32_t RemainingSeconds(uint32_t expiry_at, uint32_t now);

}  // namespace collect_box_script

#endif  // OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_SCRIPT_H_
