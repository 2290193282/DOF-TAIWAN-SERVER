#ifndef OLDSRV_OLD_DATA_PVF_ETC_CRANE_MINIGAME_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_CRANE_MINIGAME_SCRIPT_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_data/PVF/event/in_game_event_pvf_scripts.h"


// IDA unit: 99_misc_global/CraneItemInfo.
// This PVF record is owned by the Crane RDAR importer.  The Crane gameplay
// manager only consumes it through CraneMinigameScript::getItem().
struct CraneItemInfo {
    // IDA_EA: 0x88E41F6.
    CraneItemInfo();
    // IDA_EA: 0x88E3708.
    CraneItemInfo& operator=(const CraneItemInfo& rhs);

    int16_t item_index;       // +0x00: parser-assigned vector position
    int16_t reserved_02;      // +0x02: untouched by the original ctor/copy
    int32_t item_id;          // +0x04
    int16_t item_count;       // +0x08
    int16_t reserved_0a;      // +0x0a
    float view_ratio;         // +0x0c
    int32_t pick_ratio;       // +0x10
};

// IDA unit: 12_pvf_datamanager_scripts/CraneMinigameScript.
struct CraneMinigameScript {
    // IDA_EA: 0x88E3736.
    CraneMinigameScript();
    // IDA_EA: 0x88E37A8.
    ~CraneMinigameScript();

    // IDA_EA: 0x88E38D6.
    CraneItemInfo* getItem(int16_t item_index);

    int16_t view_count;       // +0x00
    int16_t reserved_02;      // +0x02
    std::vector<CraneItemInfo*> items;  // +0x04
    int32_t material_item_id; // +0x10
    int16_t material_count;   // +0x14
    int16_t reserved_16;      // +0x16
    Action_SendMail send_mail;  // +0x18
    std::vector<std::pair<int, int> > need_materials;  // +0x5c

private:
    CraneMinigameScript(const CraneMinigameScript&);
    CraneMinigameScript& operator=(const CraneMinigameScript&);
};

// IDA_EA: 0x88E3924.
int importCraneMinigameScript(CraneMinigameScript* script,
                              const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_CRANE_MINIGAME_SCRIPT_H_
