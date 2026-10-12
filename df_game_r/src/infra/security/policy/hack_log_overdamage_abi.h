#ifndef OLDSRV_GLOBAL_HACK_LOG_OVERDAMAGE_ABI_H_
#define OLDSRV_GLOBAL_HACK_LOG_OVERDAMAGE_ABI_H_

#include <stdint.h>

class CUser;

namespace WongWork {
enum ENUM_HACKTYPE : int;
}

// IDA unit: CHackLog_OverDamage.  It has no object fields; all seven
// functions are class-namespaced static helpers consumed by CMD42.
class CHackLog_OverDamage {
public:
    // IDA_EA: 0x8286790
    // IDA_NAME: _ZN19CHackLog_OverDamage32GetDungeonLimitDamageAndHackTypeEP5CUserRjRN8WongWork13ENUM_HACKTYPEE
    static int GetDungeonLimitDamageAndHackType(
        CUser* user,
        uint32_t& limit_damage,
        WongWork::ENUM_HACKTYPE& hack_type);

    // IDA_EA: 0x8286984 / 0x828698E
    static int GetDeathTowerLimitDamage();
    static int GetDeathTowerOverDamageHackType();

    // IDA_EA: 0x8286998 / 0x82869A2
    static int GetDungeonLimitMaxDamage();
    static int GetHackTypeOverLimitMaxDamage();

    // IDA_EA: 0x82869AC / 0x82869B6
    static int GetBossTowerLimitDamage();
    static int GetBossTowerOverDamageHackType();
};

#endif  // OLDSRV_GLOBAL_HACK_LOG_OVERDAMAGE_ABI_H_
