#include "old_game/appearance/pet/creature.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/character/tag_charac_info_payload.h"

#include <climits>
#include <cstring>
#include <map>

namespace WongWork {
namespace {

typedef std::map<int, Avatar_Item*> AvatarItemMap;
typedef std::map<int, int> AvatarEnduranceMap;

}  // namespace

// IDA_EA: 0x82F94BC
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr27RegistNotAssignedAvatarItemEiiPKcbRK20stAvatarEmblemInfo_t
// DEMANGLED: WongWork::CAvatarItemMgr::RegistNotAssignedAvatarItem(int,int,char const*,bool,stAvatarEmblemInfo_t const&)
int CAvatarItemMgr::RegistNotAssignedAvatarItem(
    int usable_period,
    int regist_time,
    const char* agency_no,
    bool period_is_seconds,
    const stAvatarEmblemInfo_t& emblem) {
    const int key = _AssignTempKey();
    Avatar_Item* const item = m_AvatarItemPool.Acquire();
    if (usable_period && usable_period != 365) {
        item->SetData(regist_time +
                          (period_is_seconds ? usable_period
                                             : 86400 * usable_period),
                      regist_time, agency_no);
    } else {
        item->SetData(0, regist_time, agency_no);
    }
    item->setEmblemSocketData(emblem);
    item->setColor(0, 0);
    wait_assign_items_.insert(std::make_pair(key, item));
    return key;
}

// IDA_EA: 0x82F98C2
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr14GetIPGAgencyNoEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetIPGAgencyNo(int)const
const char* CAvatarItemMgr::GetIPGAgencyNo(int key) const {
    Avatar_Item* const item = _FindItemAll(key);
    return item ? item->GetIPGAgencyNo() : 0;
}

// IDA_EA: 0x82F9E02
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr14_AssignTempKeyEv
// DEMANGLED: WongWork::CAvatarItemMgr::_AssignTempKey(void)
int CAvatarItemMgr::_AssignTempKey() {
    if (next_temp_key_ == INT_MAX) next_temp_key_ = 0;
    return static_cast<int>(_MakeTempKey(++next_temp_key_));
}

// IDA_EA: 0x82F9402
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr12UnRegistItemEi
// DEMANGLED: WongWork::CAvatarItemMgr::UnRegistItem(int)
int CAvatarItemMgr::UnRegistItem(int key) {
    Avatar_Item* const item = _FindItem(key);
    if (!item) return 0;

    assigned_items_.erase(key);
    m_AvatarItemPool.Free(item);

    AvatarEnduranceMap::iterator rate = emblem_reduce_rates_.find(key);
    if (rate != emblem_reduce_rates_.end()) emblem_reduce_rates_.erase(rate);
    return 1;
}

// IDA_EA: 0x82F9F9E
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr19GetEmblemReduceRateEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetEmblemReduceRate(int)
int CAvatarItemMgr::GetEmblemReduceRate(int key) {
    AvatarEnduranceMap::const_iterator rate = emblem_reduce_rates_.find(key);
    return rate == emblem_reduce_rates_.end() ? 0 : rate->second;
}

// IDA_EA: 0x82F9E4C
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr18AddEmblemEnduranceEiPKNS_11Avatar_ItemE
// DEMANGLED: WongWork::CAvatarItemMgr::AddEmblemEndurance(int,WongWork::Avatar_Item const*)
int CAvatarItemMgr::AddEmblemEndurance(int key, const Avatar_Item* item) {
    int reduction_rate = 0;
    const stAvatarEmblemInfo_t* const emblem = item->getJewelSocketData();

    // The five physical socket records are six bytes each; the item index is
    // the named dword at +0x02, exactly as in 0x82F9E4C.
    for (int socket = 0; socket <= 4; ++socket) {
        const uint32_t item_index = emblem->sockets[socket].emblem_item_index;
        if (item_index == 0u) {
            continue;
        }

        CItem* const emblem_item = G_CDataManager()->find_item(
            static_cast<int>(item_index));
        if (!emblem_item) {
            return 0;
        }
        reduction_rate += ARAD::Singleton<AvatarRechargeServer>::Get()
                              ->GetRateFromEmblemGrade(
                                  emblem_item->get_grade());
    }

    AvatarEnduranceMap::iterator found = emblem_reduce_rates_.find(key);
    if (found == emblem_reduce_rates_.end()) {
        emblem_reduce_rates_.insert(std::make_pair(key, reduction_rate));
    } else {
        found->second = reduction_rate;
    }
    return 1;
}

// IDA_EA: 0x82F91CE
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr19setEmblemSocketDataEiRK20stAvatarEmblemInfo_t
// DEMANGLED: WongWork::CAvatarItemMgr::setEmblemSocketData(int,stAvatarEmblemInfo_t const&)
int CAvatarItemMgr::setEmblemSocketData(int key,
                                        const stAvatarEmblemInfo_t& emblem) {
    Avatar_Item* const item = _FindItem(key);
    if (!item) {
        return 0;
    }
    item->setEmblemSocketData(emblem);
    AddEmblemEndurance(key, item);
    return 1;
}

// IDA_EA: 0x82F9228
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr14isSocketAvatarEi
// DEMANGLED: WongWork::CAvatarItemMgr::isSocketAvatar(int)
int CAvatarItemMgr::isSocketAvatar(int key) {
    return getJewelSocketData(key)->sockets[0].socket_type != 0u;
}

// IDA_EA: 0x82F99D6
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr13AddAvatarItemEiPNS_11Avatar_ItemE
// DEMANGLED: WongWork::CAvatarItemMgr::AddAvatarItem(int,WongWork::Avatar_Item *)
int CAvatarItemMgr::AddAvatarItem(int key, Avatar_Item* item) {
    assigned_items_.insert(std::make_pair(key, item));
    return AddEmblemEndurance(key, item);
}

// IDA_EA: 0x82FA008
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr15UpdateEnduranceEii
// DEMANGLED: WongWork::CAvatarItemMgr::UpdateEndurance(int,int)
void CAvatarItemMgr::UpdateEndurance(int key, int endurance_value) {
    AvatarEnduranceMap::iterator found = pending_endurance_.find(key);
    if (found == pending_endurance_.end()) {
        pending_endurance_.insert(std::make_pair(key, endurance_value));
        return;
    }
    found->second = endurance_value;
}

// IDA_EA: 0x82FA0A8
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr13SaveEnduranceEv
// DEMANGLED: WongWork::CAvatarItemMgr::SaveEndurance(void)
int CAvatarItemMgr::SaveEndurance() {
    const int was_empty = pending_endurance_.empty();
    if (!was_empty) {
        for (AvatarEnduranceMap::const_iterator it = pending_endurance_.begin();
             it != pending_endurance_.end(); ++it) {
            AvatarRechargeServer::SaveAvatarEnduranceUnit(it->first,
                                                          it->second);
        }
        pending_endurance_.clear();
        return 0;
    }
    return was_empty;
}

// IDA_EA: 0x82FA3B2
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr12_MakeTempKeyEi
// DEMANGLED: WongWork::CAvatarItemMgr::_MakeTempKey(int)const
unsigned int CAvatarItemMgr::_MakeTempKey(int key) const {
    return static_cast<unsigned int>(key) | 0x80000000u;
}

// IDA_EA: 0x82F925C
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr7GetItemEP16AvatarDetailInfoPi
// DEMANGLED: WongWork::CAvatarItemMgr::GetItem(AvatarDetailInfo *,int *)const
int CAvatarItemMgr::GetItem(AvatarDetailInfo* details, int* count) const {
    int index = 0;
    for (AvatarItemMap::const_iterator it = assigned_items_.begin();
         it != assigned_items_.end() &&
         index < oldsrv::config::kAvatarDetailItemCount;
         ++it, ++index) {
        AvatarDetailInfo& detail = details[index];
        Avatar_Item* const item = it->second;

        detail.ui_id = static_cast<uint32_t>(it->first);
        detail.expire_date = static_cast<uint32_t>(item->GetExpireDate());
        detail.registration_date =
            static_cast<uint32_t>(item->GetRegistTime());
        std::strncpy(detail.ipg_agency_no, item->GetIPGAgencyNo(),
                     sizeof(detail.ipg_agency_no));
        detail.jewel_socket = *item->getJewelSocketData();

        const stAvatarExpansionInfo_t* const expansion =
            item->getExnansionInfoRef();
        detail.color1 = expansion->avatar_expansion[0];
        detail.color2 = expansion->avatar_expansion[1];
    }
    *count = index;
    return 1;
}

}  // namespace WongWork
