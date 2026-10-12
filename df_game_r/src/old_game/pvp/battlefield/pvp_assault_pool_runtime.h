#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_POOL_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_POOL_RUNTIME_H_

#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

namespace pvp_assault {

// IDA unit: pvp_assault::CAssaultPlaceFactory / 
// StaticPool<pvp_assault::CAssaultPlace,128>.  This is the original fixed
// 128-node pool, not a dynamically allocated replacement state machine.
class CAssaultPlaceFactory : public StaticPool<CAssaultPlace, 128> {
public:
    // IDA_EA: 0x82F0F96
    // IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactoryC2Ev
    CAssaultPlaceFactory();

    // IDA_EA: 0x82F0F82
    // IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactoryD2Ev
    ~CAssaultPlaceFactory();

    // IDA_EA: 0x82F0F54
    // IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactory19AcquireAssaultPlaceEv
    CAssaultPlace* AcquireAssaultPlace();

    // IDA_EA: 0x82F0F68
    // IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactory16FreeAssaultPlaceEPNS_13CAssaultPlaceE
    CAssaultPlace* FreeAssaultPlace(CAssaultPlace* place);
};

}  // namespace pvp_assault

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_POOL_RUNTIME_H_
