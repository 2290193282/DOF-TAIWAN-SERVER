#ifndef OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_TYPES_H_

#include <stdint.h>

#include <utility>
#include <vector>


// B PVF/Etc value parsed by IndependentDropParameterScript. Old i386 size:
// 0x48.
// IDA_EA: copy/dtor/operator=/ctor @0x8376BB2/0x8376C5A/0x83A3942/0x897CBC4.
struct stMonsterIndependentDrop_t {
    stMonsterIndependentDrop_t();
    stMonsterIndependentDrop_t(const stMonsterIndependentDrop_t& other);
    ~stMonsterIndependentDrop_t();

    stMonsterIndependentDrop_t& operator=(
        const stMonsterIndependentDrop_t& other);

    int32_t value_00;    // +0x00
    int32_t value_04;    // +0x04
    int32_t values[9];   // +0x08
    int32_t total_rate;  // +0x2C
    std::vector<std::pair<int, int> > drops;  // +0x30
    int32_t min_count;  // +0x3C
    int32_t max_count;  // +0x40
    int32_t rate;       // +0x44
};

static_assert(sizeof(stMonsterIndependentDrop_t) == 0x48u,
              "stMonsterIndependentDrop_t IDA ABI size must stay 0x48");


#endif  // OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_TYPES_H_
