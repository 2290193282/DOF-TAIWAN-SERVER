#include "common/manager/reseal_table.h"

// IDA_EA: 0x8370742
// IDA_NAME: _ZN14reseal_table_tC2Ev
// DEMANGLED: reseal_table_t::reseal_table_t(void)
reseal_table_t::reseal_table_t() : grade_costs() {
    for (int index = 0; index <= 5; ++index) {
        rarity_weights[index] = 0;
    }
    for (int index = 0; index <= 5; ++index) {
        extended_rarity_weights[index] = 0;
    }
}

// IDA_EA: 0x837BB2E
// IDA_NAME: _ZN14reseal_table_tD2Ev
// DEMANGLED: reseal_table_t::~reseal_table_t()
reseal_table_t::~reseal_table_t() {}

// IDA_EA: 0x8900B02
// IDA_NAME: _ZN14reseal_table_t15get_reseal_costEhhh
// DEMANGLED: reseal_table_t::get_reseal_cost(unsigned char,unsigned char,unsigned char)
int reseal_table_t::get_reseal_cost(unsigned char rarity,
                                    unsigned char grade,
                                    unsigned char reseal_count) {
    if (!grade) {
        return -1;
    }
    if (rarity > 5u) {
        return -2;
    }
    if (grade > 0x63u) {
        return -3;
    }
    if (reseal_count > 6u) {
        return -4;
    }

    unsigned int grade_index = (grade - 1) / 10;
    if (grade_costs.size() <= grade_index) {
        grade_index = grade_costs.size() - 1;
    }
    return static_cast<int>(rarity_weights[rarity] *
                                grade_costs[grade_index] +
                            extended_rarity_weights[rarity] * reseal_count);
}

// IDA_EA: 0x8900BE0
// IDA_NAME: _ZN14reseal_table_t23get_oneshot_reseal_costEhhh
// DEMANGLED: reseal_table_t::get_oneshot_reseal_cost(unsigned char,unsigned char,unsigned char)
int reseal_table_t::get_oneshot_reseal_cost(unsigned char rarity,
                                            unsigned char grade,
                                            unsigned char) {
    if (!grade) {
        return -1;
    }
    if (rarity > 5u) {
        return -2;
    }
    if (grade <= 0x63u) {
        return 1;
    }
    return -3;
}
