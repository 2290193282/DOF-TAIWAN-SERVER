#include "common/manager/server_utility.h"

#include <stdlib.h>

// IDA_EA: 0x86B1B87
// IDA_NAME: _Z12get_rand_inti
// DEMANGLED: get_rand_int(int)
unsigned int get_rand_int(int limit) {
    if (limit < 0) {
        return 0;
    }
    if (limit == 0) {
        return static_cast<unsigned int>(::rand());
    }

    int first = ::rand();
    if (first > limit) {
        return static_cast<unsigned int>(::rand() % limit);
    }

    unsigned int value =
        ((1103515245u *
              (1103515245u * (1103515245u * first + 12345u) + 12345u) +
          12345u) /
         0x10000u) &
        0x3FFu;
    value ^=
        ((((1103515245u * (1103515245u * first + 12345u) + 12345u) /
           0x10000u) &
          0x3FFu) ^
         ((((1103515245u * first + 12345u) / 0x10000u) & 0x7FFu) << 10)) << 10;

    if (static_cast<unsigned int>(limit) >= value) {
        return value;
    }
    return value % static_cast<unsigned int>(limit);
}

namespace server_utility {

// IDA_EA: 0x808EAFC
// IDA_NAME: _ZN14server_utility12get_rand_intEi
// DEMANGLED: server_utility::get_rand_int(int)
unsigned int get_rand_int(int limit) {
    if (limit < 0) {
        return 0;
    }
    if (limit == 0) {
        return static_cast<unsigned int>(::rand());
    }

    int first = ::rand();
    if (first > limit) {
        return static_cast<unsigned int>(::rand() % limit);
    }

    unsigned int value =
        ((1103515245u *
              (1103515245u * (1103515245u * first + 12345u) + 12345u) +
          12345u) /
         0x10000u) &
        0x3FFu;
    value ^=
        ((((1103515245u * (1103515245u * first + 12345u) + 12345u) /
           0x10000u) &
          0x3FFu) ^
         ((((1103515245u * first + 12345u) / 0x10000u) & 0x7FFu) << 10)) << 10;

    if (static_cast<unsigned int>(limit) >= value) {
        return value;
    }
    return value % static_cast<unsigned int>(limit);
}

}  // namespace server_utility
