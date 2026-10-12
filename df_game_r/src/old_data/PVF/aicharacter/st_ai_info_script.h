#ifndef OLDSRV_OLD_DATA_PVF_AICHARACTER_ST_AI_INFO_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_AICHARACTER_ST_AI_INFO_SCRIPT_H_

#include <stdint.h>

#include <string>
#include <vector>

// IDA type: STAIInfoScript.  The five string-pointer lists, 22 fixed values,
// 22 integer lists, five trailing lists, and scalar tail occupy 0x204 bytes
// on the original i386 ABI.
class STAIInfoScript {
public:
    STAIInfoScript();
    STAIInfoScript(const STAIInfoScript& other);
    virtual ~STAIInfoScript();

    STAIInfoScript& operator=(const STAIInfoScript& other);
    STAIInfoScript* clear();
    bool destroy();

    int32_t value_04;
    int32_t value_08;
    std::string value_0C;
    std::vector<std::string*> string_lists[5];
    int32_t values_4C[22];
    std::vector<int> int_lists[22];
    int32_t value_1AC;
    std::vector<int> values_1B0;
    std::vector<int> values_1BC;
    std::vector<int> values_1C8;
    std::vector<int> values_1D4;
    std::vector<int> values_1E0;
    int32_t value_1EC;
    int32_t value_1F0;
    int32_t value_1F4;
    int32_t value_1F8;
    int32_t value_1FC;
    uint8_t value_200;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STAIInfoScript) == 0x204u,
              "STAIInfoScript IDA ABI size must stay 0x204");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_AICHARACTER_ST_AI_INFO_SCRIPT_H_
