#ifndef OLDSRV_OLD_DATA_PVF_EVENT_UPGRADE_ROOM_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_UPGRADE_ROOM_H_

#include <stdint.h>

#include <map>

namespace upgrade_room {

// Native projection of event/upgraderoom/upgraderoom.info.  The file owns
// donation limits and spectator capacity only; it contains no probability or
// penalty policy.  Reinforcement continues to use the ordinary upgrade PVF.
struct UpgradeRoomConfig {
    UpgradeRoomConfig();

    std::map<uint32_t, uint32_t> donation_limits;
    uint16_t max_spectators;
    uint32_t default_gold;
};

// Loads once through the canonical RDAR/PVF owner.  Missing, duplicate,
// unknown or out-of-range fields fail closed and return null.
const UpgradeRoomConfig* GetUpgradeRoomConfig();
bool IsUpgradeRoomConfigLoaded();

}  // namespace upgrade_room_0828

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_UPGRADE_ROOM_0828_H_
