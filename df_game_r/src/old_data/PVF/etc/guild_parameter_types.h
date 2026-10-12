#ifndef OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_TYPES_H_

#include <stdint.h>


// IDA unit: 99_misc_global/STAgitUpgradeInfo. Public value record used by
// IDA_EA: STAgitUpgradeInfo @0x8979978/0x897994C/0x897A678;
// STGuildStorageUpgradeInfo @0x89799C2/0x897998C/0x897A88A.
// GuildParameterScript vectors; script import/population remains B-owned.
struct STAgitUpgradeInfo {
    STAgitUpgradeInfo();
    ~STAgitUpgradeInfo();

    STAgitUpgradeInfo* clear();

    int32_t values[4];
};

static_assert(sizeof(STAgitUpgradeInfo) == 0x10u,
              "STAgitUpgradeInfo IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/STGuildStorageUpgradeInfo. Public value record used
// by GuildParameterScript vectors; storage upgrade semantics remain owner work.
struct STGuildStorageUpgradeInfo {
    STGuildStorageUpgradeInfo();
    ~STGuildStorageUpgradeInfo();

    STGuildStorageUpgradeInfo* clear();

    int32_t values[5];
};

static_assert(sizeof(STGuildStorageUpgradeInfo) == 0x14u,
              "STGuildStorageUpgradeInfo IDA ABI size must stay 0x14");


#endif  // OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_TYPES_H_
