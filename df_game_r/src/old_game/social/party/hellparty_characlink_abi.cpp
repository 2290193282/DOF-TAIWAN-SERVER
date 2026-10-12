#include "old_game/social/party/hellparty_characlink_abi.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <cstring>
#include <string>


// IDA_EA: 0x891B1D6
// IDA_NAME: _ZN27stHellPartyMonsterGroupListC2Ev
// DEMANGLED: stHellPartyMonsterGroupList::stHellPartyMonsterGroupList(void)
stHellPartyMonsterGroupList::stHellPartyMonsterGroupList() {
    std::memset(monster_index, -1, sizeof(monster_index));
    std::memset(monster_grade, -1, sizeof(monster_grade));
    difficulty_value = 0;
    reserved_51 = 0;
    difficulty_group_value = 0;
    group_order = 0;
    std::memset(reserved_55, 0, sizeof(reserved_55));
}

// IDA_EA: 0x891B242
// IDA_NAME: _ZN13stHellParty_tC2Ev
// DEMANGLED: stHellParty_t::stHellParty_t(void)
stHellParty_t::stHellParty_t()
    : max_base_value(0),
      min_base_value(0),
      base_value(0),
      base_rising_value(0),
      sub_base_rates(0.0f),
      pass_sub_base_rates(0.0f),
      level_difference_rates(),
      difficulty_rates(),
      party_rates(),
      ancient_dungeon_add_rate(0.0f),
      current_group_index(0),
      reserved_52(),
      monster_groups(),
      difficulty_value(),
      difficulty_group_value(),
      difficulty_extra_7e(),
      difficulty_extra_84(),
      difficulty_extra_8a(),
      guide_line_value(0) {
}

// IDA_EA: 0x891C7E2
// IDA_NAME: _ZN13stHellParty_tD2Ev
// DEMANGLED: stHellParty_t::~stHellParty_t()
stHellParty_t::~stHellParty_t() {
}

// IDA_EA: 0x89097C4
// IDA_NAME: _Z26Get_instance_stHellParty_tv
// DEMANGLED: Get_instance_stHellParty_t(void)
stHellParty_t* Get_instance_stHellParty_t() {
    static stHellParty_t hellPartyScript_;
    return &hellPartyScript_;
}

// IDA_EA: 0x8909529
// IDA_NAME: _Z28importCharacLinkSystemScriptPKcR20stCharacLinkSystem_t
// DEMANGLED: importCharacLinkSystemScript(char const*,stCharacLinkSystem_t &)
int importCharacLinkSystemScript(const char* filename,
                                 stCharacLinkSystem_t& script) {
    bool success = false;
    std::string tag;

    if (!loadRDARScriptFile("", filename)) {
        return 6571;
    }

    while (ScanType(tag, true) == 1) {
        if (tag == "[link level]") {
            // The 0627 PVF token names the character-link level gate.  The
            // only old-server consumer is Dispatcher_SelectStriker
            // (0x81E4066), which compares the selected striker character's
            // level against this +0x00 value.
            script.striker_skill_minimum_level = ScanInt(&success);
            if (!success) {
                return 6583;
            }
        } else if (tag == "[low link character exp bonus]" ||
                   tag == "[low link character gold bonus]" ||
                   tag == "[high link character exp bonus]" ||
                   tag == "[high link character gold bonus]") {
            std::vector<stCharacLinkSystemData>* target = 0;
            if (tag == "[low link character exp bonus]") {
                target = &script.low_link_character_exp_bonus_table;
            } else if (tag == "[low link character gold bonus]") {
                target = &script.low_link_character_gold_bonus_table;
            } else if (tag == "[high link character exp bonus]") {
                target = &script.high_link_character_exp_bonus_table;
            } else if (tag == "[high link character gold bonus]") {
                target = &script.high_link_character_gold_bonus_table;
            }

            while (target) {
                stCharacLinkSystemData data;
                data.minimum_character_level = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.maximum_character_level = ScanInt(&success);
                if (!success) {
                    return 6613;
                }
                data.bonus_rate_percent = ScanInt(&success);
                if (!success) {
                    return 6618;
                }
                data.bonus_value = ScanInt(&success);
                if (!success) {
                    return 6623;
                }
                target->push_back(data);
            }
        }
    }

    return 0;
}

// IDA_EA: 0x8909857
// IDA_NAME: _Z21importHellPartyScriptPKcR13stHellParty_t
// DEMANGLED: importHellPartyScript(char const*,stHellParty_t &)
int importHellPartyScript(const char* filename, stHellParty_t& script) {
    bool success = false;
    std::string top_tag;
    std::string tag;

    if (!loadRDARScriptFile("", filename)) {
        return 6692;
    }

    while (1) {
        if (ScanType(top_tag, true) != 1) {
            return 6702;
        }

        if (top_tag == "[hellparty basic values]") {
            while (1) {
                if (ScanType(tag, true) != 1) {
                    return 6711;
                }

                if (tag == "[max base value]") {
                    script.max_base_value = ScanInt(&success);
                    if (!script.max_base_value) {
                        return 6721;
                    }
                    if (!success) {
                        return 6723;
                    }
                } else if (tag == "[min base value]") {
                    script.min_base_value = ScanInt(&success);
                    if (!success) {
                        return 6732;
                    }
                } else if (tag == "[base value]") {
                    script.base_value = ScanInt(&success);
                    if (!success) {
                        return 6739;
                    }
                } else if (tag == "[sub base rates]") {
                    script.sub_base_rates = ScanFloat(&success);
                    if (!success) {
                        return 6746;
                    }
                } else if (tag == "[pass sub base rates]") {
                    script.pass_sub_base_rates = ScanFloat(&success);
                    if (!success) {
                        return 6753;
                    }
                } else if (tag == "[base rising value]") {
                    script.base_rising_value = ScanInt(&success);
                    if (!success) {
                        return 6760;
                    }
                } else if (tag == "[level difference rates]") {
                    script.level_difference_rates[1] = ScanFloat(&success);
                    script.level_difference_rates[2] = ScanFloat(&success);
                    script.level_difference_rates[3] = ScanFloat(&success);
                    if (!success) {
                        return 6770;
                    }
                } else if (tag == "[difficulty rates]") {
                    for (int index = 0; index < 5; ++index) {
                        script.difficulty_rates[index] = ScanFloat(&success);
                    }
                    if (!success) {
                        return 6792;
                    }
                } else if (tag == "[party rates]") {
                    script.party_rates[0] = ScanFloat(3, &success);
                    script.party_rates[1] = ScanFloat(&success);
                    script.party_rates[2] = ScanFloat(&success);
                    script.party_rates[3] = ScanFloat(&success);
                    if (!success) {
                        return 6803;
                    }
                } else if (tag == "[ancient dungeon add rate]") {
                    script.ancient_dungeon_add_rate = ScanFloat(&success);
                    if (!success) {
                        return 6811;
                    }
                } else if (tag == "[guide line value]") {
                    script.guide_line_value = ScanInt(&success);
                    if (!success) {
                        return 6819;
                    }
                } else if (tag == "[/hellparty basic values]") {
                    break;
                }
            }
        } else if (top_tag == "[hellparty monster group]") {
            while (1) {
                if (ScanType(tag, true) != 1) {
                    return 6834;
                }

                if (tag == "[group index]") {
                    script.current_group_index =
                        static_cast<uint16_t>(ScanInt(&success));
                    if (!success) {
                        return 6842;
                    }
                } else if (tag == "[group]") {
                    stHellPartyMonsterGroupList group;
                    std::string group_name;
                    if (ScanStr(&group_name) != 1) {
                        return 6857;
                    }

                    if (group_name == "A") {
                        group.difficulty_value = script.difficulty_value[1];
                        group.difficulty_group_value =
                            script.difficulty_group_value[1];
                        group.group_order = 1;
                    } else if (group_name == "B") {
                        group.difficulty_value = script.difficulty_value[2];
                        group.difficulty_group_value =
                            script.difficulty_group_value[2];
                        group.group_order = 2;
                    } else if (group_name == "C") {
                        group.difficulty_value = script.difficulty_value[3];
                        group.difficulty_group_value =
                            script.difficulty_group_value[3];
                        group.group_order = 3;
                    } else if (group_name == "D") {
                        group.difficulty_value = script.difficulty_value[4];
                        group.difficulty_group_value =
                            script.difficulty_group_value[4];
                        group.group_order = 4;
                    } else if (group_name == "E") {
                        group.difficulty_value = script.difficulty_value[5];
                        group.difficulty_group_value =
                            script.difficulty_group_value[5];
                        group.group_order = 5;
                    } else {
                        return 6893;
                    }

                    int index = 0;
                    while (index < 16) {
                        group.monster_index[index] = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        group.monster_grade[index] =
                            static_cast<int8_t>(ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        ++index;
                    }

                    if (script.monster_groups.find(
                            script.current_group_index) !=
                        script.monster_groups.end()) {
                        return 6920;
                    }
                    script.monster_groups.insert(
                        std::make_pair(script.current_group_index, group));
                } else if (tag == "[/hellparty monster group]") {
                    break;
                }
            }
        } else if (top_tag == "[difficulty]") {
            const int count = ScanInt(&success);
            if (count > 6) {
                return 6938;
            }
            if (!success) {
                return 6941;
            }

            for (int index = 1; count + 1 > index; ++index) {
                std::string difficulty_name;
                if (ScanStr(&difficulty_name) != 1) {
                    return 6949;
                }
                script.difficulty_value[index] =
                    static_cast<uint8_t>(ScanInt(&success));
                script.difficulty_group_value[index] =
                    static_cast<int16_t>(ScanInt(&success));
                script.difficulty_extra_7e[index] =
                    static_cast<uint8_t>(ScanInt(&success));
                script.difficulty_extra_84[index] =
                    static_cast<uint8_t>(ScanInt(&success));
                script.difficulty_extra_8a[index] =
                    static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    return 6960;
                }
            }
        } else if (top_tag == "[hellparty script end]") {
            return 0;
        }
    }
}
