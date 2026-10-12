#include "old_game/player/core/cuser.h"

#include "old_game/player/core/character_view_data.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"

#include <stdint.h>

// IDA_EA: 0x81253F4
// IDA_NAME: _ZN5CUser16EnableCharacInfoE13CACHE_SECTION
// DEMANGLED: CUser::EnableCharacInfo(CACHE_SECTION)
int CUser::EnableCharacInfo(CACHE_SECTION section) {
    cache_character_load_state_.section_loaded[static_cast<int>(section)] = 1;
    return static_cast<int>(section);
}

// IDA_EA: 0x81348CE
// IDA_NAME: _ZN5CUser17DisableCharacInfoE13CACHE_SECTION
// DEMANGLED: CUser::DisableCharacInfo(CACHE_SECTION)
int CUser::DisableCharacInfo(CACHE_SECTION section) {
    cache_character_load_state_.section_loaded[static_cast<int>(section)] = 0;
    return static_cast<int>(section);
}

bool CUser::IsCharacInfoLoaded(CACHE_SECTION section) const {
    const int index = static_cast<int>(section);
    return index >= 0 && index < CACHE_CHARACTER_LOAD_SECTION_COUNT &&
           cache_character_load_state_.section_loaded[index] != 0;
}

// IDA_EA: 0x81348E2
// IDA_NAME: _ZN5CUser20GetLastLoginCharacNoEv
// DEMANGLED: CUser::GetLastLoginCharacNo(void)
int CUser::GetLastLoginCharacNo() {
    return last_login_charac_no_;
}

// IDA_EA: 0x81348F0
// IDA_NAME: _ZN5CUser21GetLastLoginChannelNoEv
// DEMANGLED: CUser::GetLastLoginChannelNo(void)
int CUser::GetLastLoginChannelNo() {
    return last_login_channel_no_;
}

// IDA_EA: 0x81348FE
// IDA_NAME: _ZN5CUser22IsValidLastLoginCharacEv
// DEMANGLED: CUser::IsValidLastLoginCharac(void)
int CUser::IsValidLastLoginCharac() {
    return last_login_charac_valid_;
}

// IDA_EA: 0x822FD5C
// IDA_NAME: _ZN5CUser20SetLastLoginCharacNoEi
// DEMANGLED: CUser::SetLastLoginCharacNo(int)
CUser* CUser::SetLastLoginCharacNo(int charac_no) {
    last_login_charac_no_ = charac_no;
    return this;
}

// IDA_EA: 0x822FD6E
// IDA_NAME: _ZN5CUser21SetLastLoginChannelNoEi
// DEMANGLED: CUser::SetLastLoginChannelNo(int)
CUser* CUser::SetLastLoginChannelNo(int channel_no) {
    last_login_channel_no_ = channel_no;
    return this;
}

// IDA_EA: 0x822FD80
// IDA_NAME: _ZN5CUser23SetValidLastLoginCharacEb
// DEMANGLED: CUser::SetValidLastLoginCharac(bool)
CUser* CUser::SetValidLastLoginCharac(bool valid) {
    last_login_charac_valid_ = valid ? 1 : 0;
    return this;
}

// IDA_EA: 0x822FEB6
// IDA_NAME: _ZNK5CUser15get_charac_nameEi
// DEMANGLED: CUser::get_charac_name(int)const
const char* CUser::get_charac_name(int slot) const {
    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return 0;
    }
    return characters_[static_cast<std::size_t>(slot)]
        .fields_.character_name_;
}

// IDA_EA: 0x822FF0C
// IDA_NAME: _ZNK5CUser12GetCharacJobEi
// DEMANGLED: CUser::GetCharacJob(int)const
int CUser::GetCharacJob(int slot) const {
    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return 11;
    }
    return static_cast<unsigned char>(
        characters_[static_cast<std::size_t>(slot)].fields_.job_);
}

// IDA_EA: 0x822FF62
// IDA_NAME: _ZNK5CUser31CheckPossibleRequestRenameStateEi
// DEMANGLED: CUser::CheckPossibleRequestRenameState(int)const
bool CUser::CheckPossibleRequestRenameState(int slot) const {
    return slot >= 0 &&
           characters_.size() > static_cast<std::size_t>(slot) &&
           characters_[static_cast<std::size_t>(slot)]
                   .fields_.character_view_state_ == 255;
}

// IDA_EA: 0x822FFC2
// IDA_NAME: _ZN5CUser18SetCharacViewStateEi
// DEMANGLED: CUser::SetCharacViewState(int)
int CUser::SetCharacViewState(int state) {
    character_view_.enableSaveCharacView();
    return setCharacViewState(state);
}

// IDA_EA: 0x822FFEC
// IDA_NAME: _ZN5CUser18SetCharacViewStateEii
// DEMANGLED: CUser::SetCharacViewState(int,int)
int CUser::SetCharacViewState(int slot, int state) {
    _Charac_info* character = 0;
    if (slot >= 0 && characters_.size() > static_cast<std::size_t>(slot)) {
        character = &characters_[static_cast<std::size_t>(slot)];
    }
    int result = character == 0;
    if (!static_cast<unsigned char>(result)) {
        character_view_.enableSaveCharacView();
        result = reinterpret_cast<int>(character);
        character->fields_.character_view_state_ = state;
    }
    return result;
}

// IDA_EA: 0x8230054
// IDA_NAME: _ZNK5CUser18GetCharacViewStateEi
// DEMANGLED: CUser::GetCharacViewState(int)const
int CUser::GetCharacViewState(int slot) const {
    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return -1;
    }
    return characters_[static_cast<std::size_t>(slot)]
        .fields_.character_view_state_;
}

// IDA_EA: 0x8230860
// IDA_NAME: _ZNK5CUser18getCharacSlotLimitEv
// DEMANGLED: CUser::getCharacSlotLimit(void)const
int CUser::getCharacSlotLimit() const {
    return charac_slot_limit_;
}

// IDA_EA: 0x8230870
// IDA_NAME: _ZNK5CUser18getSlotEffectCountEv
// DEMANGLED: CUser::getSlotEffectCount(void)const
int CUser::getSlotEffectCount() const {
    return slot_effect_count_;
}

// IDA_EA: 0x8230880
// IDA_NAME: _ZN5CUser18setSlotEffectCountEh
// DEMANGLED: CUser::setSlotEffectCount(unsigned char)
CUser* CUser::setSlotEffectCount(unsigned char value) {
    slot_effect_count_ = value;
    return this;
}

// IDA_EA: 0x84ECB36
// IDA_NAME: _ZN5CUser28SetCharacViewStateByCharacNoEii
// DEMANGLED: CUser::SetCharacViewStateByCharacNo(int,int)
int CUser::SetCharacViewStateByCharacNo(int charac_no, int state) {
    for (std::vector<_Charac_info>::iterator character = characters_.begin();
         character != characters_.end(); ++character) {
        if (character->fields_.character_no_ == charac_no) {
            character_view_.enableSaveCharacView();
            character->fields_.character_view_state_ = state;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x863BECE
// IDA_NAME: _ZNK5CUser28get_charac_name_by_charac_noEi
// DEMANGLED: CUser::get_charac_name_by_charac_no(int)const
const char* CUser::get_charac_name_by_charac_no(int charac_no) const {
    for (std::vector<_Charac_info>::const_iterator character =
             characters_.begin();
         character != characters_.end(); ++character) {
        if (character->fields_.character_no_ == charac_no) {
            return character->fields_.character_name_;
        }
    }
    return 0;
}

// IDA_EA: 0x864A9AC
// IDA_NAME: _ZN5CUser13lock4DataLoadEv
// DEMANGLED: CUser::lock4DataLoad(void)
int CUser::lock4DataLoad() {
    if (static_cast<unsigned char>(isLocked4DataLoad())) {
        return 0;
    }
    charac_info_load_lock_ = 1;
    return 1;
}

// IDA_EA: 0x864A9DA
// IDA_NAME: _ZN5CUser15unlock4DataLoadEv
// DEMANGLED: CUser::unlock4DataLoad(void)
int CUser::unlock4DataLoad() {
    if (static_cast<unsigned char>(isLocked4DataLoad()) != 1u) {
        return 0;
    }
    charac_info_load_lock_ = 0;
    return 1;
}

// IDA_EA: 0x864AA0A
// IDA_NAME: _ZN5CUser17isLocked4DataLoadEv
// DEMANGLED: CUser::isLocked4DataLoad(void)
int CUser::isLocked4DataLoad() {
    return charac_info_load_lock_;
}

// IDA_EA: 0x864AA1A
// IDA_NAME: _ZN5CUser16lockDeleteCharacEv
// DEMANGLED: CUser::lockDeleteCharac(void)
int CUser::lockDeleteCharac() {
    if (static_cast<unsigned char>(isLockedDeleteCharac())) {
        return 0;
    }
    charac_info_delete_lock_ = 1;
    return 1;
}

// IDA_EA: 0x864AA48
// IDA_NAME: _ZN5CUser18unlockDeleteCharacEv
// DEMANGLED: CUser::unlockDeleteCharac(void)
int CUser::unlockDeleteCharac() {
    if (static_cast<unsigned char>(isLockedDeleteCharac()) != 1u) {
        return 0;
    }
    charac_info_delete_lock_ = 0;
    return 1;
}

// IDA_EA: 0x864AA78
// IDA_NAME: _ZN5CUser20isLockedDeleteCharacEv
// DEMANGLED: CUser::isLockedDeleteCharac(void)
int CUser::isLockedDeleteCharac() {
    return charac_info_delete_lock_;
}

// IDA_EA: 0x864AA88
// IDA_NAME: _ZN5CUser16lockCreateCharacEv
// DEMANGLED: CUser::lockCreateCharac(void)
int CUser::lockCreateCharac() {
    if (static_cast<unsigned char>(isLockedCreateCharac())) {
        return 0;
    }
    charac_info_create_lock_ = 1;
    return 1;
}

// IDA_EA: 0x864AAB6
// IDA_NAME: _ZN5CUser18unlockCreateCharacEv
// DEMANGLED: CUser::unlockCreateCharac(void)
int CUser::unlockCreateCharac() {
    if (static_cast<unsigned char>(isLockedCreateCharac()) != 1u) {
        return 0;
    }
    charac_info_create_lock_ = 0;
    return 1;
}

// IDA_EA: 0x864AAE6
// IDA_NAME: _ZN5CUser20isLockedCreateCharacEv
// DEMANGLED: CUser::isLockedCreateCharac(void)
int CUser::isLockedCreateCharac() {
    return charac_info_create_lock_;
}

// IDA_EA: 0x864A830
// IDA_NAME: _ZN5CUser21CheckDeleteCharacTimeEi
// DEMANGLED: CUser::CheckDeleteCharacTime(int)
int CUser::CheckDeleteCharacTime(int slot) {
    CommonTime current_time = {};
    current_time.SetCurTime();

    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return 0;
    }

    const CommonTime& create_time =
        characters_[static_cast<std::size_t>(slot)].fields_.create_time_;
    bool same_day = false;
    if (create_time.year == current_time.year) {
        if (create_time.month == current_time.month) {
            if (create_time.day == current_time.day) {
                same_day = true;
            }
        }
    }

    if (same_day) {
        const int created_minutes =
            60 * static_cast<signed char>(create_time.hour) +
            static_cast<signed char>(create_time.minute);
        const int current_minutes =
            60 * static_cast<signed char>(current_time.hour) +
            static_cast<signed char>(current_time.minute);
        if (current_minutes - created_minutes <= 1439) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x864AAF6
// IDA_NAME: _ZN5CUser17checkSelectCharacEi
// DEMANGLED: CUser::checkSelectCharac(int)
int CUser::checkSelectCharac(int slot) {
    if (this->getCurCharacR()) {
        cMyTrace trace("bool CUser::checkSelectCharac(int)", 3748, 5);
        trace("CUser::SelectCharac m_selected is not null");
        return 0;
    }

    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return 0;
    }
    const _Charac_info& character =
        characters_[static_cast<std::size_t>(slot)];

    if (G_GameWorld()->IsPVPChannel()) {
        const int level = character.fields_.level_;
        if (level < G_CDataManager()->GetMinLevelPvPChannel()) {
            return 0;
        }
        const int level_again = character.fields_.level_;
        if (level_again > G_CDataManager()->GetMaxLevelPvPChannel()) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x864DF12
// IDA_NAME: _ZNK5CUser14get_charac_idxEj
// DEMANGLED: CUser::get_charac_idx(unsigned int)const
int CUser::get_charac_idx(unsigned int charac_no) const {
    for (std::vector<_Charac_info>::size_type slot = 0;
         slot < characters_.size(); ++slot) {
        if (static_cast<unsigned int>(
                characters_[slot].fields_.character_no_) == charac_no) {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

// IDA_EA: 0x864DFA0
// IDA_NAME: _ZN5CUser13get_charac_noEi
// DEMANGLED: CUser::get_charac_no(int)
int CUser::get_charac_no(int slot) {
    if (slot == -1) {
        const _Charac_info* const current = getCurCharacR();
        if (current) {
            return current->fields_.character_no_;
        }
        return 0;
    }
    if (slot < 0 || characters_.size() <= static_cast<std::size_t>(slot)) {
        return 0;
    }
    return characters_[static_cast<std::size_t>(slot)].fields_.character_no_;
}

// IDA_EA: 0x864E024
// IDA_NAME: _ZNK5CUser17get_character_jobEj
// DEMANGLED: CUser::get_character_job(unsigned int)const
int CUser::get_character_job(unsigned int charac_no) const {
    for (std::vector<_Charac_info>::const_iterator character =
             characters_.begin();
         character != characters_.end(); ++character) {
        if (static_cast<unsigned int>(character->fields_.character_no_) ==
            charac_no) {
            return static_cast<unsigned char>(character->fields_.job_);
        }
    }
    return 11;
}
