#include "old_game/appearance/pet/creature.h"

#include "old_data/db/character/tag_charac_info_payload.h"

namespace WongWork {

// IDA_EA: 0x82F8FB0
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr10RegistItemERK16AvatarDetailInfob
// DEMANGLED: WongWork::CAvatarItemMgr::RegistItem(AvatarDetailInfo const&,bool)
int CAvatarItemMgr::RegistItem(const AvatarDetailInfo& detail,
                               bool period_in_days) {
    stAvatarExpansionInfo_t expansion;
    expansion.avatar_expansion[0] = detail.color1;
    expansion.avatar_expansion[1] = detail.color2;
    return RegistItem(static_cast<int>(detail.ui_id),
                      static_cast<int>(detail.expire_date),
                      static_cast<int>(detail.registration_date),
                      detail.ipg_agency_no,
                      detail.jewel_socket,
                      expansion,
                      period_in_days);
}

// IDA_EA: 0x82F901C
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr10RegistItemEiiiPKcRK20stAvatarEmblemInfo_tRK23stAvatarExpansionInfo_tb
// DEMANGLED: WongWork::CAvatarItemMgr::RegistItem(int,int,int,char const*,stAvatarEmblemInfo_t const&,stAvatarExpansionInfo_t const&,bool)
int CAvatarItemMgr::RegistItem(int key,
                               int expire_or_period,
                               int regist_time,
                               const char* agency_no,
                               const stAvatarEmblemInfo_t& emblem,
                               const stAvatarExpansionInfo_t& expansion,
                               bool period_in_days) {
    if (_FindItem(key)) {
        return 0;
    }

    Avatar_Item* const item = m_AvatarItemPool.Acquire();
    if (!item) {
        return 0;
    }

    int expire_time = 0;
    if (period_in_days) {
        if (expire_or_period != 0 && expire_or_period != 365) {
            expire_time = regist_time + 86400 * expire_or_period;
        }
    } else {
        expire_time = expire_or_period;
    }

    item->SetData(expire_time, regist_time, agency_no);
    item->setEmblemSocketData(emblem);
    item->setExpansionInfo(expansion);
    assigned_items_.insert(std::make_pair(key, item));
    AddEmblemEndurance(key, item);
    return 1;
}

// IDA_EA: 0x82F9772
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr19AssignAvatarItemKeyEii
// DEMANGLED: WongWork::CAvatarItemMgr::AssignAvatarItemKey(int,int)
int CAvatarItemMgr::AssignAvatarItemKey(int temporary_key, int avatar_ui_id) {
    Avatar_Item* const item = _FindWaitAssignItem(temporary_key);
    if (!item) {
        return 0;
    }

    wait_assign_items_.erase(temporary_key);
    assigned_items_.insert(std::make_pair(avatar_ui_id, item));
    AddEmblemEndurance(avatar_ui_id, item);
    return 1;
}

}  // namespace WongWork
