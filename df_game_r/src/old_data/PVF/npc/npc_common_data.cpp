#include "old_data/PVF/npc/npc_common_data.h"

#include "old_data/PVF/common/rdar_script.h"
#include "common/manager/change_status_data.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

extern "C" char g_npcScriptBaseDirectory[0x101];
char g_npcScriptBaseDirectory[0x101];

// IDA_EA: 0x8A04B54
// IDA_NAME: _ZN17stNPCCommonData_t11stOneBuff_tD2Ev
// DEMANGLED: stNPCCommonData_t::stOneBuff_t::~stOneBuff_t()
stNPCCommonData_t::stOneBuff_t::~stOneBuff_t() {}

// IDA_EA: 0x837612E
// IDA_NAME: _ZN17stNPCCommonData_tC2Ev
// DEMANGLED: stNPCCommonData_t::stNPCCommonData_t(void)
stNPCCommonData_t::stNPCCommonData_t()
    : favorable_npc_indices(),
      illust_indices(),
      favor_level_reward_npc_indices(),
      favor_duration_buffs() {}

// IDA_EA: 0x83761C6
// IDA_NAME: _ZN17stNPCCommonData_tD2Ev
// DEMANGLED: stNPCCommonData_t::~stNPCCommonData_t()
stNPCCommonData_t::~stNPCCommonData_t() {}

// IDA_EA: 0x89FAE9E
// IDA_NAME: _ZN17stNPCCommonData_t5clearEv
// DEMANGLED: stNPCCommonData_t::clear(void)
stNPCCommonData_t* stNPCCommonData_t::clear() {
    for (int index = 0; index <= 9; ++index) {
        favor_level_values[index] = 0;
    }
    max_favor_value = 0;
    favor_system_begin_quest = 0;
    favor_system_begin_level = 0;
    max_favor_level_npc = 0;
    return this;
}

// IDA_EA: 0x89FAEEE
// IDA_NAME: _ZN17stNPCCommonData_t14getIllustIndexEi
// DEMANGLED: stNPCCommonData_t::getIllustIndex(int)
int stNPCCommonData_t::getIllustIndex(int illust_id) {
    for (unsigned int index = 0; index < illust_indices.size(); ++index) {
        if (illust_indices.at(index) == illust_id) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x89FAFC5
// IDA_NAME: _Z17loadNPCCommonDataR17stNPCCommonData_tPKc
// DEMANGLED: loadNPCCommonData(stNPCCommonData_t &,char const*)
int loadNPCCommonData(stNPCCommonData_t& data, const char* filename) {
    if (!loadRDARScriptFile(g_npcScriptBaseDirectory, filename)) {
        return 60;
    }

    std::string tag;
    int value = 0;
    bool scan_success = false;
    data.clear();

    while (ScanType(tag, true) == 1) {
        if (tag == "[npc favor exp]") {
            while (true) {
                value = ScanInt(&scan_success);
                if (!scan_success) {
                    break;
                }
                // IDA @0x89FB072 writes the parsed index directly.  Do not
                // add a range-rejection branch that changes the old parser.
                data.favor_level_values[value] =
                    static_cast<uint16_t>(ScanInt(&scan_success));
                if (!scan_success) {
                    return 0;
                }
                data.max_favor_value =
                    std::max(data.max_favor_value,
                             data.favor_level_values[value]);
            }
        } else if (tag == "[favorable npc list]") {
            while (true) {
                value = ScanInt(&scan_success);
                if (!scan_success) {
                    break;
                }
                data.favorable_npc_indices.push_back(value);
            }
        } else if (tag == "[illustration variable npc list]") {
            while (true) {
                value = ScanInt(&scan_success);
                if (!scan_success) {
                    break;
                }
                data.illust_indices.push_back(value);
            }
        } else if (tag == "[max favor level npc]") {
            data.max_favor_level_npc =
                static_cast<uint16_t>(ScanInt(&scan_success));
            if (!scan_success) {
                break;
            }
        } else if (tag == "[favor system begin quest]") {
            data.favor_system_begin_quest =
                static_cast<uint16_t>(ScanInt(&scan_success));
            if (!scan_success) {
                break;
            }
        } else if (tag == "[favor system begin level]") {
            data.favor_system_begin_level =
                static_cast<uint16_t>(ScanInt(&scan_success));
            if (!scan_success) {
                break;
            }
        } else if (tag == "[favor level reward npc]") {
            while (true) {
                value = ScanInt(&scan_success);
                if (!scan_success) {
                    break;
                }
                data.favor_level_reward_npc_indices.push_back(value);
            }
        } else if (tag == "[favor duration buff list]") {
            while (true) {
                std::vector<stNPCCommonData_t::stOneBuff_t> buffs;
                const int group = ScanInt(&scan_success);
                if (!scan_success) {
                    break;
                }
                const int count = ScanInt(&scan_success);
                if (!scan_success) {
                    return 0;
                }
                for (int index = 0; index < count; ++index) {
                    std::string status_name;
                    if (ScanStr(&status_name) != 1) {
                        return 0;
                    }

                    stNPCCommonData_t::stOneBuff_t buff;
                    // The original calls this lookup twice.  Keep both calls:
                    // it is part of the recovered control flow, rather than
                    // folding the first result away.
                    (void)getChangeStatusType(status_name);
                    buff.change_status_type = getChangeStatusType(status_name);
                    buff.value = ScanInt(&scan_success);
                    if (!scan_success) {
                        return 0;
                    }
                    std::string percent_text;
                    if (ScanStr(&percent_text) != 1) {
                        return 0;
                    }
                    // IDA @0x89FB38E writes only byte +8; the following
                    // three bytes are copied as the original stack value.
                    buff.is_percent = percent_text != "+";
                    buffs.push_back(buff);
                }
                data.favor_duration_buffs.push_back(
                    std::make_pair(group, buffs));
            }
        }
    }

    return 0;
}
