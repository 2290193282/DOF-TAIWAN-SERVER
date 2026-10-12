#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_CREATURE_GUILD_PARAMS_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_CREATURE_GUILD_PARAMS_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "old_data/PVF/data/datamanager/active_status_sync_server.h"
#include "old_data/PVF/creature/datamanager/creature_script.h"
#include "infra/network/packet/message_base.h"


// IDA unit: 06_userinfo_stat_inventory/stGrowthEquipment.
struct stGrowthEquipment : public ParamBase {
    stGrowthEquipment();
    virtual ~stGrowthEquipment();

    int result;                 // +0x04
    int item_space;             // +0x08
    int slot;                   // +0x0c
    unsigned short timepiece;   // +0x10
    unsigned char pad_12[2];
    int reserved_14;            // +0x14, constructed but not written by CMD506
};

// IDA unit: 06_userinfo_stat_inventory/stGrowthCreature.
struct stGrowthCreature : public ParamBase {
    stGrowthCreature();
    virtual ~stGrowthCreature();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stHatchCreature.
struct stHatchCreature : public ParamBase {
    stHatchCreature();
    virtual ~stHatchCreature();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stRenameCreature.
struct stRenameCreature : public ParamBase {
    stRenameCreature();
    virtual ~stRenameCreature();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stGuildCargoParam.
struct stGuildCargoParam : public ParamBase {
    stGuildCargoParam();
    virtual ~stGuildCargoParam();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stGuildCargoPushItemParam.
struct stGuildCargoPushItemParam : public ParamBase {
    stGuildCargoPushItemParam();
    virtual ~stGuildCargoPushItemParam();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stGuildCargoPopItemParam.
struct stGuildCargoPopItemParam : public ParamBase {
    stGuildCargoPopItemParam();
    virtual ~stGuildCargoPopItemParam();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stGuildCargoMoveItemParam.
struct stGuildCargoMoveItemParam : public ParamBase {
    stGuildCargoMoveItemParam();
    virtual ~stGuildCargoMoveItemParam();

    int result;
};

// IDA unit: 06_userinfo_stat_inventory/stGuildCargoHistoryParam.
struct stGuildCargoHistoryParam : public ParamBase {
    stGuildCargoHistoryParam();
    virtual ~stGuildCargoHistoryParam();

    int result;
};

// C06 only consumes these PVF records; their declaration and lifecycle are
// canonical in DataManager.


#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_CREATURE_GUILD_PARAMS_H_
