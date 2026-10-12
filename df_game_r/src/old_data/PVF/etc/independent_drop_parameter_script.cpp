#include "old_data/PVF/etc/independent_drop_parameter_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <stdio.h>
#include <string.h>

#include <string>

char g_independentDropScriptBaseDirectory[256] = {};
STScriptFileList g_independentDropScriptFileList;

typedef char IndependentDropListInfo_Size_Must_Be_0x10[
    (sizeof(IndependentDropListInfo) == 0x10) ? 1 : -1];
typedef char stMonsterIndependentDrop_t_Size_Must_Be_0x48[
    (sizeof(stMonsterIndependentDrop_t) == 0x48) ? 1 : -1];
typedef char stIndependentDropInfo_Size_Must_Be_0x50[
    (sizeof(stIndependentDropInfo) == 0x50) ? 1 : -1];
typedef char stIndependentDropRateControl_Size_Must_Be_0x54[
    (sizeof(stIndependentDropRateControl) == 0x54) ? 1 : -1];
typedef char IndependentDropParameterScript_Size_Must_Be_0x3C[
    (sizeof(IndependentDropParameterScript) == 0x3C) ? 1 : -1];

// IDA_EA: 0x83DD4B4
// IDA_NAME: _ZN23IndependentDropListInfoD2Ev
// DEMANGLED: IndependentDropListInfo::~IndependentDropListInfo()
IndependentDropListInfo::~IndependentDropListInfo() {}

// IDA_EA: 0x897CBAE
// IDA_NAME: _ZN23IndependentDropListInfoC2Ev
// DEMANGLED: IndependentDropListInfo::IndependentDropListInfo(void)
IndependentDropListInfo::IndependentDropListInfo() {}

// IDA_EA: 0x897CE1E
// IDA_NAME: _ZN23IndependentDropListInfoC2ERKS_
// DEMANGLED: IndependentDropListInfo::IndependentDropListInfo(IndependentDropListInfo const&)
IndependentDropListInfo::IndependentDropListInfo(
    const IndependentDropListInfo& other)
    : file_index(other.file_index), drops(other.drops) {}

// IDA_EA: 0x897CECC
// IDA_NAME: _ZN23IndependentDropListInfoaSERKS_
// DEMANGLED: IndependentDropListInfo::operator=(IndependentDropListInfo const&)
IndependentDropListInfo& IndependentDropListInfo::operator=(
    const IndependentDropListInfo& other) {
    file_index = other.file_index;
    drops = other.drops;
    return *this;
}

// IDA_EA: 0x8376BB2
// IDA_NAME: _ZN26stMonsterIndependentDrop_tC2ERKS_
// DEMANGLED: stMonsterIndependentDrop_t::stMonsterIndependentDrop_t(stMonsterIndependentDrop_t const&)
stMonsterIndependentDrop_t::stMonsterIndependentDrop_t(
    const stMonsterIndependentDrop_t& other)
    : value_00(other.value_00),
      value_04(other.value_04),
      total_rate(other.total_rate),
      drops(other.drops),
      min_count(other.min_count),
      max_count(other.max_count),
      rate(other.rate) {
    memcpy(values, other.values, sizeof(values));
}

// IDA_EA: 0x8376C5A
// IDA_NAME: _ZN26stMonsterIndependentDrop_tD2Ev
// DEMANGLED: stMonsterIndependentDrop_t::~stMonsterIndependentDrop_t()
stMonsterIndependentDrop_t::~stMonsterIndependentDrop_t() {}

// IDA_EA: 0x83A3942
// IDA_NAME: _ZN26stMonsterIndependentDrop_taSERKS_
// DEMANGLED: stMonsterIndependentDrop_t::operator=(stMonsterIndependentDrop_t const&)
stMonsterIndependentDrop_t& stMonsterIndependentDrop_t::operator=(
    const stMonsterIndependentDrop_t& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    memcpy(values, other.values, sizeof(values));
    total_rate = other.total_rate;
    drops = other.drops;
    min_count = other.min_count;
    max_count = other.max_count;
    rate = other.rate;
    return *this;
}

// IDA_EA: 0x897CBC4
// IDA_NAME: _ZN26stMonsterIndependentDrop_tC2Ev
// DEMANGLED: stMonsterIndependentDrop_t::stMonsterIndependentDrop_t(void)
stMonsterIndependentDrop_t::stMonsterIndependentDrop_t() {}

// IDA_EA: 0x83DD4E4
// IDA_NAME: _ZN21stIndependentDropInfoD2Ev
// DEMANGLED: stIndependentDropInfo::~stIndependentDropInfo()
stIndependentDropInfo::~stIndependentDropInfo() {}

// IDA_EA: 0x897CBDA
// IDA_NAME: _ZN21stIndependentDropInfoC2Ev
// DEMANGLED: stIndependentDropInfo::stIndependentDropInfo(void)
stIndependentDropInfo::stIndependentDropInfo() {}

// IDA_EA: 0x897D1EC
// IDA_NAME: _ZN21stIndependentDropInfoC2ERKS_
// DEMANGLED: stIndependentDropInfo::stIndependentDropInfo(stIndependentDropInfo const&)
stIndependentDropInfo::stIndependentDropInfo(
    const stIndependentDropInfo& other)
    : dungeon_index(other.dungeon_index),
      start_level(other.start_level),
      drop(other.drop) {}

// IDA_EA: 0x897D2A6
// IDA_NAME: _ZN21stIndependentDropInfoaSERKS_
// DEMANGLED: stIndependentDropInfo::operator=(stIndependentDropInfo const&)
stIndependentDropInfo& stIndependentDropInfo::operator=(
    const stIndependentDropInfo& other) {
    dungeon_index = other.dungeon_index;
    start_level = other.start_level;
    drop = other.drop;
    return *this;
}

// IDA_EA: 0x83DD514
// IDA_NAME: _ZN28stIndependentDropRateControlD2Ev
// DEMANGLED: stIndependentDropRateControl::~stIndependentDropRateControl()
stIndependentDropRateControl::~stIndependentDropRateControl() {}

// IDA_EA: 0x897CBF0
// IDA_NAME: _ZN28stIndependentDropRateControlC2Ev
// DEMANGLED: stIndependentDropRateControl::stIndependentDropRateControl(void)
stIndependentDropRateControl::stIndependentDropRateControl() {}

// IDA_EA: 0x897D5F4
// IDA_NAME: _ZN28stIndependentDropRateControlC2ERKS_
// DEMANGLED: stIndependentDropRateControl::stIndependentDropRateControl(stIndependentDropRateControl const&)
stIndependentDropRateControl::stIndependentDropRateControl(
    const stIndependentDropRateControl& other)
    : dungeon_index(other.dungeon_index),
      rate_min(other.rate_min),
      rate_max(other.rate_max),
      drop(other.drop) {}

// IDA_EA: 0x897D694
// IDA_NAME: _ZN28stIndependentDropRateControlaSERKS_
// DEMANGLED: stIndependentDropRateControl::operator=(stIndependentDropRateControl const&)
stIndependentDropRateControl& stIndependentDropRateControl::operator=(
    const stIndependentDropRateControl& other) {
    dungeon_index = other.dungeon_index;
    rate_min = other.rate_min;
    rate_max = other.rate_max;
    drop = other.drop;
    return *this;
}

// IDA_EA: 0x8372D46
// IDA_NAME: _ZN30IndependentDropParameterScriptC2Ev
// DEMANGLED: IndependentDropParameterScript::IndependentDropParameterScript(void)
IndependentDropParameterScript::IndependentDropParameterScript()
    : independent_drop_list_(),
      independent_drop_info_(),
      rate_control_(),
      file_index_range_() {
    clear();
}

// IDA_EA: 0x8372DFA
// IDA_NAME: _ZN30IndependentDropParameterScript5clearEv
// DEMANGLED: IndependentDropParameterScript::clear(void)
void IndependentDropParameterScript::clear() {
    independent_drop_info_.clear();
}

// IDA_EA: 0x837DAAC
// IDA_NAME: _ZN30IndependentDropParameterScriptD2Ev
// DEMANGLED: IndependentDropParameterScript::~IndependentDropParameterScript()
IndependentDropParameterScript::~IndependentDropParameterScript() {}

// IDA_EA: 0x897C01C
// IDA_NAME: _Z25initIndependentDropScriptPcS_
// DEMANGLED: initIndependentDropScript(char *,char *)
int initIndependentDropScript(char* src, char* filename) {
    strncpy(g_independentDropScriptBaseDirectory, src, 0xFFu);
    const size_t length = strlen(g_independentDropScriptBaseDirectory);
    if (g_independentDropScriptBaseDirectory[length - 1] != '\\' &&
        g_independentDropScriptBaseDirectory[length - 1] != '/') {
        memcpy(g_independentDropScriptBaseDirectory + length, "/", 2u);
    }
    return loadRDARScriptListFile(&g_independentDropScriptFileList,
                                  g_independentDropScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x897C0D8
// IDA_NAME: _ZN30IndependentDropParameterScript26load_independent_drop_listEv
// DEMANGLED: IndependentDropParameterScript::load_independent_drop_list(void)
int IndependentDropParameterScript::load_independent_drop_list() {
    for (STScriptFileList::const_iterator it =
             g_independentDropScriptFileList.begin();
         it != g_independentDropScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            break;
        }
        if (importIndependentDropParameterScript(filename) != 1) {
            LogManager::logFormat(
                1,
                "../RDARScriptIndependentDrop.cpp",
                "load_independent_drop_list",
                0x30,
                "IndependentDropParameterScript File Script Error: %s\n",
                filename);
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x897C1C6
// IDA_NAME: _ZN30IndependentDropParameterScript36importIndependentDropParameterScriptEPKc
// DEMANGLED: IndependentDropParameterScript::importIndependentDropParameterScript(char const*)
int IndependentDropParameterScript::importIndependentDropParameterScript(
    const char* filename) {
    if (!loadRDARScriptFile(g_independentDropScriptBaseDirectory, filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    IndependentDropListInfo list_info;
    list_info.file_index =
        g_independentDropScriptFileList.FindIndexByFullScan(filename);
    int total_rate = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[list]") {
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                total_rate += rate;
                list_info.drops.push_back(std::make_pair(item_id, rate));
            }
        }
    }

    const unsigned int list_index =
        static_cast<unsigned int>(independent_drop_list_.size());
    file_index_range_[list_info.file_index] =
        std::make_pair(static_cast<int>(list_index), total_rate);
    independent_drop_list_.push_back(list_info);
    return 1;
}

// IDA_EA: 0x897C398
// IDA_NAME: _ZN30IndependentDropParameterScript21ImportIndependentDropEPKc
// DEMANGLED: IndependentDropParameterScript::ImportIndependentDrop(char const*)
int IndependentDropParameterScript::ImportIndependentDrop(const char* filename) {
    int result_line = 171;
    if (!loadRDARScriptFile("", filename)) {
        return 102;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[independent drop]") {
            while (1) {
                int unused = 0;
                stIndependentDropInfo drop_info;
                drop_info.dungeon_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                drop_info.start_level = ScanInt(&success);
                if (!success) {
                    return 125;
                }
                drop_info.drop.value_00 = ScanInt(&success);
                if (!success) {
                    return 129;
                }
                drop_info.drop.value_04 = ScanInt(&success);
                if (!success) {
                    return 137;
                }
                drop_info.drop.values[0] = ScanInt(&success);
                if (!success) {
                    return 145;
                }
                drop_info.drop.values[1] = ScanInt(&success);
                if (!success) {
                    return 153;
                }
                drop_info.drop.values[2] = ScanInt(&success);
                if (!success) {
                    return 161;
                }
                drop_info.drop.values[3] = ScanInt(&success);
                if (!success) {
                    return 165;
                }
                for (int index = 0; index <= 3; ++index) {
                    drop_info.drop.values[index + 4] = ScanInt(&success);
                    if (!success) {
                        return 171;
                    }
                }
                drop_info.drop.values[8] = ScanInt(&success);
                if (!success) {
                    return 176;
                }

                drop_info.drop.total_rate = 0;
                drop_info.drop.drops.clear();
                drop_info.drop.min_count = ScanInt(&success);
                if (!success) {
                    return 183;
                }
                drop_info.drop.max_count = ScanInt(&success);
                if (!success) {
                    return 187;
                }
                if (drop_info.drop.min_count > drop_info.drop.max_count) {
                    return 190;
                }
                drop_info.drop.rate = ScanInt(&success);
                if (!success) {
                    return 195;
                }
                const int flag = ScanInt(&success);
                if (static_cast<unsigned int>(flag) > 2u) {
                    LogManager::logFormat(1,
                                          "../RDARScriptIndependentDrop.cpp",
                                          "ImportIndependentDrop",
                                          0xCA,
                                          "flag wrong number(%d)",
                                          flag);
                    return 203;
                }
                if (!success) {
                    return 207;
                }

                std::string list_tag;
                if (flag == 1) {
                    if (ScanType(list_tag, true) == 1 && list_tag == "[list]") {
                        while (1) {
                            unused = ScanInt(&success);
                            if (!success) {
                                break;
                            }
                            const int rate = ScanInt(&success);
                            if (!success) {
                                return 225;
                            }
                            drop_info.drop.total_rate += rate;
                            drop_info.drop.drops.push_back(std::make_pair(
                                unused, drop_info.drop.total_rate));
                        }
                    }
                } else if (flag == 2) {
                    if (ScanType(list_tag, true) == 1 && list_tag == "[list]") {
                        while (1) {
                            bool found = false;
                            const int list_index = ScanInt(&success);
                            if (!success) {
                                break;
                            }
                            for (std::size_t i = 0;
                                 i < independent_drop_list_.size(); ++i) {
                                if (independent_drop_list_[i].file_index ==
                                    list_index) {
                                    for (std::size_t j = 0;
                                         j < independent_drop_list_[i]
                                                 .drops.size();
                                         ++j) {
                                        unused = independent_drop_list_[i]
                                                     .drops[j]
                                                     .first;
                                        const int rate =
                                            independent_drop_list_[i]
                                                .drops[j]
                                                .second;
                                        drop_info.drop.total_rate += rate;
                                        drop_info.drop.drops.push_back(
                                            std::make_pair(
                                                unused,
                                                drop_info.drop.total_rate));
                                    }
                                    found = true;
                                }
                                if (found) {
                                    break;
                                }
                            }
                        }
                    }
                }

                independent_drop_info_.push_back(drop_info);
            }
        }

        if (tag == "[dungeon drop rate balance]") {
            while (1) {
                stIndependentDropRateControl rate_control;
                rate_control.dungeon_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                rate_control.rate_min = ScanInt(&success);
                if (!success) {
                    break;
                }
                rate_control.rate_max = ScanInt(&success);
                if (!success) {
                    break;
                }
                rate_control_.push_back(rate_control);
            }
        }
    }

    result_line = 0;
    return result_line;
}
