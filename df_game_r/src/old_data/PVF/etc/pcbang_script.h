#ifndef OLDSRV_OLD_DATA_PVF_ETC_PCBANG_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_PCBANG_SCRIPT_H_

#include <stdint.h>

#include <utility>
#include <vector>

// IDA unit: 99_misc_global/stNeoplePCBANG.  The original enum has a signed
// 32-bit underlying type and is stored in the status/value pair below.
enum ENUM_CHANGE_STATUS_TYPE : int {
};

// IDA_EA: 0x891AC9A (clear), 0x891ACCE (C2), 0x891C6C2 (D2).
// The third scalar is cleared and copied by the original lifecycle but is not
// assigned by importPCBANGScript's tag grammar.
struct stNeoplePCBANG {
    int32_t level;
    int32_t people_count;
    int32_t unknown_08;
    std::vector<std::pair<ENUM_CHANGE_STATUS_TYPE, int> > status_values;

    stNeoplePCBANG();
    stNeoplePCBANG(const stNeoplePCBANG& other);
    ~stNeoplePCBANG();
    stNeoplePCBANG& operator=(const stNeoplePCBANG& other);

    int clear();
};

static_assert(sizeof(stNeoplePCBANG) == 0x18u,
              "stNeoplePCBANG IDA ABI size must stay 0x18");

// IDA_EA: 0x8905773
// IDA_NAME: _Z18importPCBANGScriptPKcRSt6vectorI14stNeoplePCBANGSaIS2_EE
// DEMANGLED: importPCBANGScript(char const*,std::vector<stNeoplePCBANG,std::allocator<stNeoplePCBANG>> &)
int importPCBANGScript(const char* filename,
                       std::vector<stNeoplePCBANG>& entries);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_PCBANG_SCRIPT_H_
