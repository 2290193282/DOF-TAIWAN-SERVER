#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <cstring>

// IDA_EA: 0x81B4198
// IDA_NAME: _ZN20UserMercenaryInfoMgr16CheckValidAppendERK21CHARAC_LOAD_MERCENARY
// DEMANGLED: UserMercenaryInfoMgr::CheckValidAppend(CHARAC_LOAD_MERCENARY const&)
int UserMercenaryInfoMgr::CheckValidAppend(
    const CHARAC_LOAD_MERCENARY& data) {
    for (std::vector<CHARAC_LOAD_MERCENARY>::size_type index = 0;
         index < mercenaries_.size();
         ++index) {
        CHARAC_LOAD_MERCENARY& existing = mercenaries_[index];
        if (existing.charac_no != data.charac_no) {
            continue;
        }
        if (existing.level != data.level) {
            existing.level = data.level;
            existing.alter();
        }
        return 0;
    }
    return 1;
}

// IDA_EA: 0x81B4300
// IDA_NAME: _ZN20UserMercenaryInfoMgr9AddCharacER21CHARAC_LOAD_MERCENARY
// DEMANGLED: UserMercenaryInfoMgr::AddCharac(CHARAC_LOAD_MERCENARY&)
int UserMercenaryInfoMgr::AddCharac(CHARAC_LOAD_MERCENARY& data) {
    if (CheckValidAppend(data) != 1) {
        return 0;
    }

    // The old body initializes the aggregate grade to 10, then takes the
    // minimum grade across all eight avatar equipment records.  is_avatar_item
    // is CItem's IDA-verified vtable slot +0x10; it retains the original
    // CEquipItem dynamic-type test without manufacturing a replacement item
    // hierarchy here.
    data.avatar_bonus_index = 10;
    for (int slot = 0; slot <= 7; ++slot) {
        CItem* const item = G_CDataManager()->find_item(data.avatar_item_id[slot]);
        if (!item || !item->vtable_ || !item->vtable_->is_avatar_item(item)) {
            data.avatar_bonus_index = 0;
            break;
        }

        const int grade = item->get_grade();
        if (grade < static_cast<int>(data.avatar_bonus_index)) {
            data.avatar_bonus_index = static_cast<unsigned char>(grade);
        }
    }

    mercenaries_.push_back(data);
    return 1;
}

// IDA_EA: 0x81B4262
// IDA_NAME: _ZNK20UserMercenaryInfoMgr16CheckCharacIndexEh
// DEMANGLED: UserMercenaryInfoMgr::CheckCharacIndex(unsigned char) const
bool UserMercenaryInfoMgr::CheckCharacIndex(unsigned char index) const {
    return !mercenaries_.empty() &&
           static_cast<std::vector<CHARAC_LOAD_MERCENARY>::size_type>(index) <
               mercenaries_.size();
}

// IDA_EA: 0x81B42AE
// IDA_NAME: _ZNK20UserMercenaryInfoMgr19FindIndexByCharacNoEj
// DEMANGLED: UserMercenaryInfoMgr::FindIndexByCharacNo(unsigned int) const
int UserMercenaryInfoMgr::FindIndexByCharacNo(unsigned int charac_no) const {
    for (std::vector<CHARAC_LOAD_MERCENARY>::size_type index = 0;
         index < mercenaries_.size(); ++index) {
        if (mercenaries_[index].charac_no == charac_no) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x81B45CE
// IDA_NAME: _ZN20UserMercenaryInfoMgr20IsWaitingStateCharacEj
// DEMANGLED: UserMercenaryInfoMgr::IsWaitingStateCharac(unsigned int)
bool UserMercenaryInfoMgr::IsWaitingStateCharac(unsigned int charac_no) {
    const int index = FindIndexByCharacNo(charac_no);
    if (!CheckCharacIndex(static_cast<unsigned char>(index))) {
        return true;
    }

    const CHARAC_LOAD_MERCENARY& mercenary =
        mercenaries_[static_cast<unsigned char>(index)];
    const int now = GlobalData::s_systemTime_.getCurSec();

    // CHARAC_LOAD_MERCENARY::GetCompetitionState @0x81B2E86:
    // active period => 1; a cleared area byte => 0; otherwise => 2.
    const int competition_state =
        static_cast<int>(mercenary.end_time) - now > 0
            ? 1
            : (mercenary.competition_area == 0xffu ? 0 : 2);
    return competition_state == 0;
}

// IDA_EA: 0x81B462E
// IDA_NAME: _ZN20UserMercenaryInfoMgr16UpdateCharacNameEjRKSs
// DEMANGLED: UserMercenaryInfoMgr::UpdateCharacName(unsigned int,std::string const&)
char* UserMercenaryInfoMgr::UpdateCharacName(
    unsigned int charac_no,
    const std::string& charac_name) {
    const int index = FindIndexByCharacNo(charac_no);
    if (!CheckCharacIndex(static_cast<unsigned char>(index))) {
        return 0;
    }

    CHARAC_LOAD_MERCENARY& entry =
        mercenaries_[static_cast<unsigned char>(index)];
    std::memset(entry.name, 0, sizeof(entry.name));
    return std::strncpy(entry.name, charac_name.c_str(), 0x1d);
}

// IDA_EA: 0x81B44C0
// IDA_NAME: _ZN20UserMercenaryInfoMgr12ReturnCharacEP5CUserjR25competition_reward_result
// DEMANGLED: UserMercenaryInfoMgr::ReturnCharac(
//             CUser *,unsigned int,competition_reward_result &)
int UserMercenaryInfoMgr::ReturnCharac(
    CUser* user,
    unsigned int charac_no,
    competition_reward_result& result) {
    if (!user) {
        return 0;
    }

    const int index = FindIndexByCharacNo(charac_no);
    if (!CheckCharacIndex(static_cast<unsigned char>(index))) {
        return 0;
    }

    return mercenaries_[static_cast<unsigned char>(index)].Reward(user,
                                                                  result);
}

// IDA_EA: 0x81B4536
// IDA_NAME: _ZN20UserMercenaryInfoMgr17CompetitionCharacEP5CUserjcc
// DEMANGLED: UserMercenaryInfoMgr::CompetitionCharac(
//             CUser *,unsigned int,char,char)
bool UserMercenaryInfoMgr::CompetitionCharac(CUser* user,
                                              unsigned int charac_no,
                                              char competition_type,
                                              char competition_count) {
    const int index = FindIndexByCharacNo(charac_no);
    if (!CheckCharacIndex(static_cast<unsigned char>(index))) {
        return false;
    }

    return mercenaries_[static_cast<unsigned char>(index)].Competition(
               user, competition_type, competition_count) == 1;
}
