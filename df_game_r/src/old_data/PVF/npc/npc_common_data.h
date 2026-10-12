#ifndef OLDSRV_OLD_DATA_PVF_NPC_NPC_COMMON_DATA_H_
#define OLDSRV_OLD_DATA_PVF_NPC_NPC_COMMON_DATA_H_

#include <stdint.h>

#include <utility>
#include <vector>

// IDA unit: 07_npc_shop_dialog/stNPCCommonData_t.
// These values are PVF/RDAR configuration owned by B.  NPC mood and user
// interaction state remain in C's runtime modules.
struct stNPCCommonData_t {
    struct stOneBuff_t {
        int32_t change_status_type;
        int32_t value;
        uint8_t is_percent;

        // IDA_EA: 0x8A04B54
        // IDA_NAME: _ZN17stNPCCommonData_t11stOneBuff_tD2Ev
        // DEMANGLED: stNPCCommonData_t::stOneBuff_t::~stOneBuff_t()
        ~stOneBuff_t();
    };

    // IDA_EA: 0x837612E
    // IDA_NAME: _ZN17stNPCCommonData_tC2Ev
    // DEMANGLED: stNPCCommonData_t::stNPCCommonData_t(void)
    stNPCCommonData_t();
    // IDA_EA: 0x83761C6
    // IDA_NAME: _ZN17stNPCCommonData_tD2Ev
    // DEMANGLED: stNPCCommonData_t::~stNPCCommonData_t()
    ~stNPCCommonData_t();
    // IDA_EA: 0x89FAE9E
    // IDA_NAME: _ZN17stNPCCommonData_t5clearEv
    // DEMANGLED: stNPCCommonData_t::clear(void)
    stNPCCommonData_t* clear();
    // IDA_EA: 0x89FAEEE
    // IDA_NAME: _ZN17stNPCCommonData_t14getIllustIndexEi
    // DEMANGLED: stNPCCommonData_t::getIllustIndex(int)
    int getIllustIndex(int illust_id);

    uint16_t max_favor_value;
    uint16_t favor_level_values[10];
    std::vector<int> favorable_npc_indices;
    std::vector<int> illust_indices;
    uint16_t favor_system_begin_quest;
    uint16_t favor_system_begin_level;
    uint16_t max_favor_level_npc;
    std::vector<int> favor_level_reward_npc_indices;
    std::vector<std::pair<int, std::vector<stOneBuff_t> > >
        favor_duration_buffs;
};

// The original NPC RDAR list and common-data parser share this global base
// directory.  Its single definition is in the B canonical provider.
extern "C" char g_npcScriptBaseDirectory[0x101];

// IDA_EA: 0x89FAFC5
// IDA_NAME: _Z17loadNPCCommonDataR17stNPCCommonData_tPKc
// DEMANGLED: loadNPCCommonData(stNPCCommonData_t &,char const*)
int loadNPCCommonData(stNPCCommonData_t& data, const char* filename);

static_assert(sizeof(stNPCCommonData_t::stOneBuff_t) == 0x0Cu,
              "stNPCCommonData_t::stOneBuff_t IDA ABI size must stay 0x0C");
static_assert(sizeof(stNPCCommonData_t) == 0x50u,
              "stNPCCommonData_t IDA ABI size must stay 0x50");

#endif  // OLDSRV_OLD_DATA_PVF_NPC_NPC_COMMON_DATA_H_
