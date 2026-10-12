#include "old_data/PVF/data/data_manager.h"

// IDA_EA: 0x8110B62
// IDA_NAME: _ZN12CDataManager20get_event_script_mngEv
// DEMANGLED: CDataManager::get_event_script_mng(void)
EventClassify::CEventScriptMng* CDataManager::get_event_script_mng() {
    return event_script_manager_;
}
