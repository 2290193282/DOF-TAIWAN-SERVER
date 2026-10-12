#include "old_data/PVF/etc/vending_machine_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/common/script_file_list.h"
#include "infra/log/format/trace.h"

#include <string>
#include <utility>


// IDA_EA: 0x890B2A3
// IDA_NAME: _Z28importVendingMachineInfoCorePKcS0_R18stVendingMachine_t
// DEMANGLED: importVendingMachineInfoCore(char const*,char const*,stVendingMachine_t&)
int importVendingMachineInfoCore(const char* dir,
                                 const char* filename,
                                 stVendingMachine_t& script) {
    if (!loadRDARScriptFile(dir, filename)) {
        return 7691;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[item group]") {
            stVendingMachineInfo_t info;
            int result = 1;
            while (ScanType(tag, true) == 1) {
                if (tag == "[/item group]") {
                    script.groups.push_back(info);
                    info.clear();
                    break;
                }

                if (tag == "[group num]") {
                    info.group_num = ScanInt(&success);
                    if (!success) {
                        result = 7719;
                        break;
                    }
                } else if (tag == "[material]") {
                    info.material_item = ScanInt(&success);
                    if (!success) {
                        result = 7725;
                        break;
                    }
                    info.material_count = ScanInt(&success);
                    if (!success) {
                        result = 7728;
                        break;
                    }
                } else if (tag == "[output]") {
                    while (1) {
                        stVendingMachineOutput_t output;
                        output.item_index =
                            static_cast<unsigned int>(ScanInt(&success));
                        if (!success) {
                            break;
                        }

                        const int rate = ScanInt(&success);
                        if (!success) {
                            result = 7741;
                            break;
                        }

                        const int item_count = ScanInt(&success);
                        if (!success) {
                            result = 7744;
                            break;
                        }

                        if (ScanInt(&success) == 1) {
                            info.dictionary_items.push_back(std::make_pair(
                                static_cast<int>(output.item_index), item_count));
                        }
                        if (!success) {
                            result = 7748;
                            break;
                        }

                        info.normal_total_rate += rate;
                        output.cumulative_rate =
                            static_cast<unsigned int>(info.normal_total_rate);
                        output.item_count = static_cast<unsigned int>(item_count);
                        info.normal_outputs.push_back(output);
                    }
                    if (result != 1) {
                        break;
                    }
                } else if (tag == "[bonus output]") {
                    while (1) {
                        stVendingMachineOutput_t output;
                        output.item_index =
                            static_cast<unsigned int>(ScanInt(&success));
                        if (!success) {
                            break;
                        }

                        const int rate = ScanInt(&success);
                        if (!success) {
                            result = 7765;
                            break;
                        }

                        const int item_count = ScanInt(&success);
                        if (!success) {
                            result = 7768;
                            break;
                        }

                        if (ScanInt(&success) == 1) {
                            info.dictionary_items.push_back(std::make_pair(
                                static_cast<int>(output.item_index), item_count));
                        }
                        if (!success) {
                            result = 7772;
                            break;
                        }

                        info.bonus_total_rate += rate;
                        output.cumulative_rate =
                            static_cast<unsigned int>(info.bonus_total_rate);
                        output.item_count = static_cast<unsigned int>(item_count);
                        info.bonus_outputs.push_back(output);
                    }
                    if (result != 1) {
                        break;
                    }
                }
            }

            if (result != 1) {
                return result;
            }
        } else {
            LogManager::logFormat(
                0,
                "../RDARScriptEtcParameter.cpp",
                "importVendingMachineInfoCore",
                0x1E66,
                "ERROR : Unknown tag error. %s",
                tag.c_str());
        }
    }

    return 0;
}

// IDA_EA: 0x890B74E
// IDA_NAME: _Z24importVendingMachineInfoPKcS0_RSt6vectorI18stVendingMachine_tSaIS2_EE
// DEMANGLED: importVendingMachineInfo(char const*,char const*,std::vector<stVendingMachine_t>&)
int importVendingMachineInfo(
    const char* dir,
    const char* filename,
    std::vector<stVendingMachine_t>& table) {
    STScriptFileList file_list;
    if (!loadRDARScriptListFile(&file_list, dir, filename)) {
        return 7811;
    }

    if (file_list.empty()) {
        return 7795;
    }

    for (STScriptFileList::iterator it = file_list.begin();
         it != file_list.end(); ++it) {
        stVendingMachine_t script;
        script.script_index = it->first;
        if (importVendingMachineInfoCore(dir, it->second.c_str(), script) != 0) {
            return 7804;
        }
        table.push_back(script);
    }

    return 0;
}
