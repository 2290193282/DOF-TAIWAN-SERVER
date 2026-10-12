#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/server_parameter_script_abi.h"

// IDA_EA: CDataManager::C2 @0x8355B30 calls ServerParameterScript::C2 with
// this + 0x68 (call site 0x8355B50).  The same embedded object is destroyed
// from +0x68 by CDataManager::D2 @0x8356B68.
ServerParameterScript* CDataManager::get_server_parameter_script() {
    return &server_parameter_script_;
}

const ServerParameterScript*
CDataManager::get_server_parameter_script() const {
    return &server_parameter_script_;
}

QuestList* CDataManager::get_quest_list() {
    return quest_list_;
}
