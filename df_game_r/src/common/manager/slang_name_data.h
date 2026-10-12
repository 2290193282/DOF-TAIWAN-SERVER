#ifndef OLDSRV_GLOBAL_SLANG_NAME_DATA_H_
#define OLDSRV_GLOBAL_SLANG_NAME_DATA_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

// IDA unit: 99_misc_global/SlangNameData.
struct SlangNameData {
    SlangNameData();
    ~SlangNameData();

    std::vector<std::string> slang_names;       // +0x00
    std::vector<unsigned char> one_byte_skips;  // +0x0c
    std::vector<unsigned short> two_byte_skips; // +0x18
};

static_assert(sizeof(SlangNameData) == 0x24u,
              "SlangNameData IDA ABI size must stay 0x24");
static_assert(offsetof(SlangNameData, one_byte_skips) == 0x0cu,
              "SlangNameData one_byte_skips offset must stay 0x0c");
static_assert(offsetof(SlangNameData, two_byte_skips) == 0x18u,
              "SlangNameData two_byte_skips offset must stay 0x18");

using ::SlangNameData;


#endif  // OLDSRV_GLOBAL_SLANG_NAME_DATA_H_
