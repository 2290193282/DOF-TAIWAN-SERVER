#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "common/manager/system_time.h"

#include <cstring>

namespace {

char kEmptyOnlinePreliminaryTeamName[] = "";

}  // namespace

// IDA_EA: 0x822F21C
// IDA_NAME: _ZN15CUserCharacInfo21set_charac_createtimeEl
// DEMANGLED: CUserCharacInfo::set_charac_createtime(long)
CommonTime* CUserCharacInfo::set_charac_createtime(long timer) {
    return current_charac_->fields_.create_time_.SetCreateTime(timer);
}

// IDA_EA: 0x86456D0
// IDA_NAME: _ZN15CUserCharacInfo24GetWarAreaKillCountTotalEv
// DEMANGLED: CUserCharacInfo::GetWarAreaKillCountTotal(void)
float CUserCharacInfo::GetWarAreaKillCountTotal() {
    if (!current_charac_) {
        cMyTrace trace(
            "float CUserCharacInfo::GetWarAreaKillCountTotal()", 559, 5);
        trace("WARAREA CUserCharacInfo::GetWarAreaKillCountTotal() : "
              "m_selected is NULL");
        return 0.0f;
    }

    float count = 0.0f;
    for (int area = 0; area <= 3; ++area) {
        count += current_charac_->fields_.finish_point_detail_[area];
    }
    return count;
}

// IDA_EA: 0x8645866
// IDA_NAME: _ZN15CUserCharacInfo19GetWarAreaKillCountEi
// DEMANGLED: CUserCharacInfo::GetWarAreaKillCount(int)
float CUserCharacInfo::GetWarAreaKillCount(int area) {
    if (!current_charac_) {
        cMyTrace trace("float CUserCharacInfo::GetWarAreaKillCount(int)",
                       608,
                       5);
        trace("WARAREA CUserCharacInfo::GetWarAreaKillCount() : "
              "m_selected is NULL");
        return 0.0f;
    }

    return current_charac_->fields_.finish_point_detail_[area];
}

// IDA_EA: 0x8645E20
// IDA_NAME: _ZNK15CUserCharacInfo37GetCurCharacOnlinePreliminaryTeamNameEv
// DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamName(void)const
char* CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamName() const {
    online_preliminary::COnlinePreliminaryTeam* const team =
        const_cast<CUserCharacInfo*>(this)->
            GetCurCharacOnlinePreliminaryTeam();
    return team ? team->GetTeamName() : kEmptyOnlinePreliminaryTeamName;
}

// IDA_EA: 0x8645E5E
// IDA_NAME: _ZN15CUserCharacInfo21GainCurCharacChaosExpEi
// DEMANGLED: CUserCharacInfo::GainCurCharacChaosExp(int)
int CUserCharacInfo::GainCurCharacChaosExp(int experience) {
    // The original i386 `int` ABI deliberately propagates the selected
    // character address on the null and post-clamp paths.  This preserves
    // that EAX result without reopening the selected-character layout.
    int result = static_cast<int>(
        reinterpret_cast<uintptr_t>(current_charac_));
    if (!current_charac_) {
        return result;
    }

    _Charac_info::Fields& fields = current_charac_->fields_;
    result = fields.chaos_point_;
    if (result <= 0) {
        return result;
    }

    fields.chaos_experience_ += experience;
    const int level_exp_percent =
        G_CDataManager()->get_level_exp_percent(get_charac_level(), 1);
    result = fields.chaos_experience_;
    if (result > level_exp_percent && level_exp_percent > 0) {
        enableSaveCharacStat();
        fields.chaos_point_ -= fields.chaos_experience_ / level_exp_percent;
        fields.chaos_experience_ %= level_exp_percent;
        result = fields.chaos_point_;
        if (result < 0) {
            fields.chaos_point_ = 0;
            result = static_cast<int>(
                reinterpret_cast<uintptr_t>(current_charac_));
            fields.chaos_experience_ = 0;
        }
    }
    return result;
}

// IDA_EA: 0x864607E
// IDA_NAME: _ZN15CUserCharacInfo21getSeparatedTextUnitsERSt6vectorISsSaISsEEPKcS5_i
// DEMANGLED: CUserCharacInfo::getSeparatedTextUnits(std::vector<std::string,std::allocator<std::string>> &,char const*,char const*,int)
int CUserCharacInfo::getSeparatedTextUnits(std::vector<std::string>& units,
                                            const char* source,
                                            const char* delimiter,
                                            int maximum) {
    units.clear();

    std::size_t offset = 6u;
    while (maximum == 0 ||
           units.size() < static_cast<std::size_t>(maximum - 1)) {
        const char* const delimiter_position =
            std::strstr(source + offset, delimiter);
        if (!delimiter_position) {
            break;
        }

        const std::size_t unit_size = static_cast<std::size_t>(
            delimiter_position - (source + offset));
        if (unit_size != 0u) {
            units.push_back(std::string(source + offset, unit_size));
        }
        offset += unit_size + std::strlen(delimiter);
    }

    if (source[offset] != '\0') {
        units.push_back(std::string(source + offset));
    }
    return 0;
}

// IDA_EA: 0x8659BEC
// IDA_NAME: _ZN15CUserCharacInfo21EnablePeriodExpDoubleEv
// DEMANGLED: CUserCharacInfo::EnablePeriodExpDouble(void)
int CUserCharacInfo::EnablePeriodExpDouble() {
    std::vector<ContinuousItemInfo>* const items =
        GetCurCharacEffectItemList();
    const unsigned int current_second = static_cast<unsigned int>(
        GlobalData::s_systemTime_.getCurSec());

    for (std::vector<ContinuousItemInfo>::iterator it = items->begin();
         it != items->end(); ++it) {
        if (it->item_index != 7377) {
            continue;
        }

        if (current_second < static_cast<unsigned int>(it->expire_tick)) {
            return 1;
        }

        items->erase(it);
        SetIsAffectedExpDouble(false);
        return 0;
    }
    return 0;
}

// IDA_EA: 0x8678084
// IDA_NAME: _ZN15CUserCharacInfo11isExistItemEi
// DEMANGLED: CUserCharacInfo::isExistItem(int)
bool CUserCharacInfo::isExistItem(int item_id) {
    if (!current_charac_) {
        return false;
    }

    if (getCurCharacCargoR()->check_item_exist(item_id) == -1) {
        return getCurCharacInvenR()->check_item_exist(item_id) != -1;
    }
    return true;
}
