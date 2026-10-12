#ifndef OLDSRV_PVF_DUNGEON_DUNGEON_SCRIPT_VALUE_TYPES_H_
#define OLDSRV_PVF_DUNGEON_DUNGEON_SCRIPT_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

// IDA unit: 99_misc_global/AbsoluteChangeStat.
// ImportDungeonScript @ 0x88E5FAA maps the five accepted text labels to this
// exact int-backed nested enum before inserting its +0x290 map.
struct AbsoluteChangeStat {
    enum T {
        MOVE_SPEED = 0,
        PHYSICAL_ATTACK_ADD = 1,
        PHYSICAL_DEFENSE_ADD = 2,
        MAGICAL_ATTACK_ADD = 3,
        MAGICAL_DEFENSE_ADD = 4,
    };
};

// IDA_EA: D2 0x88F9346.
// Each [tower recovery] row supplies these three IEEE-754 values.
struct RecoveryInfo {
    float value_00;
    float value_04;
    float value_08;

    ~RecoveryInfo();
};

// IDA_EA: C2 0x88F2E84, copy C2 0x88F627E, D2 0x83DC7D2,
//         operator= 0x88F6344.
struct RecoveryStairInfo {
    RecoveryStairInfo();
    RecoveryStairInfo(const RecoveryStairInfo& rhs);
    ~RecoveryStairInfo();

    RecoveryStairInfo& operator=(const RecoveryStairInfo& rhs);

    int32_t value_00;
    int32_t value_04;
    float value_08;
    std::vector<RecoveryInfo> recovery_infos;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(RecoveryInfo) == 0x0c,
              "RecoveryInfo IDA ABI size mismatch");
static_assert(offsetof(RecoveryStairInfo, recovery_infos) == 0x0c,
              "RecoveryStairInfo recovery vector offset mismatch");
static_assert(sizeof(RecoveryStairInfo) == 0x18,
              "RecoveryStairInfo IDA ABI size mismatch");
#endif

#endif  // OLDSRV_PVF_DUNGEON_DUNGEON_SCRIPT_VALUE_TYPES_H_
