#ifndef OLDSRV_GLOBAL_HELLPARTY_CHARACLINK_ABI_H_
#define OLDSRV_GLOBAL_HELLPARTY_CHARACLINK_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "old_game/player/core/character_misc_data.h"

// CDataManager old embedded CharacLinkSystem slot. HellParty is a function
// local static object, not a CDataManager member.
// IDA unit: 09_dungeon_pvp_party/stHellPartyMonsterGroupList.
struct stHellPartyMonsterGroupList {
    stHellPartyMonsterGroupList();

    int32_t monster_index[16];       // +0x00
    int8_t monster_grade[16];        // +0x40
    uint8_t difficulty_value;        // +0x50
    uint8_t reserved_51;             // +0x51
    int16_t difficulty_group_value;  // +0x52
    uint8_t group_order;             // +0x54
    uint8_t reserved_55[3];          // +0x55
};

static_assert(sizeof(stHellPartyMonsterGroupList) == 0x58u,
              "stHellPartyMonsterGroupList IDA size must stay 0x58");
static_assert(offsetof(stHellPartyMonsterGroupList, monster_grade) == 0x40u,
              "stHellPartyMonsterGroupList grade offset must stay 0x40");
static_assert(offsetof(stHellPartyMonsterGroupList,
                       difficulty_group_value) == 0x52u,
              "stHellPartyMonsterGroupList difficulty group offset must stay 0x52");

// IDA unit: 09_dungeon_pvp_party/stHellParty_t.
struct stHellParty_t {
    stHellParty_t();
    ~stHellParty_t();

    int32_t max_base_value;       // +0x00
    int32_t min_base_value;       // +0x04
    int32_t base_value;           // +0x08
    int32_t base_rising_value;    // +0x0c
    float sub_base_rates;         // +0x10
    float pass_sub_base_rates;    // +0x14
    float level_difference_rates[4];  // +0x18, IDA memset covers 0x10 bytes
    float difficulty_rates[5];    // +0x28
    float party_rates[4];         // +0x3c
    float ancient_dungeon_add_rate;  // +0x4c
    uint16_t current_group_index; // +0x50
    uint8_t reserved_52[2];       // +0x52
    std::map<uint16_t, stHellPartyMonsterGroupList> monster_groups;  // +0x54
    uint8_t difficulty_value[6];        // +0x6c
    int16_t difficulty_group_value[6];  // +0x72
    uint8_t difficulty_extra_7e[6];      // +0x7e
    uint8_t difficulty_extra_84[6];      // +0x86
    uint8_t difficulty_extra_8a[6];      // +0x8c
    int32_t guide_line_value;            // +0x90
};

static_assert(sizeof(stHellParty_t) == 0x94u,
              "stHellParty_t IDA size must stay 0x94");
static_assert(offsetof(stHellParty_t, monster_groups) == 0x54u,
              "stHellParty_t monster map offset must stay 0x54");
static_assert(offsetof(stHellParty_t, difficulty_value) == 0x6cu,
              "stHellParty_t difficulty value offset must stay 0x6c");
static_assert(offsetof(stHellParty_t, difficulty_group_value) == 0x72u,
              "stHellParty_t difficulty group offset must stay 0x72");
static_assert(offsetof(stHellParty_t, guide_line_value) == 0x90u,
              "stHellParty_t guide-line offset must stay 0x90");

// C04 owns this public IDA layout and its lifecycle functions. This importer
// consumes the same four vectors in their original offset order.
static_assert(sizeof(stCharacLinkSystemData) == 0x10u,
              "stCharacLinkSystemData IDA size must stay 0x10");
static_assert(offsetof(stCharacLinkSystemData, minimum_character_level) ==
                  0x00u,
              "charac-link row minimum level must stay at +0x00");
static_assert(offsetof(stCharacLinkSystemData, maximum_character_level) ==
                  0x04u,
              "charac-link row maximum level must stay at +0x04");
static_assert(offsetof(stCharacLinkSystemData, bonus_rate_percent) == 0x08u,
              "charac-link row bonus rate must stay at +0x08");
static_assert(offsetof(stCharacLinkSystemData, bonus_value) == 0x0cu,
              "charac-link row bonus value must stay at +0x0c");
static_assert(sizeof(stCharacLinkSystem_t) == 0x34u,
              "stCharacLinkSystem_t IDA size must stay 0x34");
static_assert(offsetof(stCharacLinkSystem_t, striker_skill_minimum_level) ==
                  0x00u,
              "stCharacLinkSystem_t striker level gate must stay at +0x00");
static_assert(offsetof(stCharacLinkSystem_t,
                       low_link_character_exp_bonus_table) == 0x04u,
              "low-link EXP table offset must stay 0x04");
static_assert(offsetof(stCharacLinkSystem_t,
                       low_link_character_gold_bonus_table) == 0x10u,
              "low-link gold table offset must stay 0x10");
static_assert(offsetof(stCharacLinkSystem_t,
                       high_link_character_exp_bonus_table) == 0x1cu,
              "high-link EXP table offset must stay 0x1c");
static_assert(offsetof(stCharacLinkSystem_t,
                       high_link_character_gold_bonus_table) == 0x28u,
              "high-link gold table offset must stay 0x28");

// IDA_EA: 0x89097C4
// IDA_NAME: _Z26Get_instance_stHellParty_tv
// DEMANGLED: Get_instance_stHellParty_t(void)
stHellParty_t* Get_instance_stHellParty_t();

// IDA_EA: 0x8909857
// IDA_NAME: _Z21importHellPartyScriptPKcR13stHellParty_t
// DEMANGLED: importHellPartyScript(char const*,stHellParty_t &)
int importHellPartyScript(const char* filename, stHellParty_t& script);

// IDA_EA: 0x8909529
// IDA_NAME: _Z28importCharacLinkSystemScriptPKcR20stCharacLinkSystem_t
// DEMANGLED: importCharacLinkSystemScript(char const*,stCharacLinkSystem_t &)
int importCharacLinkSystemScript(const char* filename,
                                 stCharacLinkSystem_t& script);


#endif  // OLDSRV_GLOBAL_HELLPARTY_CHARACLINK_ABI_H_
