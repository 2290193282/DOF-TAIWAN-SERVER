#ifndef OLDSRV_GLOBAL_RESEAL_TABLE_H_
#define OLDSRV_GLOBAL_RESEAL_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

// IDA unit: 99_misc_global/reseal_table_t.
// IDA_EA: ctor @0x8370742, dtor @0x837BB2E,
// get_reseal_cost @0x8900B02, get_oneshot_reseal_cost @0x8900BE0.
// CDataManager embeds this table at old offset +0x2C; RDAR population remains
// in the DataManager/PVF importer owner.
struct reseal_table_t {
    reseal_table_t();
    ~reseal_table_t();

    int get_reseal_cost(unsigned char rarity,
                        unsigned char grade,
                        unsigned char reseal_count);
    int get_oneshot_reseal_cost(unsigned char rarity,
                                unsigned char grade,
                                unsigned char reseal_count);

    std::vector<unsigned int> grade_costs;
    int32_t rarity_weights[6];
    int32_t extended_rarity_weights[6];
};

static_assert(sizeof(reseal_table_t) == 0x3Cu,
              "reseal_table_t IDA ABI size must stay 0x3C");
static_assert(offsetof(reseal_table_t, rarity_weights) == 0x0Cu,
              "reseal_table_t rarity_weights offset must stay 0x0C");
static_assert(offsetof(reseal_table_t, extended_rarity_weights) == 0x24u,
              "reseal_table_t extended_rarity_weights offset must stay 0x24");


using ::reseal_table_t;

// IDA_EA: CDataManager::CDataManager @0x8355B30 constructs this slot;
// CDataManager::~CDataManager @0x8356B68 destroys it in reverse order.

#endif  // OLDSRV_GLOBAL_RESEAL_TABLE_H_
