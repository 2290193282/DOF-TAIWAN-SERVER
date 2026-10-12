#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_data/PVF/common/rdar_script.h"

#include <stdint.h>

#include <string>

// IDA_EA: 0x8968EC2
// IDA_NAME: _ZN37ST_Event_Upgrade_Jar_EventNPCs_ScriptC2Ev
// DEMANGLED: ST_Event_Upgrade_Jar_EventNPCs_Script::ST_Event_Upgrade_Jar_EventNPCs_Script(void)
ST_Event_Upgrade_Jar_EventNPCs_Script::
    ST_Event_Upgrade_Jar_EventNPCs_Script()
    : fail_probability(0), material_count(0) {}

// IDA_EA: 0x8969E90
// IDA_NAME: _ZN37ST_Event_Upgrade_Jar_EventNPCs_ScriptD2Ev
// DEMANGLED: ST_Event_Upgrade_Jar_EventNPCs_Script::~ST_Event_Upgrade_Jar_EventNPCs_Script()
ST_Event_Upgrade_Jar_EventNPCs_Script::
    ~ST_Event_Upgrade_Jar_EventNPCs_Script() {}

// IDA_EA: 0x80F50A4
// IDA_NAME: _ZN19EventUpgradeJarInfoC2Ev
// DEMANGLED: EventUpgradeJarInfo::EventUpgradeJarInfo(void)
EventUpgradeJarInfo::EventUpgradeJarInfo()
    : group_to_booster(),
      booster_pair_to_index(),
      fortune_indexes(),
      fortune_fail_rates(),
      event_npc_scripts() {}

// IDA_EA: 0x80F51D2
// IDA_NAME: _ZN19EventUpgradeJarInfoD2Ev
// DEMANGLED: EventUpgradeJarInfo::~EventUpgradeJarInfo()
EventUpgradeJarInfo::~EventUpgradeJarInfo() {}

// IDA_EA: 0x80F49B8
// IDA_NAME: _ZN17Event_Upgrade_JarC2Ev
// DEMANGLED: Event_Upgrade_Jar::Event_Upgrade_Jar(void)
Event_Upgrade_Jar::Event_Upgrade_Jar() : EventUpgradeJarInfo() {}

// IDA_EA: 0x80F49CC
// IDA_NAME: _ZN17Event_Upgrade_JarD2Ev
// DEMANGLED: Event_Upgrade_Jar::~Event_Upgrade_Jar()
Event_Upgrade_Jar::~Event_Upgrade_Jar() {}

// IDA_EA: 0x80F49E0
// IDA_NAME: _ZN17Event_Upgrade_Jar10LoadScriptEPKcS1_
// DEMANGLED: Event_Upgrade_Jar::LoadScript(char const*,char const*)
bool Event_Upgrade_Jar::LoadScript(const char* dir, const char* filename) {
    (void)dir;
    return importEventUpgradeJarScript(this, filename) != 0;
}

// IDA_EA: 0x80F4A0A
// IDA_NAME: _ZN17Event_Upgrade_Jar34getBoosterIndex_byUpgradeCardIndexERmS0_
// DEMANGLED: Event_Upgrade_Jar::getBoosterIndex_byUpgradeCardIndex(unsigned long &,unsigned long &)
int Event_Upgrade_Jar::getBoosterIndex_byUpgradeCardIndex(
    unsigned long& first_card_index,
    unsigned long& second_card_index) {
    std::map<unsigned long, short>::iterator first_it =
        group_to_booster.find(first_card_index);
    std::map<unsigned long, short>::iterator second_it =
        group_to_booster.find(second_card_index);
    if (first_it == group_to_booster.end()) {
        return 0;
    }
    if (second_it == group_to_booster.end()) {
        return 0;
    }

    short first_booster = first_it->second;
    short second_booster = second_it->second;
    if (first_booster > second_booster) {
        first_booster ^= second_booster;
        second_booster ^= first_booster;
        first_booster ^= second_booster;
    }

    std::pair<int, int> key(first_booster, second_booster);
    std::map<std::pair<int, int>, unsigned long>::iterator index_it =
        booster_pair_to_index.find(key);
    if (index_it != booster_pair_to_index.end()) {
        return index_it->second;
    }
    return 0;
}

// IDA_EA: 0x80F4BB0
// IDA_NAME: _ZNK17Event_Upgrade_Jar20check_requiredRecipeERK10Inven_Item
// DEMANGLED: Event_Upgrade_Jar::check_requiredRecipe(Inven_Item const&)const
bool Event_Upgrade_Jar::check_requiredRecipe(const Inven_Item& item) const {
    return item.getKey() == 2670205 && item.get_add_info() > 0;
}

// IDA_EA: 0x80F4BE0
// IDA_NAME: _ZNK17Event_Upgrade_Jar20_getFailProb_FortuneEij
// DEMANGLED: Event_Upgrade_Jar::_getFailProb_Fortune(int,unsigned int)const
int Event_Upgrade_Jar::_getFailProb_Fortune(
    int item_index,
    unsigned int upgrade_index) const {
    for (unsigned int index = 0; index < fortune_indexes.size(); ++index) {
        if (item_index == static_cast<int>(fortune_indexes[index])) {
            if (fortune_fail_rates[index].size() > upgrade_index) {
                return fortune_fail_rates[index][upgrade_index];
            }
            return 100000;
        }
    }
    return 100000;
}

// IDA_EA: 0x80F4C8E
// IDA_NAME: _ZNK17Event_Upgrade_Jar19_getFailProb_NewNPCEj
// DEMANGLED: Event_Upgrade_Jar::_getFailProb_NewNPC(unsigned int)const
int Event_Upgrade_Jar::_getFailProb_NewNPC(unsigned int script_index) const {
    if (event_npc_scripts.size() > script_index) {
        return event_npc_scripts[script_index].fail_probability;
    }
    return 100000;
}

// IDA_EA: 0x80F4CCC
// IDA_NAME: _ZN17Event_Upgrade_Jar24_getMeterialCount_NewNPCEj
// DEMANGLED: Event_Upgrade_Jar::_getMeterialCount_NewNPC(unsigned int)
int Event_Upgrade_Jar::_getMeterialCount_NewNPC(unsigned int script_index) {
    if (event_npc_scripts.size() > script_index) {
        return event_npc_scripts[script_index].material_count;
    }
    return 1000000;
}

// IDA_EA: 0x80F4D0C
// IDA_NAME: _ZNK17Event_Upgrade_Jar23_getDiscountRate_NewNPCEv
// DEMANGLED: Event_Upgrade_Jar::_getDiscountRate_NewNPC(void)const
long double Event_Upgrade_Jar::_getDiscountRate_NewNPC() const {
    return new_npc_discount_rate;
}

// IDA_EA: 0x80F4D20
// IDA_NAME: _ZN17Event_Upgrade_Jar18CheckMaterialIndexEm
// DEMANGLED: Event_Upgrade_Jar::CheckMaterialIndex(unsigned long)
char Event_Upgrade_Jar::CheckMaterialIndex(unsigned long item_index) {
    union {
        Event_Upgrade_Jar* pointer;
        uintptr_t integer;
    } self_value;
    self_value.pointer = this;
    event_mode = 0;
    event_upgrade_index = 0;
    if (item_index == 2670203) {
        event_mode = 2;
        return static_cast<char>(self_value.integer);
    }

    for (unsigned int index = 0; index < fortune_indexes.size(); ++index) {
        if (fortune_indexes[index] == item_index) {
            event_mode = 1;
            return static_cast<char>(self_value.integer);
        }
    }
    return 0;
}

// IDA_EA: 0x80F4DAC
// IDA_NAME: _ZN17Event_Upgrade_Jar22SettingRecipeNUpConuntEiR14upgrade_info_th
// DEMANGLED: Event_Upgrade_Jar::SettingRecipeNUpConunt(int,upgrade_info_t &,unsigned char)
Event_Upgrade_Jar* Event_Upgrade_Jar::SettingRecipeNUpConunt(
    int item_index,
    upgrade_info_t& upgrade_info,
    uint8_t upgrade_index) {
    if (event_mode == 0) {
        return 0;
    }

    if (event_mode == 1) {
        upgrade_info.calculated_cost = 0;
        upgrade_info.event_material_item_count = 1;
        for (unsigned int index = 0; index < fortune_indexes.size(); ++index) {
            if (item_index == static_cast<int>(fortune_indexes[index])) {
                upgrade_info.event_material_item_index =
                    static_cast<int>(fortune_indexes[index]);
                break;
            }
        }
    } else if (event_mode == 2) {
        // IDA 0x80F4E79 first narrows the integer cost to float before the
        // long-double discount multiplication. Retain that rounding point.
        const float calculated_cost =
            static_cast<float>(upgrade_info.calculated_cost);
        upgrade_info.calculated_cost = static_cast<int>(
            _getDiscountRate_NewNPC() * calculated_cost);
        upgrade_info.event_material_item_count =
            _getMeterialCount_NewNPC(upgrade_index);
        upgrade_info.event_material_item_index = 2670203;
    }

    event_upgrade_index = upgrade_index;
    return this;
}

// IDA_EA: 0x80F4EE2
// IDA_NAME: _ZNK17Event_Upgrade_Jar14GetProbabilityEiRj
// DEMANGLED: Event_Upgrade_Jar::GetProbability(int,unsigned int &)const
int Event_Upgrade_Jar::GetProbability(
    int item_index,
    unsigned int& probability) const {
    int mode = event_mode;
    if (mode != 0) {
        if (event_mode == 1) {
            mode = _getFailProb_Fortune(item_index, event_upgrade_index);
            probability = mode;
        } else if (event_mode == 2) {
            mode = _getFailProb_NewNPC(event_upgrade_index);
            probability = mode;
        }
    }
    return mode;
}

// IDA_EA: 0x80F4F62
// IDA_NAME: _ZN17Event_Upgrade_Jar18CheckMaxLuckyLevelER10Inven_Itemm
// DEMANGLED: Event_Upgrade_Jar::CheckMaxLuckyLevel(Inven_Item &,unsigned long)
bool Event_Upgrade_Jar::CheckMaxLuckyLevel(
    Inven_Item& item,
    unsigned long item_index) {
    return fortune_indexes.size() <= 1u || fortune_indexes[1] != item_index ||
           static_cast<unsigned int>(
               static_cast<unsigned char>(item.GetUpgrade())) <
               static_cast<unsigned int>(upgrade_max);
}

// IDA_EA: 0x80F4FD0
// IDA_NAME: _ZN17Event_Upgrade_Jar30GetIndexLuckyStrengtheningDeedEv
// DEMANGLED: Event_Upgrade_Jar::GetIndexLuckyStrengtheningDeed(void)
int Event_Upgrade_Jar::GetIndexLuckyStrengtheningDeed() {
    if (fortune_indexes.size() <= 1u) {
        return 0;
    }
    return fortune_indexes[1];
}

// IDA_EA: 0x854B57C
// IDA_NAME: _ZNK17Event_Upgrade_Jar9isFortuneEv
// DEMANGLED: Event_Upgrade_Jar::isFortune(void)const
bool Event_Upgrade_Jar::isFortune() const {
    return event_mode == 1;
}

// IDA_EA: 0x854B590
// IDA_NAME: _ZNK17Event_Upgrade_Jar8isNewNPCEv
// DEMANGLED: Event_Upgrade_Jar::isNewNPC(void)const
bool Event_Upgrade_Jar::isNewNPC() const {
    return event_mode == 2;
}

// IDA_EA: 0x89687D1
// IDA_NAME: _Z27importEventUpgradeJarScriptP19EventUpgradeJarInfoPKc
// DEMANGLED: importEventUpgradeJarScript(EventUpgradeJarInfo *,char const*)
int importEventUpgradeJarScript(EventUpgradeJarInfo* info,
                                const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    ST_Event_Upgrade_Jar_EventNPCs_Script event_npc_script;
    while (ScanType(tag, true) == 1) {
        if (tag == "[group info]") {
            info->group_count =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }

            for (uint16_t group = 0; group < info->group_count; ++group) {
                while (1) {
                    const int item_index = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    if (item_index == -1) {
                        break;
                    }
                    info->group_to_booster.insert(std::make_pair(
                        static_cast<unsigned long>(item_index),
                        static_cast<short>(group)));
                }
            }

            const int pair_count =
                (static_cast<int>(info->group_count) + 1) *
                static_cast<int>(info->group_count) / 2;
            for (int pair_index = 0; pair_index < pair_count; ++pair_index) {
                const int first_group = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int second_group = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int booster_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                info->booster_pair_to_index[std::make_pair(first_group - 1,
                                                           second_group - 1)] =
                    static_cast<unsigned long>(booster_index);
            }
        } else if (tag == "[fortune index]") {
            for (int index = 0; index <= 3; ++index) {
                const int fortune_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                info->fortune_indexes.push_back(
                    static_cast<unsigned long>(fortune_index));
            }
        } else if (tag == "[fortune fail rate]") {
            while (1) {
                const int fortune_group = ScanInt(&success);
                if (!success) {
                    break;
                }
                ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int fail_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                // IDA 0x8968C0A addresses the row as this + 52 +
                // 12 * fortune_group. The four vector objects begin at
                // +64, so Script.pvf's one-based group numbers map to the
                // zero-based C++ array here.
                info->fortune_fail_rates[fortune_group - 1].push_back(
                    static_cast<unsigned int>(fail_rate));
            }
        } else if (tag == "[event upgrade table]") {
            info->new_npc_discount_rate = ScanFloat(&success);
            if (!success) {
                return 0;
            }
            while (1) {
                // IDA 0x8968C5B consumes this column but immediately
                // overwrites its success flag with the required probability
                // read; it intentionally does not branch here.
                ScanInt(&success);
                event_npc_script.fail_probability =
                    ScanInt(&success);
                if (!success) {
                    break;
                }
                event_npc_script.material_count =
                    ScanInt(&success);
                if (!success) {
                    return 0;
                }
                info->event_npc_scripts.push_back(event_npc_script);
            }
        } else if (tag == "[upgrade max]") {
            info->upgrade_max = ScanInt(&success);
            if (!success) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8968D59
// IDA_NAME: _Z43updateUpgradeTableFromEventUpgradeJarScriptP15upgrade_table_tP19EventUpgradeJarInfo
// DEMANGLED: updateUpgradeTableFromEventUpgradeJarScript(upgrade_table_t *,EventUpgradeJarInfo *)
int updateUpgradeTableFromEventUpgradeJarScript(
    upgrade_table_t* table,
    EventUpgradeJarInfo* info) {
    if (table == 0 || info == 0) {
        return 0;
    }

    for (unsigned int index = 0; index < info->event_npc_scripts.size();
         ++index) {
        if (index < table->upgrade_infos.size()) {
            upgrade_info_t& upgrade_info = table->upgrade_infos[index];
            upgrade_info.event_material_item_index = 2670203;
            upgrade_info.event_material_item_count =
                info->event_npc_scripts[index].material_count;
        }
    }

    for (unsigned int index = 0; index <= 99; ++index) {
        table->upgrade_costs[index] = static_cast<int>(
            static_cast<long double>(table->upgrade_costs[index]) *
            static_cast<long double>(info->new_npc_discount_rate));
    }
    return 1;
}
