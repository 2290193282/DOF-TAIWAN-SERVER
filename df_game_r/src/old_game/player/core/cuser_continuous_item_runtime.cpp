#include "old_game/player/core/cuser.h"

#include "core/config/env/environment.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/limit_item_usage_info_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/inventory/db_update_limit_item_usage.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <stdint.h>

namespace {

const uint32_t kContinuousItemMaxCount = 50u;
const uint32_t kContinuousItemMaxIndex = 0x7ffffffeu;
const uint32_t kContinuousItemMaxExpiry = 0x77359400u;

bool IsInvalidContinuousItem(const ContinuousItemInfo& item,
                             uint32_t current_sec) {
    const uint32_t item_index = static_cast<uint32_t>(item.item_index);
    const uint32_t expiry_tick = static_cast<uint32_t>(item.expire_tick);
    return expiry_tick < current_sec || item_index > kContinuousItemMaxIndex ||
           expiry_tick > kContinuousItemMaxExpiry;
}

}  // namespace

// IDA_EA: 0x8688FBA
// IDA_NAME: _ZN14LimitItemUsage12isAnotherDayEv
// DEMANGLED: LimitItemUsage::isAnotherDay(void)
int LimitItemUsage::isAnotherDay() {
    const CommonTime& last_usage_time = common_time;
    return CheckDailyScheduleTime(
        G_CEnvironment()->get_fatigue_time(),
        static_cast<long>(last_usage_time),
        static_cast<long>(GlobalData::s_systemTime_.getCurSec()));
}

// IDA_EA: 0x8695EF0
// IDA_NAME: _ZN14LimitItemUsageC2Ev
// DEMANGLED: LimitItemUsage::LimitItemUsage(void)
LimitItemUsage::LimitItemUsage() {
    common_time.SetCurTime();
    limit_type = 0;
    limit_count = 0;
    item_id = 0;
    used_count = 0;
}

// IDA_EA: 0x8695F28
// IDA_NAME: _ZN14LimitItemUsage5useItEv
// DEMANGLED: LimitItemUsage::useIt(void)
int LimitItemUsage::useIt() {
    if (isUsable() != 1)
        return 0;
    used_count = static_cast<int32_t>(
        static_cast<uint32_t>(used_count) + 1u);
    common_time.SetCurTime();
    return 1;
}

// IDA_EA: 0x8695F68
// IDA_NAME: _ZN14LimitItemUsage8isUsableEv
// DEMANGLED: LimitItemUsage::isUsable(void)
int LimitItemUsage::isUsable() {
    if (limit_type == 2u)
        return used_count < limit_count;
    if (limit_type > 1u)
        return 1;
    if (isAnotherDay())
        used_count = 0;
    return used_count < limit_count;
}

// IDA_EA: 0x86A6480
// IDA_NAME: _ZN14LimitItemUsageD2Ev
// DEMANGLED: LimitItemUsage::~LimitItemUsage()
LimitItemUsage::~LimitItemUsage() {
}

// IDA_EA: 0x8688AE6
// IDA_NAME: _ZN5CUser12UseLimitItemEib
// DEMANGLED: CUser::UseLimitItem(int,bool)
int CUser::UseLimitItem(int item_id, bool notify) {
    int result = 1;
    for (std::vector<LimitItemUsage>::iterator it =
             limit_item_usage_.begin();
         it != limit_item_usage_.end(); ++it) {
        if (it->item_id != item_id)
            continue;

        if (it->isUsable() != 1)
            result = 0;
        if (notify) {
            SendLimitItemUsageNotiInfo(item_id,
                                       it->limit_count,
                                       it->used_count);
        }
        if (!it->useIt())
            return 0;

        const CommonTime& usage_time = it->common_time;
        DB_UpdateLimitItemUsage::makeRequest(
            get_charac_no(-1),
            static_cast<int>(static_cast<int8_t>(it->limit_type)),
            item_id,
            it->used_count,
            static_cast<long>(usage_time.current_time));
        return 1;
    }
    return result;
}

// IDA_EA: 0x8688C70
// IDA_NAME: _ZN5CUser17IsLimitItemUsableEi
// DEMANGLED: CUser::IsLimitItemUsable(int)
int CUser::IsLimitItemUsable(int item_id) {
    for (std::vector<LimitItemUsage>::iterator it =
             limit_item_usage_.begin();
         it != limit_item_usage_.end(); ++it) {
        if (it->item_id == item_id)
            return it->isUsable() != 0;
    }
    return 1;
}

// IDA_EA: 0x8677F24
// IDA_NAME: _ZNK15CUserCharacInfo21GetContinuousItemInfoEP12SIG_LOAD_ETC
// DEMANGLED: CUserCharacInfo::GetContinuousItemInfo(SIG_LOAD_ETC *)const
bool CUserCharacInfo::GetContinuousItemInfo(SIG_LOAD_ETC* payload) const {
    payload->cooltime_items.charac_no = payload->charac_no;
    payload->effect_items.charac_no = payload->charac_no;

    const std::vector<ContinuousItemInfo>* const cooltime_items =
        GetCurCharacCoolTimeItemListR();
    payload->cooltime_items.item_info.count =
        static_cast<uint8_t>(cooltime_items->size());
    for (size_t index = 0; cooltime_items->size() > index; ++index) {
        const ContinuousItemInfo& source = (*cooltime_items)[index];
        CONTINUOUS_ITEM_INFO::Entry& target =
            payload->cooltime_items.item_info.entries[index];
        target.item_id = static_cast<uint32_t>(source.item_index);
        target.remain_time = static_cast<uint32_t>(source.expire_tick);
    }

    const std::vector<ContinuousItemInfo>* const effect_items =
        GetCurCharacEffectItemListR();
    payload->effect_items.item_info.count =
        static_cast<uint8_t>(effect_items->size());
    for (size_t index = 0; effect_items->size() > index; ++index) {
        const ContinuousItemInfo& source = (*effect_items)[index];
        CONTINUOUS_ITEM_INFO::Entry& target =
            payload->effect_items.item_info.entries[index];
        target.item_id = static_cast<uint32_t>(source.item_index);
        target.remain_time = static_cast<uint32_t>(source.expire_tick);
    }

    return false;
}

// IDA_EA: 0x8658B2A
// IDA_NAME: _ZN5CUser22UpdateCoolTimeItemInfoEj
// DEMANGLED: CUser::UpdateCoolTimeItemInfo(unsigned int)
void CUser::UpdateCoolTimeItemInfo(unsigned int current_sec) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("user.cpp", 11448), true);
    guard << 306;
    guard << GetUID();

    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO* const saved =
        guard.GetInBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>();
    saved->charac_no = static_cast<uint32_t>(getCurCharacNo());

    CONTINUOUS_ITEM_INFO serialized;
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemList();
    if (items->size() <= kContinuousItemMaxCount) {
        uint32_t saved_count = 0;
        for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
             it != items->end();) {
            if (IsInvalidContinuousItem(*it, current_sec)) {
                it = items->erase(it);
                continue;
            }

            serialized.entries[saved_count].item_id =
                static_cast<uint32_t>(it->item_index);
            serialized.entries[saved_count].remain_time =
                static_cast<uint32_t>(it->expire_tick);
            ++saved_count;
            ++it;
        }
        serialized.count = static_cast<uint8_t>(saved_count);
        saved->item_info = serialized;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x8658D90
// IDA_NAME: _ZN5CUser20UpdateEffectItemInfoEj
// DEMANGLED: CUser::UpdateEffectItemInfo(unsigned int)
void CUser::UpdateEffectItemInfo(unsigned int current_sec) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("user.cpp", 11499), true);
    guard << 311;
    guard << GetUID();

    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO* const saved =
        guard.GetInBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>();
    saved->charac_no = static_cast<uint32_t>(getCurCharacNo());

    CONTINUOUS_ITEM_INFO serialized;
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    if (items->size() <= kContinuousItemMaxCount) {
        uint32_t saved_count = 0;
        for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
             it != items->end();) {
            if (IsInvalidContinuousItem(*it, current_sec)) {
                it = items->erase(it);
                continue;
            }

            serialized.entries[saved_count].item_id =
                static_cast<uint32_t>(it->item_index);
            serialized.entries[saved_count].remain_time =
                static_cast<uint32_t>(it->expire_tick);
            ++saved_count;
            ++it;
        }
        serialized.count = static_cast<uint8_t>(saved_count);
        saved->item_info = serialized;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x8659B60
// IDA_NAME: _ZN5CUser33_processSpeicalContinuousTimeItemEj
// DEMANGLED: CUser::_processSpeicalContinuousTimeItem(unsigned int)
char CUser::_processSpeicalContinuousTimeItem(unsigned int item_index) {
    CItem* const item =
        G_CDataManager()->find_item(static_cast<int>(item_index));
    if (!item || !item->is_stackable()) {
        return 0;
    }

    CStackableItem* const stackable_item =
        static_cast<CStackableItem*>(item);
    if (!stackable_item->IsExpAffect()) {
        return 0;
    }

    const float exp_ratio =
        static_cast<float>(stackable_item->GetExpAffectRatio());
    if (exp_ratio <= 0.0f) {
        return 0;
    }
    return static_cast<char>(SetExpAffectItemEffect(true, exp_ratio));
}

// IDA_EA: 0x8659CF0
// IDA_NAME: _ZN15CUserCharacInfo19EnablePeriodExpItemEv
// DEMANGLED: CUserCharacInfo::EnablePeriodExpItem(void)
int CUserCharacInfo::EnablePeriodExpItem() {
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    const unsigned int current_second = static_cast<unsigned int>(
        GlobalData::s_systemTime_.getCurSec());

    for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
         it != items->end(); ++it) {
        CItem* const item = G_CDataManager()->find_item(it->item_index);
        // This early return is intentional: it is the original behavior when
        // an effect-list record refers to a missing PVF prototype.
        if (!item) {
            return 0;
        }
        if (!item->is_stackable() ||
            !static_cast<CStackableItem*>(item)->IsExpAffect()) {
            continue;
        }

        if (current_second < static_cast<unsigned int>(it->expire_tick)) {
            return 1;
        }

        items->erase(it);
        SetExpAffectItemEffect(false, 0.0f);
        return 0;
    }
    return 0;
}

// IDA_EA: 0x865E994
// IDA_NAME: _ZNK5CUser17CheckCoolTimeItemEm
// DEMANGLED: CUser::CheckCoolTimeItem(unsigned long)const
int CUser::CheckCoolTimeItem(unsigned long item_index) const {
    if (!getCurCharacR()) {
        return 0;
    }

    const std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemListR();
    for (std::vector<ContinuousItemInfo>::const_iterator it = items->begin();
         it != items->end(); ++it) {
        if (static_cast<unsigned long>(
                static_cast<uint32_t>(it->item_index)) != item_index) {
            continue;
        }
        return GlobalData::s_systemTime_.getCurSec() <
                       static_cast<uint32_t>(it->expire_tick)
                   ? 1
                   : 0;
    }
    return 0;
}

// IDA_EA: 0x865EA62
// IDA_NAME: _ZN5CUser22_processContinuousItemEjP14CStackableItem
// DEMANGLED: CUser::_processContinuousItem(unsigned int,CStackableItem *)
char CUser::_processContinuousItem(unsigned int item_index,
                                   CStackableItem* item) {
    const uint32_t current_sec = GlobalData::s_systemTime_.getCurSec();
    if (item) {
        if (item->GetCoolTimeOption().enabled) {
            _processCoolTimeItem(item, item_index, current_sec);
        }
        if (item->GetEffectItemOption().enabled) {
            _processEffectItem(item, item_index, current_sec);
        }
    }
    return _processSpeicalContinuousTimeItem(item_index);
}

// IDA_EA: 0x865EB0C
// IDA_NAME: _ZN5CUser20_processCoolTimeItemEP14CStackableItemjj
// DEMANGLED: CUser::_processCoolTimeItem(CStackableItem *,unsigned int,unsigned int)
void CUser::_processCoolTimeItem(CStackableItem* item,
                                 unsigned int item_index,
                                 unsigned int current_sec) {
    const uint32_t expire_tick =
        current_sec +
        static_cast<uint32_t>(item->continuous_item_state_.cool_time_ms) /
            1000u;
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemList();

    bool found = false;
    for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
         it != items->end(); ++it) {
        if (static_cast<uint32_t>(it->item_index) != item_index) {
            continue;
        }
        if (expire_tick > static_cast<uint32_t>(it->expire_tick)) {
            it->expire_tick = static_cast<int>(expire_tick);
        }
        found = true;
    }
    if (!found) {
        ContinuousItemInfo continuous_item;
        continuous_item.item_index = static_cast<int>(item_index);
        continuous_item.expire_tick = static_cast<int>(expire_tick);
        items->push_back(continuous_item);
    }
    UpdateCoolTimeItemInfo(current_sec);
}

// IDA_EA: 0x865EC0E
// IDA_NAME: _ZN5CUser18_processEffectItemEP14CStackableItemjj
// DEMANGLED: CUser::_processEffectItem(CStackableItem *,unsigned int,unsigned int)
void CUser::_processEffectItem(CStackableItem* item,
                               unsigned int item_index,
                               unsigned int current_sec) {
    const CStackableStatChangeInfo& configured_stat_change =
        item->continuous_item_state_.stat_change;
    if (configured_stat_change.duration_tag != 1u) {
        cMyTrace trace(
            "void CUser::_processEffectItem(CStackableItem*, unsigned int, unsigned int)",
            14622,
            0);
        trace("ERROR : Mismatch tag [stat change duration] and [effect maintenance]. There is no duration value.");
        return;
    }

    const uint32_t duration_sec =
        static_cast<uint32_t>(configured_stat_change.duration_ms) / 1000u;
    const uint32_t expire_tick = current_sec + duration_sec;
    const CStackableStatChangeInfo stat_change = item->GetStatChangeInfo();
    if (stat_change.timer_enabled) {
        G_TimerQueue()->InsertTimer(0,
                                    GetUID(),
                                    118,
                                    static_cast<int>(duration_sec),
                                    item_index,
                                    getCurCharacNo());
    }

    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    bool found = false;
    for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
         it != items->end(); ++it) {
        if (static_cast<uint32_t>(it->item_index) != item_index) {
            continue;
        }
        if (expire_tick > static_cast<uint32_t>(it->expire_tick)) {
            it->expire_tick = static_cast<int>(expire_tick);
        }
        found = true;
    }
    if (!found) {
        ContinuousItemInfo continuous_item;
        continuous_item.item_index = static_cast<int>(item_index);
        continuous_item.expire_tick = static_cast<int>(expire_tick);
        items->push_back(continuous_item);
    }
    UpdateEffectItemInfo(current_sec);
}

// IDA_EA: 0x8659E3E
// IDA_NAME: _ZN15CUserCharacInfo24ClearUselessCooltimeItemEj
// DEMANGLED: CUserCharacInfo::ClearUselessCooltimeItem(unsigned int)
int CUserCharacInfo::ClearUselessCooltimeItem(unsigned int current_sec) {
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemList();
    std::vector<ContinuousItemInfo>::iterator it = items->begin();
    while (it != items->end()) {
        if (static_cast<uint32_t>(it->expire_tick) >= current_sec) {
            ++it;
        } else {
            it = items->erase(it);
        }
    }
    // The old iterator comparison result is returned after the loop.  It is
    // always zero once the end iterator has been reached.
    return 0;
}

// IDA_EA: 0x8659EF4
// IDA_NAME: _ZN15CUserCharacInfo22ClearUselessEffectItemEj
// DEMANGLED: CUserCharacInfo::ClearUselessEffectItem(unsigned int)
int CUserCharacInfo::ClearUselessEffectItem(unsigned int current_sec) {
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    std::vector<ContinuousItemInfo>::iterator it = items->begin();
    while (it != items->end()) {
        if (static_cast<uint32_t>(it->expire_tick) >= current_sec) {
            ++it;
        } else {
            it = items->erase(it);
        }
    }
    // See ClearUselessCooltimeItem: the original loop exits on a false
    // iterator comparison, so its observable return value is zero.
    return 0;
}

// IDA_EA: 0x8658FF6
// IDA_NAME: _ZN5CUser20LoadCoolTimeItemInfoEP20CONTINUOUS_ITEM_INFO
// DEMANGLED: CUser::LoadCoolTimeItemInfo(CONTINUOUS_ITEM_INFO *)
int CUser::LoadCoolTimeItemInfo(CONTINUOUS_ITEM_INFO* item_info) {
    const uint32_t current_sec = GlobalData::s_systemTime_.getCurSec();
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemList();
    items->clear();

    // 0x8658FF6 compares the packed byte as the old signed `char`, rather
    // than treating it as an unconstrained unsigned count.
    const int saved_count = static_cast<int>(
        static_cast<int8_t>(item_info->count));
    for (int index = 0; saved_count > index; ++index) {
        const CONTINUOUS_ITEM_INFO::Entry& source = item_info->entries[index];
        if (source.remain_time <= current_sec ||
            source.item_id > kContinuousItemMaxIndex ||
            source.remain_time > kContinuousItemMaxExpiry) {
            continue;
        }

        ContinuousItemInfo item;
        item.item_index = static_cast<int>(source.item_id);
        item.expire_tick = static_cast<int>(source.remain_time);
        items->push_back(item);
    }

    ClearUselessCooltimeItem(current_sec);
    return SendCoolTimeItemInfo(current_sec) == 1;
}

// IDA_EA: 0x86590F0
// IDA_NAME: _ZN5CUser18LoadEffectItemInfoEP20CONTINUOUS_ITEM_INFO
// DEMANGLED: CUser::LoadEffectItemInfo(CONTINUOUS_ITEM_INFO *)
int CUser::LoadEffectItemInfo(CONTINUOUS_ITEM_INFO* item_info) {
    const uint32_t current_sec = GlobalData::s_systemTime_.getCurSec();
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    items->clear();

    // Preserve the same signed-byte termination as the cool-time loader.
    const int saved_count = static_cast<int>(
        static_cast<int8_t>(item_info->count));
    for (int index = 0; saved_count > index; ++index) {
        const CONTINUOUS_ITEM_INFO::Entry& source = item_info->entries[index];
        if (source.remain_time <= current_sec ||
            source.item_id > kContinuousItemMaxIndex ||
            source.remain_time > kContinuousItemMaxExpiry) {
            continue;
        }

        ContinuousItemInfo item;
        item.item_index = static_cast<int>(source.item_id);
        item.expire_tick = static_cast<int>(source.remain_time);
        _processSpeicalContinuousTimeItem(source.item_id);
        items->push_back(item);
    }

    ClearUselessEffectItem(current_sec);
    return SendEffectItemInfo(current_sec) == 1;
}

// IDA_EA: 0x86591FE
// IDA_NAME: _ZN5CUser20SendCoolTimeItemInfoEj
// DEMANGLED: CUser::SendCoolTimeItemInfo(unsigned int)
int CUser::SendCoolTimeItemInfo(unsigned int current_sec) {
    PacketGuard packet;
    packet.put_header(0, 172);

    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacCoolTimeItemList();
    packet.put_byte(static_cast<unsigned char>(items->size()));
    // Unlike the effect-item sender, the old cool-time path keeps iterating
    // after the byte-sized count has wrapped.  Preserve that distinct wire
    // behavior rather than normalising the two senders.
    for (std::vector<ContinuousItemInfo>::const_iterator it = items->begin();
         it != items->end(); ++it) {
        packet.put_int(it->item_index);
        packet.put_int(static_cast<int>(
            static_cast<uint32_t>(it->expire_tick) - current_sec));
    }
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x8659366
// IDA_NAME: _ZN5CUser18SendEffectItemInfoEj
// DEMANGLED: CUser::SendEffectItemInfo(unsigned int)
int CUser::SendEffectItemInfo(unsigned int current_sec) {
    PacketGuard packet;
    packet.put_header(0, 174);

    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    const uint8_t count = static_cast<uint8_t>(items->size());
    packet.put_byte(count);
    // The original indexed loop deliberately uses the narrowed byte count.
    for (uint32_t index = 0; index < count; ++index) {
        const ContinuousItemInfo& item = (*items)[index];
        packet.put_int(item.item_index);
        packet.put_int(static_cast<int>(
            static_cast<uint32_t>(item.expire_tick) - current_sec));
    }
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x86888F8
// IDA_NAME: _ZN5CUser12SetLimitInfoEP19SIG_LIMIT_ITEM_INFO
// DEMANGLED: CUser::SetLimitInfo(SIG_LIMIT_ITEM_INFO *)
void CUser::SetLimitInfo(SIG_LIMIT_ITEM_INFO* info) {
    if (!info) {
        return;
    }

    limit_item_usage_.clear();
    const stLimitItemUsageInfo* const configured =
        G_CDataManager()->GetLimitItemUsageInfo();
    for (std::vector<stLimitItemUsageInfo::LimitInfo>::const_iterator script =
             configured->values.begin();
         script != configured->values.end(); ++script) {
        bool loaded = false;
        for (uint32_t index = 0; index < info->count; ++index) {
            const LimitItemUsagePayload& source = info->entries[index];
            if (static_cast<int32_t>(source.item_id) != script->item_id) {
                continue;
            }

            LimitItemUsage usage;
            usage.common_time.SetCreateTime(
                static_cast<long>(source.last_usage_time));
            usage.limit_type = script->flag;
            usage.limit_count = script->usage_count;
            usage.item_id = script->item_id;
            usage.used_count = static_cast<int32_t>(source.used_count);
            limit_item_usage_.push_back(usage);
            loaded = true;
            break;
        }

        if (!loaded) {
            LimitItemUsage usage;
            usage.common_time.SetCurTime();
            usage.limit_type = script->flag;
            usage.limit_count = script->usage_count;
            usage.item_id = script->item_id;
            usage.used_count = 0;
            limit_item_usage_.push_back(usage);
        }
    }
}
