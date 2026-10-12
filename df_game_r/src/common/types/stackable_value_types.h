#ifndef OLDSRV_GLOBAL_STACKABLE_VALUE_TYPES_H_
#define OLDSRV_GLOBAL_STACKABLE_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>


// IDA unit: 99_misc_global/STRecovery. Public value record embedded by
// IDA_EA: STRecovery @0x89C3098/0x89C30AC; STBuff @0x89C3194/0x89C31A8;
// StackableItemData @0x891BCCC/0x891BCE0.
// STStackableScript; full stackable import/population remains B-owned.
struct STRecovery {
    STRecovery();

    STRecovery* clear();

    uint8_t sign;
    uint8_t reserved_01[3];
    int32_t value;
    int32_t duration;
    int32_t target;
    uint8_t enabled;
    uint8_t reserved_11[3];
};

static_assert(sizeof(STRecovery) == 0x14u,
              "STRecovery IDA ABI size must stay 0x14");

// IDA unit: 99_misc_global/STStatusRecovery.
struct STStatusRecovery {
    STStatusRecovery();

    STStatusRecovery* clear();

    int32_t status_type;
    int32_t value;
    int32_t target;
    int32_t value_0c;
    uint8_t enabled;
    uint8_t reserved_11[3];
};

static_assert(sizeof(STStatusRecovery) == 0x14u,
              "STStatusRecovery IDA ABI size must stay 0x14");

// IDA unit: 99_misc_global/STBuff.
struct STBuff {
    STBuff();

    STBuff* clear();

    int32_t buff_type;
    int32_t value;
    int32_t target;
    uint8_t enabled;
    uint8_t reserved_0d[3];
};

static_assert(sizeof(STBuff) == 0x10u,
              "STBuff IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/STSummonMonster.
struct STSummonMonster {
    STSummonMonster();

    STSummonMonster* clear();

    int32_t values_00[6];
    uint8_t enabled;
    uint8_t reserved_19[3];
};

static_assert(sizeof(STSummonMonster) == 0x1Cu,
              "STSummonMonster IDA ABI size must stay 0x1C");

// IDA unit: 99_misc_global/STSummonApc.
struct STSummonApc {
    STSummonApc();

    STSummonApc* clear();

    int32_t values_00[5];
    uint8_t enabled;
    uint8_t reserved_15[3];
};

static_assert(sizeof(STSummonApc) == 0x18u,
              "STSummonApc IDA ABI size must stay 0x18");

// IDA unit: 99_misc_global/STPassiveObjectInStackable.
struct STPassiveObjectInStackable {
    STPassiveObjectInStackable();
    ~STPassiveObjectInStackable();

    int32_t values_00[5];
};

static_assert(sizeof(STPassiveObjectInStackable) == 0x14u,
              "STPassiveObjectInStackable IDA ABI size must stay 0x14");

// IDA unit: 99_misc_global/STStackablePackageData.
struct STStackablePackageData {
    STStackablePackageData();
    ~STStackablePackageData();

    int32_t item_index;
    int32_t item_count;
};

static_assert(sizeof(STStackablePackageData) == 0x08u,
              "STStackablePackageData IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/StackableItemData. Public reward item payload used
// by stackable reward containers; importer/container behavior remains owner
// work in DataManager/event modules.
struct StackableItemData {
    StackableItemData();
    ~StackableItemData();

    StackableItemData* clear();

    int32_t item_index;  // +0x00, default -1
    int32_t item_count;  // +0x04
};

static_assert(sizeof(StackableItemData) == 0x08u,
              "StackableItemData IDA ABI size must stay 0x08");

static_assert(offsetof(STRecovery, value) == 0x04u &&
                  offsetof(STRecovery, enabled) == 0x10u,
              "STRecovery IDA field offsets must stay exact");
static_assert(offsetof(STStatusRecovery, enabled) == 0x10u,
              "STStatusRecovery IDA enabled offset must stay 0x10");
static_assert(offsetof(STBuff, enabled) == 0x0cu,
              "STBuff IDA enabled offset must stay 0x0C");
static_assert(offsetof(STSummonMonster, enabled) == 0x18u,
              "STSummonMonster IDA enabled offset must stay 0x18");
static_assert(offsetof(STSummonApc, enabled) == 0x14u,
              "STSummonApc IDA enabled offset must stay 0x14");
static_assert(offsetof(STStackablePackageData, item_count) == 0x04u,
              "STStackablePackageData IDA item-count offset must stay 0x04");


#endif  // OLDSRV_GLOBAL_STACKABLE_VALUE_TYPES_H_
