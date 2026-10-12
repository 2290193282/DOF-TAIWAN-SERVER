#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_GROWTH_TABLES_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_GROWTH_TABLES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x834828C in the companion source.


#include <stdint.h>

#include <utility>
#include <vector>

#include "common/config.h"
#include "common/manager/play_info_public_abi.h"


struct STCharacterScript;

// IDA unit: 99_misc_global/STSecondGrowTypeInfo. Old i386 size: 0xc0.
// It is embedded seven times by CCharacter at +0x294.
struct STSecondGrowTypeInfo {
    STSecondGrowTypeInfo();
    ~STSecondGrowTypeInfo();

    void* clear();

    _Additioal_info status[2];
    std::vector<std::pair<int, int> > skill_values[2];
    uint8_t grow_flags[2];
};

class CCharacter {
public:
    CCharacter();
    ~CCharacter();

    bool clear();
    bool destroy();
    int get_give_skill_firstGrowtype(
        int charac_job,
        std::vector<std::pair<int, int> >& skill_values) const;
    int get_give_skill_secondGrowtype(
        int charac_job, int second_grow_type,
        std::vector<std::pair<int, int> >& skill_values) const;
    int get_give_skill(int charac_job, int second_grow_type,
                       std::vector<std::pair<int, int> >& skill_values,
                       unsigned int grow_stage) const;
    _Additioal_info* get_growtype_stat(int charac_job, int grow_type) const;
    bool set_data(STCharacterScript* script);

    // DisPatcher_CreateCharac @0x81C0638 copies the first script flag at
    // physical +0x7d4 into SIG_CREATE_CHARAC::initial_grow_flag.  The field
    // belongs to CCharacter; expose that named owner instead of making the
    // dispatcher reproduce CDataManager/CCharacList byte arithmetic.
    uint8_t get_initial_create_grow_flag() const {
        return script_status_flags_[0];
    }

    uint8_t get_start_town() const {
        return start_town_;
    }

    // IDA physical storage at +0x000.  CCalcCharacStat::CalcStat and
    // CUser::calc_lev_stat both consume status zero directly before applying
    // first- and second-grow increments.
    _Additioal_info base_status_[7];

private:
    // CDataManager owns the contiguous CCharacList and has one original
    // consumer (CMD509) which reads job 9's first-grow skill vector directly.
    // Friendship confines that recovered physical access to its real owner.
    friend class CDataManager;

    uint8_t alignment_padding_23e_[2];
    std::vector<std::pair<int, int> > first_grow_skills_[7];
    STSecondGrowTypeInfo second_grow_infos_[7];
    uint8_t script_status_flags_[7];
    uint8_t start_town_;
};

// IDA unit: 04_character_select_create/CCharacList.
class CCharacList {
public:
    CCharacList();
    ~CCharacList();

    int init();
    bool destroy();

    // IDA physical storage at +0x000. Kept directly addressable for the
    // CDataManager-owned character-list boundary; no accessor ABI is added.
    CCharacter characters_[oldsrv::config::kCharacterJobCount];
};


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_GROWTH_TABLES_H_
