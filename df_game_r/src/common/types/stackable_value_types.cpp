#include "common/types/stackable_value_types.h"


// IDA_EA: 0x89C3098
// IDA_NAME: _ZN10STRecoveryC2Ev
// DEMANGLED: STRecovery::STRecovery(void)
STRecovery::STRecovery() {
    clear();
}

// IDA_EA: 0x89C30AC
// IDA_NAME: _ZN10STRecovery5clearEv
// DEMANGLED: STRecovery::clear(void)
STRecovery* STRecovery::clear() {
    sign = 0;
    value = 0;
    duration = 0;
    target = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C30DC
// IDA_NAME: _ZN16STStatusRecoveryC2Ev
// DEMANGLED: STStatusRecovery::STStatusRecovery(void)
STStatusRecovery::STStatusRecovery() {
    clear();
}

// IDA_EA: 0x89C30F0
// IDA_NAME: _ZN16STStatusRecovery5clearEv
// DEMANGLED: STStatusRecovery::clear(void)
STStatusRecovery* STStatusRecovery::clear() {
    status_type = 0;
    value = 0;
    target = 0;
    value_0c = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C3194
// IDA_NAME: _ZN6STBuffC2Ev
// DEMANGLED: STBuff::STBuff(void)
STBuff::STBuff() {
    clear();
}

// IDA_EA: 0x89C31A8
// IDA_NAME: _ZN6STBuff5clearEv
// DEMANGLED: STBuff::clear(void)
STBuff* STBuff::clear() {
    buff_type = 0;
    value = 0;
    target = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C31D2
// IDA_NAME: _ZN15STSummonMonsterC2Ev
// DEMANGLED: STSummonMonster::STSummonMonster(void)
STSummonMonster::STSummonMonster() {
    clear();
}

// IDA_EA: 0x89C31E6
// IDA_NAME: _ZN15STSummonMonster5clearEv
// DEMANGLED: STSummonMonster::clear(void)
STSummonMonster* STSummonMonster::clear() {
    for (int index = 0; index != 6; ++index) {
        values_00[index] = 0;
    }
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C3238
// IDA_NAME: _ZN11STSummonApcC2Ev
// DEMANGLED: STSummonApc::STSummonApc(void)
STSummonApc::STSummonApc() {
    clear();
}

// IDA_EA: 0x89C324C
// IDA_NAME: _ZN11STSummonApc5clearEv
// DEMANGLED: STSummonApc::clear(void)
STSummonApc* STSummonApc::clear() {
    for (int index = 0; index != 5; ++index) {
        values_00[index] = 0;
    }
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C34E8
// IDA_NAME: _ZN26STPassiveObjectInStackableC2Ev
// DEMANGLED: STPassiveObjectInStackable::STPassiveObjectInStackable(void)
STPassiveObjectInStackable::STPassiveObjectInStackable() {
    for (int index = 0; index != 5; ++index) {
        values_00[index] = 0;
    }
}

// IDA_EA: 0x89D30BC
// IDA_NAME: _ZN26STPassiveObjectInStackableD2Ev
// DEMANGLED: STPassiveObjectInStackable::~STPassiveObjectInStackable()
STPassiveObjectInStackable::~STPassiveObjectInStackable() {
}

// IDA_EA: 0x8919346
// IDA_NAME: _ZN22STStackablePackageDataC2Ev
// DEMANGLED: STStackablePackageData::STStackablePackageData(void)
STStackablePackageData::STStackablePackageData() {
    item_index = 0;
    item_count = 0;
}

// IDA_EA: 0x827EED0
// IDA_NAME: _ZN22STStackablePackageDataD2Ev
// DEMANGLED: STStackablePackageData::~STStackablePackageData()
STStackablePackageData::~STStackablePackageData() {
}

// IDA_EA: 0x891BCCC
// IDA_NAME: _ZN17StackableItemDataC2Ev
// DEMANGLED: StackableItemData::StackableItemData(void)
StackableItemData::StackableItemData() {
    clear();
}

// IDA_EA: 0x891BCE0
// IDA_NAME: _ZN17StackableItemData5clearEv
// DEMANGLED: StackableItemData::clear(void)
StackableItemData* StackableItemData::clear() {
    item_index = -1;
    item_count = 0;
    return this;
}

// IDA_EA: 0x8935C36
// IDA_NAME: _ZN17StackableItemDataD2Ev
// DEMANGLED: StackableItemData::~StackableItemData()
StackableItemData::~StackableItemData() {
}
