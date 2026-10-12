#include "old_data/PVF/etc/guild_parameter_types.h"


// IDA_EA: 0x8979978
// IDA_NAME: _ZN17STAgitUpgradeInfoC2Ev
// DEMANGLED: STAgitUpgradeInfo::STAgitUpgradeInfo(void)
STAgitUpgradeInfo::STAgitUpgradeInfo() {
    clear();
}

// IDA_EA: 0x897994C
// IDA_NAME: _ZN17STAgitUpgradeInfo5clearEv
// DEMANGLED: STAgitUpgradeInfo::clear(void)
STAgitUpgradeInfo* STAgitUpgradeInfo::clear() {
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;
    return this;
}

// IDA_EA: 0x897A678
// IDA_NAME: _ZN17STAgitUpgradeInfoD2Ev
// DEMANGLED: STAgitUpgradeInfo::~STAgitUpgradeInfo()
STAgitUpgradeInfo::~STAgitUpgradeInfo() {
}

// IDA_EA: 0x89799C2
// IDA_NAME: _ZN25STGuildStorageUpgradeInfoC2Ev
// DEMANGLED: STGuildStorageUpgradeInfo::STGuildStorageUpgradeInfo(void)
STGuildStorageUpgradeInfo::STGuildStorageUpgradeInfo() {
    clear();
}

// IDA_EA: 0x897998C
// IDA_NAME: _ZN25STGuildStorageUpgradeInfo5clearEv
// DEMANGLED: STGuildStorageUpgradeInfo::clear(void)
STGuildStorageUpgradeInfo* STGuildStorageUpgradeInfo::clear() {
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;
    values[4] = 0;
    return this;
}

// IDA_EA: 0x897A88A
// IDA_NAME: _ZN25STGuildStorageUpgradeInfoD2Ev
// DEMANGLED: STGuildStorageUpgradeInfo::~STGuildStorageUpgradeInfo()
STGuildStorageUpgradeInfo::~STGuildStorageUpgradeInfo() {
}
