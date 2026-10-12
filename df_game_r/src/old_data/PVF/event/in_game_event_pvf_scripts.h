#ifndef OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_PVF_SCRIPTS_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_PVF_SCRIPTS_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_game/activity/schedule/event_value_types.h"


// IDA unit: 12_pvf_datamanager_scripts/AddItemCountDBScript.
struct AddItemCountDBScript : public InGameEventScript {
    // IDA_EA: 0x8986134
    AddItemCountDBScript();

    std::vector<int> values_08;
    std::vector<int> values_14;
    std::string value_20;
};

// IDA unit: 12_pvf_datamanager_scripts/FatigueQuantityScript.
struct FatigueQuantityScript : public InGameEventScript {
    // IDA_EA: 0x8985C74
    FatigueQuantityScript();

    std::string value_08;
    uint8_t flag_0c;
};

// IDA unit: 12_pvf_datamanager_scripts/UsedFatigueGiveItemScript.
struct UsedFatigueGiveItemScript : public InGameEventScript {
    // IDA_EA: 0x8985EEC
    UsedFatigueGiveItemScript();

    std::string value_08;
    uint8_t flag_0c;
    int32_t value_10;
    uint8_t flag_14;
    int32_t value_18;
    int32_t value_1c;
    Action_SendMail send_mail_20;
};

// IDA unit: 12_pvf_datamanager_scripts/PcBangPlayTimeScript.
struct PcBangPlayTimeScript : public InGameEventScript {
    // IDA_EA: 0x8985FB8
    PcBangPlayTimeScript();

    std::string value_08;
    int32_t value_0c;
    uint8_t flag_10;
    uint16_t value_12;
    uint16_t value_14;
};

// IDA_EA: 0x897FC99
int importAddItemCountDBEventScript(AddItemCountDBScript* script,
                                    const char* filename);
// IDA_EA: 0x8982FC6
int importFatigueQuantityEventScript(FatigueQuantityScript* script,
                                     const char* filename);
// IDA_EA: 0x898491B
int importUsedFatigueGiveItemEventScript(UsedFatigueGiveItemScript* script,
                                         const char* filename);
// IDA_EA: 0x898512C
int importPcBangPlayTimeScript(PcBangPlayTimeScript* script,
                               const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_PVF_SCRIPTS_H_
