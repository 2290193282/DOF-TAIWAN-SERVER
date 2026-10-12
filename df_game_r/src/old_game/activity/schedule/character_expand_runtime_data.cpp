#include "old_game/activity/schedule/character_expand_runtime_data.h"

#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"

#include <cstring>


typedef char advancealtar_CharacData_Size_Must_Be_0x28[
    (sizeof(advancealtar::CharacData) == 0x28) ? 1 : -1];

namespace advancealtar {

// IDA_EA: 0x812D024
// IDA_NAME: _ZN12advancealtar10CharacDataC2Ev
// DEMANGLED: advancealtar::CharacData::CharacData(void)
CharacData::CharacData()
    : updated(0),
      user(0),
      gage_level(0),
      gage(0),
      slot_items() {}

// IDA_EA: 0x812D062
// IDA_NAME: _ZN12advancealtar10CharacDataD2Ev
// DEMANGLED: advancealtar::CharacData::~CharacData()
CharacData::~CharacData() {}

// IDA_EA: 0x812D078
// IDA_NAME: _ZN12advancealtar10CharacData5resetEv
// DEMANGLED: advancealtar::CharacData::reset(void)
int CharacData::reset() {
    gage_level = 0;
    user = 0;
    gage = 0;
    updated = 0;
    slot_items.clear();
    return 0;
}

// IDA_EA: 0x812D0B6
// IDA_NAME: _ZN12advancealtar10CharacData6upGageEv
// DEMANGLED: advancealtar::CharacData::upGage(void)
int CharacData::upGage() {
    CDataManager* data_manager = G_CDataManager();
    if (!data_manager || gage_level < 0 || gage_level >= 4) {
        sendTestCheckGage();
        return 0;
    }

    const ConfigParameter* config =
        data_manager->get_advance_altar_config_parameter();
    const int multiplier = config->gage_up_rates[0];
    if (multiplier <= 0) {
        sendTestCheckGage();
        return 0;
    }

    const int max_gage = config->gage_upgrade_costs[gage_level] * multiplier;
    if (gage < max_gage) {
        const long double rate_ratio =
            static_cast<long double>(config->gage_up_rates[gage_level]) /
            static_cast<long double>(multiplier);
        gage += static_cast<int>(
            static_cast<long double>(config->gage_up_rates[0]) * rate_ratio);
    } else {
        gage = max_gage;
    }
    sendTestCheckGage();
    return 0;
}

// IDA_EA: 0x812D176
// IDA_NAME: _ZN12advancealtar10CharacData11upgreadGageEv
// DEMANGLED: advancealtar::CharacData::upgreadGage(void)
int CharacData::upgreadGage() {
    if (gage_level == 3) {
        return 2;
    }
    CDataManager* data_manager = G_CDataManager();
    if (!data_manager || gage_level < 0 || gage_level >= 4) {
        return 3;
    }

    const ConfigParameter* config =
        data_manager->get_advance_altar_config_parameter();
    const int multiplier = config->gage_up_rates[0];
    if (multiplier <= 0) {
        return 3;
    }

    const int upgrade_cost =
        static_cast<int>(
            static_cast<long double>(config->gage_upgrade_costs[gage_level]) *
            config->gage_upgrade_cost_percent) *
        multiplier;
    if (gage < upgrade_cost) {
        return 3;
    }

    gage -= upgrade_cost;
    switch (gage_level) {
        case 0:
            gage_level = 1;
            break;
        case 1:
            gage_level = 2;
            break;
        case 2:
            gage_level = 3;
            break;
    }
    return 0;
}

// IDA_EA: 0x812D246
// IDA_NAME: _ZN12advancealtar10CharacData11setSlotItemEv
// DEMANGLED: advancealtar::CharacData::setSlotItem(void)
int CharacData::setSlotItem() {
    if (!user) {
        return 0;
    }
    return user->advance_altar_manager_.getSlotItem(slot_items);
}

// IDA_EA: 0x812D278
// IDA_NAME: _ZN12advancealtar10CharacData14summonSlotUnitEhi
// DEMANGLED: advancealtar::CharacData::summonSlotUnit(unsigned char,int)
int CharacData::summonSlotUnit(uint8_t slot, int item_id) {
    std::map<uint8_t, _SlotItem>::const_iterator it = slot_items.find(slot);
    if (it == slot_items.end()) {
        return 4;
    }
    if (it->second.item_id != item_id) {
        return 2;
    }

    CDataManager* data_manager = G_CDataManager();
    if (!data_manager) {
        return 2;
    }
    const ConfigParameter* config =
        data_manager->get_advance_altar_config_parameter();
    const int multiplier = config->gage_up_rates[0];
    if (multiplier <= 0) {
        return 2;
    }

    const int summon_cost = it->second.required_gage * multiplier;
    if (gage < summon_cost) {
        return 2;
    }

    gage -= summon_cost;
    return 0;
}

// IDA_EA: 0x812D372
// IDA_NAME: _ZN12advancealtar10CharacData17sendTestCheckGageEv
// DEMANGLED: advancealtar::CharacData::sendTestCheckGage(void)
void CharacData::sendTestCheckGage() {}

// IDA_EA: 0x8134806
// IDA_NAME: _ZN12advancealtar10CharacData7setUserEP5CUser
// DEMANGLED: advancealtar::CharacData::setUser(CUser *)
CharacData* CharacData::setUser(CUser* new_user) {
    user = new_user;
    return this;
}

// IDA_EA: 0x8134814
// IDA_NAME: _ZNK12advancealtar10CharacData7getUserEv
// DEMANGLED: advancealtar::CharacData::getUser(void)const
CUser* CharacData::getUser() const {
    return user;
}

// IDA_EA: 0x8142088
// IDA_NAME: _ZNK12advancealtar10CharacData7getGageEv
// DEMANGLED: advancealtar::CharacData::getGage(void)const
int CharacData::getGage() const {
    return gage;
}

// IDA_EA: 0x8142094
// IDA_NAME: _ZNK12advancealtar10CharacData12getGageLevelEv
// DEMANGLED: advancealtar::CharacData::getGageLevel(void)const
int CharacData::getGageLevel() const {
    return gage_level;
}

}  // namespace advancealtar
