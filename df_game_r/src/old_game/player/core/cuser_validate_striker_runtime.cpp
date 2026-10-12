#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/PVF/etc/striker_script.h"

// IDA_EA: 0x8690BE6
// IDA_NAME: _ZN5CUser19ValidateStrikerInfoEv
// DEMANGLED: CUser::ValidateStrikerInfo(void)
bool CUser::ValidateStrikerInfo() {
    for (std::vector<_Charac_info>::size_type index = 0;
         index < characters_.size();
         ++index) {
        _Charac_info& tag_character = characters_[index];
        if (!tag_character.fields_.tag_character_no_)
            continue;

        const int source_index = get_charac_idx(
            tag_character.fields_.tag_character_no_);
        if (source_index == -1) {
            tag_character.fields_.tag_character_no_ = 0;
            tag_character.fields_.striker_skill_index_ = 0;
            continue;
        }

        _Charac_info& source_character =
            characters_[static_cast<std::vector<_Charac_info>::size_type>(
                source_index)];

        // The original is `movzx; shl eax, 4; sar al, 4`: preserve its
        // signed low-nibble extraction rather than treating grow type as a
        // generic byte.
        const int16_t grow_type = static_cast<int8_t>(
            static_cast<uint8_t>(source_character.fields_.grow_type_.growth_type) << 4u) >>
            4;
        const int16_t job = static_cast<int8_t>(source_character.fields_.job_);
        const int skill_index =
            static_cast<int>(tag_character.fields_.striker_skill_index_);

        if (G_CDataManager()->get_striker_script()->checkSkillIndex(
                job, grow_type, skill_index) != 1) {
            tag_character.fields_.tag_character_no_ = 0;
            tag_character.fields_.striker_skill_index_ = 0;
        }
    }
    return true;
}

// IDA_EA: 0x867CEAE
// IDA_NAME: _ZN5CUser25SetNeedCheckOverEquipItemEv
// DEMANGLED: CUser::SetNeedCheckOverEquipItem(void)
bool CUser::SetNeedCheckOverEquipItem() {
    for (std::vector<_Charac_info>::size_type index = 0;
         index < characters_.size();
         ++index) {
        characters_[index].fields_.need_check_over_equip_item_ = 1;
    }
    // The old loop returns its final `size() > index` predicate, so it is
    // false after both an empty and a non-empty character list.
    return false;
}

// IDA_EA: 0x8691DCA
// IDA_NAME: _ZN5CUser32setCharacLevelUpGift_AccountOnceER29SIG_LEVELUP_GIFT_ACCOUNT_ONCE
// DEMANGLED: CUser::setCharacLevelUpGift_AccountOnce(SIG_LEVELUP_GIFT_ACCOUNT_ONCE &)
char CUser::setCharacLevelUpGift_AccountOnce(
    SIG_LEVELUP_GIFT_ACCOUNT_ONCE& event_info) {
    for (int index = 0; index <= 10; ++index) {
        const SIG_LEVELUP_GIFT_ACCOUNT_ONCE::Entry& entry =
            event_info.entries[index];
        if (!entry.level)
            return 0;
        levelup_gifts_.gifts_.insert(
            std::make_pair(entry.job_type, entry.level));
    }
    return 0;
}
